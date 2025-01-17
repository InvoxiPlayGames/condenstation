#include <sys/prx.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <sys/timer.h>

#include <cell/cell_fs.h>

#include <stdio.h>
#include <string.h>
#include <ppcasm.h>

#include "inih.h"
#include "ps3_utilities.h"
#include "CMsgClientLogonAutogen.h"
#include "steamclient_utils.h"
#include "steamclient_google_protobufs.h"
#include "valve_ps3.h"
#include "SteamAuthentication.h"
#include "steammessages_auth.steamclient.pb-c.h"
#include "steamclient_steamps3params.h"

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

char SteamCM[0x40];
char SteamUsername[0x80];
char SteamAccessToken[0x400];

std_basic_string username_bs;

// function pointers that we need to call from within our hooks
CodedOutputStream_VarintSize32Fallback_t CodedOutputStream_VarintSize32Fallback;
WireFormatLite_WriteString_t WireFormatLite_WriteString;
std_basic_string_constructor_t std_basic_string_constructor;
// we need "toc stub"s for these functions because we are setting by address rather than setting from other function pointers
toc_stub std_basic_string_constructor_stub;
toc_stub CodedOutputStream_VarintSize32Fallback_stub;
toc_stub WireFormatLite_WriteString_stub;

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
        std_basic_string_constructor(&username_bs, SteamUsername);
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

static int INIHandler(void *user, const char *section, const char *name, const char *value) {
    //_sys_printf("[%s] %s = %s\n", section, name, value);
    if (strcmp(section, "Account") == 0) {
        if (strcmp(name, "Username") == 0)
            strncpy(SteamUsername, value, sizeof(SteamUsername));
        else if (strcmp(name, "RefreshToken") == 0)
            strncpy(SteamAccessToken, value, sizeof(SteamAccessToken));
    } else if (strcmp(section, "Server") == 0) {
        if (strcmp(name, "CM") == 0)
            strncpy(SteamCM, value, sizeof(SteamCM));
    }
    return 1;
}

// parse the user info out of our config file
bool parse_config_file(const char *file_path)
{
    int fd = -1;
    CellFsErrno r = cellFsOpen(file_path, CELL_FS_O_RDONLY, &fd, NULL, 0);
    if (r != CELL_FS_SUCCEEDED)
        return false;
    char configBuffer[4096];
    memset(configBuffer, 0, sizeof(configBuffer));
    uint64_t bytesRead = 0;
    cellFsRead(fd, configBuffer, sizeof(configBuffer), &bytesRead);
    cellFsClose(fd);
    if (ini_parse_string(configBuffer, INIHandler, NULL) < 0)
        return false;
    _sys_printf("parsed ini successfully!\n");
    _sys_printf("SteamUsername = %s\n", SteamUsername);
    // THIS MIGHT GET BROADCAST OVER UDP ON YOUR NETWORK LOL
    //_sys_printf("SteamAccessToken = %s\n", SteamAccessToken);
    _sys_printf("SteamCM = %s\n", SteamCM);
    return true;
}

typedef struct _shitalloc_data {
    uint8_t *buffer;
    size_t allocated;
    size_t max_size;
} shitalloc_data;

static void *shitalloc_alloc(shitalloc_data *data, size_t len) {
    void *buf = data->buffer + data->allocated;
    data->allocated += len;
    return buf;
}

static void shitalloc_free(shitalloc_data *data, void *buf) {
    return; //hahahahha you thought
}

void save_credentials(const char *file_path) {
    char inifile[0x1000];
    int fd = -1;
    snprintf(inifile, sizeof(inifile), "[Account]\nUsername=%s\nRefreshToken=%s\n", SteamUsername, SteamAccessToken);
    CellFsErrno r = cellFsOpen(file_path, CELL_FS_O_WRONLY, &fd, NULL, 0);
    if (r != CELL_FS_SUCCEEDED)
        return false;
    uint64_t bytesWrite = 0;
    cellFsWrite(fd, inifile, strlen(inifile), &bytesWrite);
    cellFsClose(fd);
}

// fully self-contained authentication test
// TODO(Emma):
//   - Render QR code on-screen
//   - Handle failure
//   - Do it triggered by user-action
void test_steamauthentication() {
    int r = 0;
    
    r = SteamAuthenticationInit();
    _sys_printf("SteamAuthenticationInit = %i\n", r);

    
    uint8_t msg_buffer[0x400];
    size_t msg_size = 0;
    uint8_t out_buffer[0x800];
    size_t out_size = sizeof(out_buffer);

    uint8_t shitalloc_buffer[0x400];
    shitalloc_data allocdata = {
        .allocated = 0,
        .max_size = sizeof(shitalloc_buffer),
        .buffer = shitalloc_buffer
    };
    ProtobufCAllocator shitalloc = {
        .alloc = shitalloc_alloc,
        .free = shitalloc_free,
        .allocator_data = &allocdata
    };

    CAuthenticationDeviceDetails details;
    cauthentication__device_details__init(&details);
    // TODO: the device friendly name should use the PS3's name
    details.device_friendly_name = "condenstation on PS3";
    details.has_os_type = true;
    details.os_type = 20; // Windows 11 - for PS3, it's -300.
    details.has_platform_type = true;
    details.platform_type = EAUTH_TOKEN_PLATFORM_TYPE__k_EAuthTokenPlatformType_SteamClient;
    
    CAuthenticationBeginAuthSessionViaQRRequest req;
    cauthentication__begin_auth_session_via_qr__request__init(&req);
    req.device_friendly_name = "condenstation on PS3";
    req.has_platform_type = true;
    req.platform_type = EAUTH_TOKEN_PLATFORM_TYPE__k_EAuthTokenPlatformType_SteamClient;
    req.website_id = "Client";
    req.device_details = &details;

    msg_size = cauthentication__begin_auth_session_via_qr__request__get_packed_size(&req);
    _sys_printf("msg_size = %i\n", msg_size);

    cauthentication__begin_auth_session_via_qr__request__pack(&req, msg_buffer);

    out_size = sizeof(out_buffer);
    SteamAuthenticationRPC(STEAMAUTH_POST, "BeginAuthSessionViaQR", 1, msg_buffer, msg_size, out_buffer, &out_size);

    _sys_printf("out_size = %i\n", out_size);
    hexdump(out_buffer, out_size);

    CAuthenticationBeginAuthSessionViaQRResponse *resp = 
        cauthentication__begin_auth_session_via_qr__response__unpack(&shitalloc, out_size, out_buffer);

    if (resp->has_client_id)
        _sys_printf("client id = %llu\n", resp->client_id);
    if (resp->challenge_url != NULL)
        _sys_printf("challenge url = %s\n", resp->challenge_url);
    if (resp->has_request_id) {
        _sys_printf("request id = ");
        hexdump(resp->request_id.data, resp->request_id.len);
    }
    if (resp->has_interval)
        _sys_printf("interval = %.2f\n", resp->interval);
    if (resp->has_version)
        _sys_printf("version = %i\n", resp->version);
    if (resp->n_allowed_confirmations > 0) {
        for (int i = 0; i < resp->n_allowed_confirmations; i++) {
            _sys_printf("allowed confirmation [%i] type = %i\n", i, resp->allowed_confirmations[i]->confirmation_type);
        }
    }

    uint64_t current_client_id = resp->client_id;
    uint8_t current_request_id[0x20];
    size_t request_id_size = resp->request_id.len;
    memcpy(current_request_id, resp->request_id.data, request_id_size);

    // clear our shitalloc
    resp = NULL;
    memset(shitalloc_buffer, 0, sizeof(shitalloc_buffer));
    allocdata.allocated = 0;

    bool auth_session_running = true;

    while (auth_session_running) {
        sys_timer_sleep(5);

        CAuthenticationPollAuthSessionStatusRequest pollreq;
        cauthentication__poll_auth_session_status__request__init(&pollreq);
        pollreq.client_id = current_client_id;
        pollreq.has_client_id = true;

        pollreq.request_id.data = current_request_id;
        pollreq.request_id.len = request_id_size;
        pollreq.has_request_id = true;

        msg_size = cauthentication__poll_auth_session_status__request__get_packed_size(&pollreq);
        _sys_printf("msg_size = %i\n", msg_size);

        msg_size = cauthentication__poll_auth_session_status__request__pack(&pollreq, msg_buffer);
        // if the buffer has a trailing zero then get rid of it
        if (msg_buffer[msg_size - 1] == 0)
            msg_size -= 1;

        out_size = sizeof(out_buffer);
        SteamAuthenticationRPC(STEAMAUTH_POST, "PollAuthSessionStatus", 1, msg_buffer, msg_size, out_buffer, &out_size);

        _sys_printf("out_size = %i\n", out_size);
        //hexdump(out_buffer, out_size);

        CAuthenticationPollAuthSessionStatusResponse *pollresp = 
            cauthentication__poll_auth_session_status__response__unpack(&shitalloc, out_size, out_buffer);
        
        if (pollresp->has_new_client_id) {
            _sys_printf("new client id: %llu\n", pollresp->new_client_id);
            current_client_id = pollresp->new_client_id;
        }
        if (pollresp->new_challenge_url != NULL) {
            _sys_printf("new challenge url: %s\n", pollresp->new_challenge_url);
        }
        if (pollresp->account_name != NULL && pollresp->access_token != NULL) {
            _sys_printf("we are LOGGED IN CHAT!!\n");
            _sys_printf("account name: %s\n", pollresp->account_name);
            //_sys_printf("access token: snip\n");
            //_sys_printf("refresh token: snip\n");
            strncpy(SteamUsername, pollresp->account_name, sizeof(SteamUsername));
            strncpy(SteamAccessToken, pollresp->refresh_token, sizeof(SteamAccessToken));
            auth_session_running = false;
            save_credentials("/dev_hdd0/tmp/condenstation.ini");
        }
    }
}

steamclient_GetSteamPS3Params_t steamclient_GetSteamPS3Params;

SteamPS3ParamsInternal_t fakeInternal;

void apply_steamclient_patches()
{
    //sys_ppu_thread_t loginThread;
    //sys_ppu_thread_create(&loginThread, test_steamauthentication, NULL, 1024, 0x8000, SYS_PPU_THREAD_CREATE_JOINABLE, "test_steamauthentication");

    // initialise our SteamClient utility library
    SCUtils_Init();

    // detect the current game by looking at the appid passed into steamclient
    steamclient_GetSteamPS3Params = SCUtils_LookupNID(NID_steamclient_GetSteamPS3Params);
    SteamPS3Params_t *params = steamclient_GetSteamPS3Params();
    use_v2_cmsgclientlogon = (params->m_unVersion == STEAM_PS3_CSGO_PARAMS_VER);
    if (params->m_unVersion == STEAM_PS3_PORTAL2_PARAMS_VER)
        _sys_printf("portal 2!\n");
    else if (params->m_unVersion == STEAM_PS3_CSGO_PARAMS_VER)
        _sys_printf("csgo!\n");
    else
        _sys_printf("what?\n");

    // load our config file
    parse_config_file("/dev_hdd0/tmp/condenstation.ini");

    // TODO: patching CM addresses

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


    uint32_t steamclient_base = SCUtils_BaseAddress();
    // can be found by checking for the first bl in CMsgClientLogon::GetTypeName
    std_basic_string_constructor_stub.func = SCUtils_GetFirstBranchTargetAfterInstruction(FUNC_PTR(cmsgclientlogon_vt->GetTypeName), 0, 8);
    std_basic_string_constructor_stub.toc = steamclient_toc;
    std_basic_string_constructor = &std_basic_string_constructor_stub;
    // can be found by checking for the first bl in CMsgClientLogon::ByteSize
    CodedOutputStream_VarintSize32Fallback_stub.func = SCUtils_GetFirstBranchTargetAfterInstruction(FUNC_PTR(cmsgclientlogon_vt->ByteSize), 0, 26);
    CodedOutputStream_VarintSize32Fallback_stub.toc = steamclient_toc;
    CodedOutputStream_VarintSize32Fallback = &CodedOutputStream_VarintSize32Fallback_stub;
    // can be found by checking for the bl in CMsgClientLogon::SerializeWithCachedSizes after setting r3 to 6 (client_language)
    WireFormatLite_WriteString_stub.func = SCUtils_GetFirstBranchTargetAfterInstruction(FUNC_PTR(cmsgclientlogon_vt->SerializeWithCachedSizes), LI(3, 6), 104);
    WireFormatLite_WriteString_stub.toc = steamclient_toc;
    WireFormatLite_WriteString = &WireFormatLite_WriteString_stub;

    // keep track of our original function ptr from the vtable
    CMsgClientLogon_SerializeWithCachedSizes = cmsgclientlogon_vt->SerializeWithCachedSizes;
    CMsgClientLogon_ByteSize = cmsgclientlogon_vt->ByteSize;

    // create an r2-fixing jump to our hook function, then overwrite the vtable
    uint32_t jump_tramp_1 = PS3_CreateTrampoline(FUNC_PTR(CMsgClientLogon_SerializeWithCachedSizes_Hook));
    PS3_Write32((uint32_t)CMsgClientLogon_SerializeWithCachedSizes_Hook, jump_tramp_1);
    PS3_Write32(&(cmsgclientlogon_vt->SerializeWithCachedSizes), (uint32_t)CMsgClientLogon_SerializeWithCachedSizes_Hook);

    // create an r2-fixing jump to our hook function, then overwrite the vtable
    uint32_t jump_tramp_2 = PS3_CreateTrampoline(FUNC_PTR(CMsgClientLogon_ByteSize_Hook));
    PS3_Write32((uint32_t)CMsgClientLogon_ByteSize_Hook, jump_tramp_2);
    PS3_Write32(&(cmsgclientlogon_vt->ByteSize), (uint32_t)CMsgClientLogon_ByteSize_Hook);
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
    // TODO(Emma): add all the stuff for RPCS3 support from RB3Enhanced and add warning if no syscalls available (unlikely but possible)
    PS3_MemoryWriteCheck();

    // steamclient_ps3 is always loaded at prx entrypoint when using addons folder on Portal 2
    // TODO(Emma): this will need to be adjusted to wait for steamclient to be linked if using SPRX loaded at eboot entry
    apply_steamclient_patches();

    return SYS_PRX_RESIDENT;
}

int _prx_stop()
{
    // ideally we would remove all our hooks when the PRX unloads but idc about that rn
    _sys_printf("PRX is unloading! SteamClient will be unstable!\n");
    return CELL_OK;
}
