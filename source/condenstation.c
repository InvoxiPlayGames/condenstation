#include <sys/prx.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <sysutil/sysutil_msgdialog.h>
#include <sys/timer.h>

#include <cell/cell_fs.h>
#include <cell/pad.h>

#include <stdio.h>
#include <string.h>
#include <ppcasm.h>

#include "condenstation_config.h"
#include "cellHttpHelper.h"
#include "memmem.h"
#include "inih.h"
#include "steam_api.h"
#include "ISteamPS3OverlayRender_c.h"
#include "ISteamMatchmaking_c.h"
#include "ps3_utilities.h"
#include "CMsgClientLogonAutogen.h"
#include "CMsgClientLogonResponseAutogen.h"
#include "steamclient_utils.h"
#include "steamclient_google_protobufs.h"
#include "valve_ps3.h"
#include "SteamAuthentication.h"
#include "steammessages_auth.steamclient.pb-c.h"
#include "steamclient_steamps3params.h"

#include "libqrencode_qrencode.h"

#define TOC_SETTER_STUB(x)                \
    void SetTOCFor_##x()                    \
    {                                     \
        asm("lis 2, toc_register@h;");    \
        asm("ori 2, 2, toc_register@l;"); \
        asm("b ." #x ";");           \
    }

// lv2 printf, regular printf doesn't (..?) work (or needs configuration)
extern int _sys_printf(char *fmt, ...);
extern int _sys_sprintf(char *buf, char *fmt, ...);

SYS_MODULE_INFO(condenstation, 0, 1, 0);
SYS_MODULE_START(_prx_start);
SYS_MODULE_STOP(_prx_stop);

void hexdump(uint8_t *source, size_t len) {
    for(int i = 0; i < len; i++)
        _sys_printf("%02x ", source[i]);
    _sys_printf("\n");
}

static uint32_t get_toc_base()
{
    asm("mr 3, 2;");
}

std_basic_string username_bs;

// we need "toc stub"s for these functions because we are setting by address rather than setting from other function pointers
toc_stub CodedOutputStream_VarintSize32Fallback_stub;
toc_stub WireFormatLite_WriteString_stub;
toc_stub std_basic_string_constructor_stub;
// function pointers that we need to call from within our hooks
CodedOutputStream_VarintSize32Fallback_t CodedOutputStream_VarintSize32Fallback = &CodedOutputStream_VarintSize32Fallback_stub;
WireFormatLite_WriteString_t WireFormatLite_WriteString = &WireFormatLite_WriteString_stub;
std_basic_string_constructor_t std_basic_string_constructor = &std_basic_string_constructor_stub;

bool use_v2_cmsgclientlogon = false;
// general function to fix up a CMsgClientLogon message to include our username and correct base fields
void fix_up_CMsgClientLogon(CMsgClientLogonAutogen *cmsg)
{
    CMsgClientLogonAutogenV2 *cmsgv2 = (CMsgClientLogonAutogenV2 *)cmsg;
    _sys_printf("fix_up_CMsgClientLogon %p\n", cmsg);

    // set OS type to Windows 11
    cmsg->client_os_type = 20;

    // unset the PSN ticket and service ID
    if (use_v2_cmsgclientlogon)
        cmsgv2->enabled_fields &= ~(0x180000); // unset PSN ticket/service
    else
        cmsg->enabled_fields &= ~(0x180000); // unset PSN ticket/service

    // required to use persistent refresh tokens
    cmsg->should_remember_password = true; // must be disabled to use non-persistent refresh tokens
    if (use_v2_cmsgclientlogon)
        cmsgv2->enabled_fields |= 0x80; // set should remember password
    else
        cmsg->enabled_fields |= 0x80; // set should remember password

    // if we haven't got one already, set up a username
    if ((cmsg->enabled_fields & 0x10000) == 0) {
        _sys_printf("adding an account name\n");
        std_basic_string_constructor(&username_bs, SteamAccountName);
        if (cmsg->account_name == NULL) {
            _sys_printf("setting to our username fake string - could crash\n");
            cmsg->account_name = &username_bs;
        } else {
            _sys_printf("modifying existing basic string\n");
            cmsg->account_name->current_size = username_bs.current_size;
            cmsg->account_name->max_size = username_bs.max_size;
            // TODO(Emma): check, does this actually work..?
            if (cmsg->account_name->current_size <= 0x10)
                memcpy(cmsg->account_name->data.buffer, username_bs.data.buffer, cmsg->account_name->current_size);
            else
                cmsg->account_name->data.data_ptr = username_bs.data.data_ptr;
        }
        // enable the field to be sent
        if (use_v2_cmsgclientlogon)
            cmsgv2->enabled_fields |= 0x10000;
        else
            cmsg->enabled_fields |= 0x10000;
    }
}

TOC_SETTER_STUB(CMsgClientLogon_ByteSize_Hook)
ProtobufByteSize_t CMsgClientLogon_ByteSize;
// hook when the client gets the size of the CMsgClientLogon protobuf
// we have to add the size of our access token
int CMsgClientLogon_ByteSize_Hook(void *protobuf)
{
    _sys_printf("CMsgClientLogon_ByteSize_Hook\n");
    CMsgClientLogonAutogen *cmsg = (CMsgClientLogonAutogen *)protobuf;

    // make sure the original struct is populated with the right info
    fix_up_CMsgClientLogon(cmsg);

    // get the original byte length
    _sys_printf("calling CMsgClientLogon_ByteSize\n");
    int orig_len = CMsgClientLogon_ByteSize(protobuf);
    _sys_printf("original length: %i\n", orig_len);

    // add the length for our access_token
    int data_len = strlen(SteamAccessToken);
    int pref_byte_len = 1; // length byte is 1 byte long unless it needs to be a varint
    if (data_len >= 0x80)
        pref_byte_len = CodedOutputStream_VarintSize32Fallback(data_len);
    _sys_printf("data_len = %i\n", data_len);
    _sys_printf("pref_byte_len = %i\n", data_len);
    orig_len += (pref_byte_len + data_len + 2);
    _sys_printf("new length: %i\n", orig_len);

    // update the byte length in the structure (GetCachedSize as well as serializing to array uses it)
    cmsg->length = orig_len;
    return orig_len;
}

TOC_SETTER_STUB(CMsgClientLogon_SerializeWithCachedSizes_Hook)
ProtobufSerializeWithCachedSizes_t CMsgClientLogon_SerializeWithCachedSizes;
// hook when the client tries to serialize CMsgClientLogon
// we have to add our access token
void CMsgClientLogon_SerializeWithCachedSizes_Hook(void *protobuf, void *outputCodedStream)
{
    _sys_printf("CMsgClientLogon_SerializeWithCachedSizes_Hook\n");
    CMsgClientLogonAutogen *cmsg = (CMsgClientLogonAutogen *)protobuf;

    // make sure the original struct is populated with the right info
    fix_up_CMsgClientLogon(cmsg);

    // serialize the unmodified message to the output buffer
    _sys_printf("calling CMsgClientLogon_SerializeWithCachedSizes\n");
    CMsgClientLogon_SerializeWithCachedSizes(protobuf, outputCodedStream);

    // serialize our access token into the output buffer
    _sys_printf("serializing access token\n");
    std_basic_string access_token;
    std_basic_string_constructor(&access_token, SteamAccessToken);
    WireFormatLite_WriteString(108, &access_token, outputCodedStream);
}

TOC_SETTER_STUB(CMsgClientLogonResponse_MergePartialFromCodedStream_Hook)
ProtobufMergePartialFromCodedStream_t CMsgClientLogonResponse_MergePartialFromCodedStream;
// hook when the client tries to deserialize CMsgClientLogonResponse so we know the result and can change it where necessary
int CMsgClientLogonResponse_MergePartialFromCodedStream_Hook(void *protobuf, void *inputCodedStream)
{
    _sys_printf("CMsgClientLogonResponse_MergePartialFromCodedStream_Hook\n");
    int r = CMsgClientLogonResponse_MergePartialFromCodedStream(protobuf, inputCodedStream);

    CMsgClientLogonResponseAutogen *cmsg = (CMsgClientLogonResponseAutogen *)protobuf;

    _sys_printf("logon EResult: %i\n", cmsg->eresult);

    // uncomment to get the "link your psn to steam" screen
    //if (cmsg->eresult != 1)
    //    cmsg->eresult = 57; // ExternalAccountUnlinked

    if (cmsg->eresult != 1) {
        // logon failed for some reason - act like we don't have a config loaded
        // next interactive logon should trigger a QR code authentication
        HasConfigLoaded = false;
    }

    return r;
}

typedef struct _netadr_t {
    uint16_t port;
    uint32_t ip;
    int type;
} netadr_t;

bool has_netadr = false;
netadr_t netadr_to_use;
int CUtlVector_AddMultipleToTail_netadr(void *utlvector, int count, netadr_t *objects);
int CUtlVector_AddMultipleToTail_netadr_hook(void *utlvector, int count, netadr_t *objects)
{
    // return a hardcoded netadr
    _sys_printf("connecting to CM %08x:%i\n", netadr_to_use.ip, netadr_to_use.port);
    return CUtlVector_AddMultipleToTail_netadr(utlvector, 1, &netadr_to_use);
}

bool IPaddrToNetadr(const char *in_ip_addr, netadr_t *out_netadr)
{
    int ip[4];
    int port;
    int scanned = sscanf(in_ip_addr, "%d.%d.%d.%d:%d", &(ip[0]), &(ip[1]), &(ip[2]), &(ip[3]), &port);
    if (scanned < 4) return false;
    if (scanned == 4) port = 27017;
    out_netadr->type = 3;
    out_netadr->ip = ((ip[0] & 0xFF) << 24) | ((ip[1] & 0xFF) << 16) | ((ip[2] & 0xFF) << 8) | (ip[3] & 0xFF);
    out_netadr->port = port;
    return true;
}

void SetCMNetadr(const char *in_ip_addr)
{
    IPaddrToNetadr(in_ip_addr, &netadr_to_use);
    has_netadr = true;   
}

SteamPS3Params_t *steamPS3Params;
steamclient_GetSteamPS3Params_t steamclient_GetSteamPS3Params;

void ApplyOverlayHooks(ISteamPS3OverlayRender_t *renderer);

typedef void (*CSteamEngine_InitCDNCache_t)(void *csteamengine);
toc_stub CSteamEngine_InitCDNCache_stub;
CSteamEngine_InitCDNCache_t CSteamEngine_InitCDNCache = &CSteamEngine_InitCDNCache_stub;

void *stored_cuser = 0;
uint64_t stored_steamid = 0;
typedef void (*CUser_LogOn_t)(void *cuser, bool bInteractive, uint64_t steamid);
CUser_LogOn_t CUser_LogOn;

void QRoverlay_start_displaying_qr(const char *url);

bool has_pending_interactive_logon = false;
void dispatch_pending_logon(bool fail) {
    // don't dispatch this if we don't have an interactive logon pending
    if (!has_pending_interactive_logon)
        return;

    if (!use_v2_cmsgclientlogon) {
        // TODO(Emma): this relies on hardcoded offsets kinda so only works on latest Portal 2
        //             does it matter? for release no, but for future plans yes
        uint8_t *cuser_raw = (uint8_t *)stored_cuser;
        *(int *)(cuser_raw + 0x28 + 0x3F8) = fail ? 0 : 1;
        // setting this value causes the CCMInterface::Connect to fail
    }
    // dispatch the original call to CUser::LogOn
    CUser_LogOn(stored_cuser, true, stored_steamid);
}

sys_ppu_thread_t loginThread;

void qr_code_auth_thread();
void logon_initial_dialog_callback(int buttonType, void *userData)
{
    if (buttonType == CELL_MSGDIALOG_BUTTON_YES) {
        // start the QR code authentication process
        sys_ppu_thread_create(&loginThread, qr_code_auth_thread, NULL, 1024, 0x8000, SYS_PPU_THREAD_CREATE_JOINABLE, "condenstation_QRLogin");
    } else {
        // fail the logon if any other option was selected
        dispatch_pending_logon(true);
    }
}

bool has_fatal_error = false;
char fatal_error_text[256];
void set_fatal_error(const char *text) {
    strncpy(fatal_error_text, text, sizeof(fatal_error_text));
    has_fatal_error = true;
}

void nullcb(int a, void *b) {}

void CUser_LogOn_Hook(void *cuser, bool bInteractive, uint64_t steamid) {
    _sys_printf("CUser::LogOn(%p, %i, %p) called!\n", cuser, bInteractive, steamid);
    stored_cuser = cuser;
    stored_steamid = steamid;
    uint8_t *cuser_raw = (uint8_t *)cuser;
    // display a fatal error first and foremost if we have one queued
    if (has_fatal_error) {
        if (bInteractive) {
            cellMsgDialogOpen2(CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK | CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR, 
                fatal_error_text, nullcb, NULL, NULL);
        }
        // fail the logon
        if (!use_v2_cmsgclientlogon)
            *(int *)(cuser_raw + 0x28 + 0x3F8) = 0;
        CUser_LogOn(cuser, bInteractive, steamid);
    }
    // if we don't have a netadr
    if (!has_netadr) {
        if (bInteractive) {
            cellMsgDialogOpen2(CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK | CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL, 
                "condenstation is still trying to connect to Steam, please try again in a few seconds.", nullcb, NULL, NULL);
        }
        // fail the logon
        if (!use_v2_cmsgclientlogon)
            *(int *)(cuser_raw + 0x28 + 0x3F8) = 0;
        CUser_LogOn(cuser, bInteractive, steamid);
    }
    // if we don't have a saved account, decide whether to fail the logon or start the QR flow based on interactivity
    if (!HasConfigLoaded) {
        if (bInteractive) {
            has_pending_interactive_logon = true;
            cellMsgDialogOpen2(
                CELL_MSGDIALOG_TYPE_BUTTON_TYPE_YESNO |
                CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON |
                CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL,
                "You don't have a Steam account connected to condenstation yet,\nwould you like to sign in using a QR code?\n\n(You need Steam Guard Mobile Authenticator to use this.)",
                logon_initial_dialog_callback, NULL, NULL);
        } else {
            // fail the logon
            if (!use_v2_cmsgclientlogon)
                *(int *)(cuser_raw + 0x28 + 0x3F8) = 0;
            CUser_LogOn(cuser, bInteractive, steamid);
        }
    } else {
        // dispatch the logon as usual
        CUser_LogOn(cuser, bInteractive, steamid);
    }
}

void get_cm_thread();

int ISteamMatchmaking_RequestLobbyList_Hook(void *thisobj) {
    _sys_printf("Preventing call to RequestLobbyList.\n");
    cellMsgDialogOpen2(CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK | CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR,
        "Sorry, you can't search for random players with condenstation.", nullcb, NULL, NULL);
    return -1;
}

void shillthread() {
    if (!use_v2_cmsgclientlogon) {
        cellMsgDialogOpen2(CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON | CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL,
        "welcome to condenstation beta 1 for Portal 2!\n\nhttps://github.com/InvoxiPlayGames/condenstation", nullcb, NULL, NULL);
        cellMsgDialogClose(5690);
    } else {
        cellMsgDialogOpen2(CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON | CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR,
        "condenstation beta 1 - CS:GO IS NOT SUPPORTED!\n\nhttps://github.com/InvoxiPlayGames/condenstation", nullcb, NULL, NULL);
        cellMsgDialogClose(5690);
    }
}

void apply_steamclient_patches()
{
    // initialise our SteamClient utility library
    SCUtils_Init();

    // detect the current game by looking at the appid passed into steamclient
    steamclient_GetSteamPS3Params = SCUtils_LookupNID(NID_steamclient_GetSteamPS3Params);
    if (steamclient_GetSteamPS3Params != NULL)
    {
        steamPS3Params = steamclient_GetSteamPS3Params();
        use_v2_cmsgclientlogon = (steamPS3Params->m_unVersion == STEAM_PS3_CSGO_PARAMS_VER);
        if (steamPS3Params->m_unVersion == STEAM_PS3_PORTAL2_PARAMS_VER)
            _sys_printf("portal 2!\n");
        else if (steamPS3Params->m_unVersion == STEAM_PS3_CSGO_PARAMS_VER)
            _sys_printf("csgo!\n");
        else
            _sys_printf("what?\n");
        
        _sys_printf("m_nAppId = %i\n", steamPS3Params->m_nAppId);
        _sys_printf("m_rgchInstallationPath = %s\n", steamPS3Params->m_rgchInstallationPath);
        _sys_printf("m_rgchGameData = %s\n", steamPS3Params->m_rgchGameData);
        _sys_printf("m_rgchSystemCache = %s\n", steamPS3Params->m_rgchSystemCache);
    }

    SteamAPI_ClassAccessor_t SteamPS3OverlayRender = SCUtils_LookupSteamAPINID(NID_SteamPS3OverlayRender);
    // don't apply overlay hooks to csgo
    if (SteamPS3OverlayRender != NULL && !use_v2_cmsgclientlogon) {
        ISteamPS3OverlayRender_t *render = SteamPS3OverlayRender();
        ApplyOverlayHooks(render);
    }

    // hook steam matchmaking so we can't search for lobbies
    SteamAPI_ClassAccessor_t SteamMatchmaking = SCUtils_LookupSteamAPINID(NID_SteamMatchmaking);
    if (SteamMatchmaking != NULL) {
        ISteamMatchmaking_t *mm = SteamMatchmaking();
        PS3_Write32(&(mm->vt->RequestLobbyList), (uint32_t)ISteamMatchmaking_RequestLobbyList_Hook);
    }

    // load our config file
    load_auth_config();
    if (HasConfigLoaded) {
        _sys_printf("Loaded config, account: '%s'\n", SteamAccountName);
    }

    // get the vtables of the protobuf objects we want to modify
    sc_protobuf_vtable_t *cmsgclientlogon_vt = SCUtils_GetProtobufVtable("CMsgClientLogon");
    sc_protobuf_vtable_t *cmsgclientlogonresponse_vt = SCUtils_GetProtobufVtable("CMsgClientLogonResponse");
    _sys_printf("CMsgClientLogon: %p\n", cmsgclientlogon_vt);
    _sys_printf("CMsgClientLogonResponse: %p\n", cmsgclientlogonresponse_vt);

    // keep a copy of the steamclient toc to make patches and jumps a lot more seamless
    uint32_t condenstation_toc = get_toc_base() - 0x8000;
    uint32_t steamclient_toc = TOC_PTR(cmsgclientlogon_vt->SerializeWithCachedSizes) - 0x8000; // use the vtable function ptr as a place to get it from
    _sys_printf("copying steamclient toc from %08x to %08x\n", steamclient_toc, condenstation_toc);
    PS3_WriteMemory(condenstation_toc, (void *)steamclient_toc, 0x800);
    PS3_SetPluginTOCBase(get_toc_base());

    // find the functions required to hook CM addresses
    uint32_t ccminterfaceconnect = SCUtils_FindCCMInterfaceConnect();
    _sys_printf("CCMInterface::Connect = %08x\n", ccminterfaceconnect);
    uint32_t addmultipletotail = SCUtils_GetFirstBranchTargetAfterInstruction(ccminterfaceconnect, 0x7c8407b4, 120);
    _sys_printf("CUtlVector<>::AddMultipleToTail = %08x\n", addmultipletotail);
    //uint32_t basyncconnect = SCUtils_GetFirstBranchTargetAfterInstruction(ccminterfaceconnect, 0x38e0004b, 150);
    //_sys_printf("CCMConnection::BAsyncConnect = %08x\n", basyncconnect);
    HookFunction(addmultipletotail, FUNC_PTR(CUtlVector_AddMultipleToTail_netadr), FUNC_PTR(CUtlVector_AddMultipleToTail_netadr_hook));

    // find useful protobuf functions that we need
    // can be found by checking for the first bl in CMsgClientLogon::GetTypeName
    std_basic_string_constructor_stub.func = SCUtils_GetFirstBranchTargetAfterInstruction(FUNC_PTR(cmsgclientlogon_vt->GetTypeName), 0, 8);
    std_basic_string_constructor_stub.toc = steamclient_toc;
    // can be found by checking for the first bl in CMsgClientLogon::ByteSize
    CodedOutputStream_VarintSize32Fallback_stub.func = SCUtils_GetFirstBranchTargetAfterInstruction(FUNC_PTR(cmsgclientlogon_vt->ByteSize), 0, 26);
    CodedOutputStream_VarintSize32Fallback_stub.toc = steamclient_toc;
    // can be found by checking for the bl in CMsgClientLogon::SerializeWithCachedSizes after setting r3 to 6 (client_language)
    WireFormatLite_WriteString_stub.func = SCUtils_GetFirstBranchTargetAfterInstruction(FUNC_PTR(cmsgclientlogon_vt->SerializeWithCachedSizes), LI(3, 6), 104);
    WireFormatLite_WriteString_stub.toc = steamclient_toc;

    // keep track of our original function ptr from the vtable
    CMsgClientLogon_SerializeWithCachedSizes = cmsgclientlogon_vt->SerializeWithCachedSizes;
    CMsgClientLogon_ByteSize = cmsgclientlogon_vt->ByteSize;
    CMsgClientLogonResponse_MergePartialFromCodedStream = cmsgclientlogonresponse_vt->MergePartialFromCodedStream;
    // since steamclient protobufs don't save/restore r2 before calling, use stubs to set r2 first
    PS3_Write32(&(cmsgclientlogon_vt->SerializeWithCachedSizes), (uint32_t)SetTOCFor_CMsgClientLogon_SerializeWithCachedSizes_Hook);
    PS3_Write32(&(cmsgclientlogon_vt->ByteSize), (uint32_t)SetTOCFor_CMsgClientLogon_ByteSize_Hook);
    PS3_Write32(&(cmsgclientlogonresponse_vt->MergePartialFromCodedStream), (uint32_t)SetTOCFor_CMsgClientLogonResponse_MergePartialFromCodedStream_Hook);

    // patch strings used to fetch avatar urls
    SCUtils_ReplaceString("http://media.steampowered.com/steamcommunity/public/", "http://avatars.steamstatic.com/");
    SCUtils_ReplaceString("images/avatars/%.2s/%s.jpg", "%s.jpg");
    SCUtils_ReplaceString("images/avatars/%.2s/%s_full.jpg", "%s_full.jpg");
    SCUtils_ReplaceString("images/avatars/%.2s/%s_medium.jpg", "%s_medium.jpg");

    // find the global CSteamEngine and CSteamEngine::InitCDNCache, re-init the cache with the new URLs
    uint32_t g_pSteamEngineAddr = SCUtils_FindSteamEngine();
    _sys_printf("g_pSteamEngineAddr = %08x\n", g_pSteamEngineAddr);
    CSteamEngine_InitCDNCache_stub.func = SCUtils_FindCSteamEngineInitCDNCache();
    _sys_printf("CSteamEngine::InitCDNCache = %08x\n", CSteamEngine_InitCDNCache_stub.func);
    CSteamEngine_InitCDNCache_stub.toc = steamclient_toc;
    CSteamEngine_InitCDNCache((void *)g_pSteamEngineAddr);

    // find and hook CUser::LogOn's vtable entry
    if (!use_v2_cmsgclientlogon) {
        uint32_t cuserLogon = SCUtils_FindCUserLogOn();
        _sys_printf("CUser::LogOn vtable entry = %08x\n", cuserLogon);
        CUser_LogOn = (CUser_LogOn_t)*(uint32_t *)cuserLogon; // :(
        PS3_Write32(cuserLogon, (uint32_t)CUser_LogOn_Hook);
    }

    sys_ppu_thread_t cmrequest_thread;
    sys_ppu_thread_create(&cmrequest_thread, get_cm_thread, NULL, 1024, 0x8000, SYS_PPU_THREAD_CREATE_JOINABLE, "condenstation_CMList");

    sys_ppu_thread_t shill_thread;
    sys_ppu_thread_create(&shill_thread, shillthread, NULL, 1024, 0x8000, SYS_PPU_THREAD_CREATE_JOINABLE, "condenstation_shill");
}

extern void *get_CCondenstationServerPlugin(); // defined in CCondenstationServerPlugin.cpp
void *CreateInterface(const char *name, int *returnCode) {
    // we only implement v1 of IServerPluginCallbacks
    if (strcmp(name, "ISERVERPLUGINCALLBACKS001") == 0)
        return get_CCondenstationServerPlugin();
    return NULL;
}

int _prx_start(unsigned int args, unsigned int *argp)
{
    _sys_printf("PRX opened!\n");

    // when loaded via the Source engine addons loader, we'll have to provide a CreateInterface function
    if (args >= sizeof(PS3_LoadAppSystemInterface_Parameters_t)) {
        PS3_LoadAppSystemInterface_Parameters_t *params = (PS3_LoadAppSystemInterface_Parameters_t *)argp;
        if (params->size >= sizeof(PS3_LoadAppSystemInterface_Parameters_t))
            params->createInterface = CreateInterface;
    }

    // make sure memory read/write APIs are available and choose whether to use DEX or CEX syscalls
    // TODO(Emma): add warning if no syscalls available (unlikely but possible)
    PS3_MemoryWriteCheck();

    // steamclient_ps3 is always loaded at prx entrypoint when using addons folder on Portal 2
    // MAYBE TODO(Emma): this will need to be adjusted to wait for steamclient to be linked if using SPRX loaded at eboot entry
    apply_steamclient_patches();

    return SYS_PRX_RESIDENT;
}

int _prx_stop()
{
    // ideally we would remove all our hooks when the PRX unloads but idc about that rn
    _sys_printf("PRX is unloading! SteamClient will be unstable!\n");
    return CELL_OK;
}
