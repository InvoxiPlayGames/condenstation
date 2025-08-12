#include <stdint.h>
#include <sys/prx.h>
#include <string.h>

#include "memmem.h"
#include "ps3_utilities.h"
#include "steamclient_protobuf_vtable.h"

extern int _sys_printf(char *fmt, ...);
extern int _sys_sprintf(char *buf, char *fmt, ...);

typedef struct _seg_info {
    uint32_t addr;
    uint32_t length;
} seg_info;

static seg_info steamclient_seg[2];

uint32_t SCUtils_BaseAddress()
{
    return steamclient_seg[0].addr;
}

uint32_t SCUtils_BaseAddress_Data()
{
    return steamclient_seg[1].addr;
}

void SCUtils_Init()
{
    char filenameBuf[1024];
    sys_prx_segment_info_t segments[32];
    sys_prx_module_info_t modInfo = {0};
    modInfo.size = sizeof(modInfo);
    modInfo.filename = filenameBuf;
    modInfo.filename_size = sizeof(filenameBuf);
    modInfo.segments = segments;
    modInfo.segments_num = 32;

    // look up the steamclient module
    sys_prx_id_t prxId = sys_prx_get_module_id_by_name("steamclient_ps3", 0, NULL);
    sys_prx_get_module_info(prxId, 0, &modInfo);

    // keep track of our base addresses
    steamclient_seg[0].addr = (uint32_t)modInfo.segments[0].base;
    steamclient_seg[0].length = (uint32_t)modInfo.segments[0].memsz;
    steamclient_seg[1].addr = (uint32_t)modInfo.segments[1].base;
    steamclient_seg[1].length = (uint32_t)modInfo.segments[1].memsz;
    _sys_printf("steamclient_ps3(0): 0x%08x\n", steamclient_seg[0].addr);
    _sys_printf("steamclient_ps3(1): 0x%08x\n", steamclient_seg[1].addr);
}

// not a steamclient thing but who's counting?
void SCUtils_GetModuleAddresses(const char * module_name, size_t num_segments, uint32_t *seg_addrs, uint32_t *seg_sizes)
{
    char filenameBuf[1024];
    sys_prx_segment_info_t segments[32];
    sys_prx_module_info_t modInfo = {0};
    modInfo.size = sizeof(modInfo);
    modInfo.filename = filenameBuf;
    modInfo.filename_size = sizeof(filenameBuf);
    modInfo.segments = segments;
    modInfo.segments_num = 32;

    // look up the steamclient module
    sys_prx_id_t prxId = sys_prx_get_module_id_by_name(module_name, 0, NULL);
    sys_prx_get_module_info(prxId, 0, &modInfo);

    // keep track of our base addresses
    for (size_t i = 0; i < num_segments; i++) {
        seg_addrs[i] = (uint32_t)modInfo.segments[i].base;
        seg_sizes[i] = (uint32_t)modInfo.segments[i].memsz;
    }
}

void *SCUtils_LookupNID(uint32_t nid)
{
    char filenameBuf[1024];
    sys_prx_segment_info_t segments[32];
    sys_prx_module_info_v2_t modInfo = {0};
    modInfo.size = sizeof(modInfo);
    modInfo.filename = filenameBuf;
    modInfo.filename_size = sizeof(filenameBuf);
    modInfo.segments = segments;
    modInfo.segments_num = 32;

    // look up the steamclient module
    sys_prx_id_t prxId = sys_prx_get_module_id_by_name("steamclient_ps3", 0, NULL);
    int r = sys_prx_get_module_info(prxId, 0, &modInfo);
    if (r != 0)
        return 0;

    // check to make sure we actually have a libent struct and that it's valid
    sys_prx_libent32_t *libent = modInfo.libent_addr;
    if (libent == NULL)
        return 0;
    int num_libents = modInfo.libent_size / sizeof(sys_prx_libent32_t);
    if (num_libents < 1)
        return 0;
    if (libent[0].structsize != sizeof(sys_prx_libent32_t))
        return 0;

    // scan through the NID tables to see if we have the one we're looking for
    uint32_t found_nid_add = 0;
    for (int ent = 0; ent < num_libents; ent++) {
        int num_nids = libent[ent].nfunc + libent[ent].nvar + libent[ent].ntls;

        uint32_t *nidtable = (uint32_t *)libent[ent].nidtable;
        uint32_t *addtable = (uint32_t *)libent[ent].addtable;

        for (int i = 0; i < num_nids; i++) {
            //_sys_printf("NID %08x = %08x\n", nidtable[i], addtable[i]);
            if (nidtable[i] == nid)
                found_nid_add = addtable[i];
        }
    }
    return (void *)found_nid_add;
}

void *SCUtils_LookupSteamAPINID(uint32_t nid)
{
    char filenameBuf[1024];
    sys_prx_segment_info_t segments[32];
    sys_prx_module_info_v2_t modInfo = {0};
    modInfo.size = sizeof(modInfo);
    modInfo.filename = filenameBuf;
    modInfo.filename_size = sizeof(filenameBuf);
    modInfo.segments = segments;
    modInfo.segments_num = 32;

    // look up the steamclient module
    sys_prx_id_t prxId = sys_prx_get_module_id_by_name("steam_api_ps3", 0, NULL);
    int r = sys_prx_get_module_info(prxId, 0, &modInfo);
    if (r != 0)
        return 0;

    // check to make sure we actually have a libent struct and that it's valid
    sys_prx_libent32_t *libent = modInfo.libent_addr;
    if (libent == NULL)
        return 0;
    int num_libents = modInfo.libent_size / sizeof(sys_prx_libent32_t);
    if (num_libents < 1)
        return 0;
    if (libent[0].structsize != sizeof(sys_prx_libent32_t))
        return 0;

    // scan through the NID tables to see if we have the one we're looking for
    uint32_t found_nid_add = 0;
    for (int ent = 0; ent < num_libents; ent++) {
        int num_nids = libent[ent].nfunc + libent[ent].nvar + libent[ent].ntls;

        uint32_t *nidtable = (uint32_t *)libent[ent].nidtable;
        uint32_t *addtable = (uint32_t *)libent[ent].addtable;

        for (int i = 0; i < num_nids; i++) {
            //_sys_printf("NID %08x = %08x\n", nidtable[i], addtable[i]);
            if (nidtable[i] == nid)
                found_nid_add = addtable[i];
        }
    }
    return (void *)found_nid_add;
}

void SCUtils_ReplaceString(const char *source, const char *destination)
{
    int srclen = strlen(source) + 1;
    int dstlen = strlen(destination) + 1;
    void *ptr = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, source, srclen);
    if (ptr != NULL) {
        _sys_printf("patching \"%s\" @ %p to \"%s\"\n", source, ptr, destination);
        PS3_WriteMemory((uint32_t)ptr, destination, dstlen);
        _sys_printf("\"%s\"\n", (char *)ptr);
    }
}

sc_protobuf_vtable_t *SCUtils_GetProtobufVtable(const char *protobuf_name)
{
    // search for the protobuf name by itself - with a null space before and after - to find the string returned by GetTypeName
    // the null spaces avoid finding any RTTI names or related classes
    char proto_name_buffer[0x40];
    proto_name_buffer[0] = 0;
    strcpy(proto_name_buffer + 1, protobuf_name);
    proto_name_buffer[1 + strlen(protobuf_name)] = 0;
    void *proto_string = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, proto_name_buffer, strlen(protobuf_name) + 2);
    if (proto_string == NULL) {
        _sys_printf("failed to find proto string '%s'\n", protobuf_name);
        return NULL;
    }
    _sys_printf("found proto string at %p + 1\n", proto_string);
    uint32_t proto_string_addr = (uint32_t)proto_string + 1; // increment by 1 since we found a null byte before

    // build the part of the GetTypeName instruction that references this string and scans for it
    uint32_t get_type_name_instrs[3];
    get_type_name_instrs[0] = 0xF8010080; // std r0, 0x80(r1)
    get_type_name_instrs[1] = 0x3C800000 + ((proto_string_addr >> 16) & 0xFFFF); // lis r4, ?
    get_type_name_instrs[2] = 0x30840000 + (proto_string_addr & 0xFFFF); // addic/subic r4, r4, ?
    // if the lower halfword of the address is 0x8000 then it's a subic and will be subtracting off the first one
    if ((proto_string_addr & 0xFFFF) >= 0x8000)
        get_type_name_instrs[1] += 1;
    void *get_type_name_midfunc = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, get_type_name_instrs, sizeof(get_type_name_instrs));
    if (get_type_name_midfunc == NULL) {
        _sys_printf("failed to find type name midfunc\n");
        return NULL;
    }
    uint32_t get_type_name_addr = (uint32_t)get_type_name_midfunc - 8;

    // find the entry of GetTypeName in the steamclient TOC
    uint32_t get_type_name_toc_entry[1] = { get_type_name_addr };
    void *get_type_name_toc = own_memmem((void *)steamclient_seg[1].addr, steamclient_seg[1].length, get_type_name_toc_entry, sizeof(get_type_name_toc_entry));
    if (get_type_name_toc == NULL) {
        _sys_printf("failed to find type name toc entry\n");
        return NULL;
    }
    // ... then find a reference to that TOC entry in the main steamclient text segment to find the vtable
    uint32_t get_type_name_toc_addr = (uint32_t)get_type_name_toc;
    uint32_t protobuf_vtable_addrs[1] = { get_type_name_toc_addr };
    void *protobuf_vtable = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, protobuf_vtable_addrs, sizeof(protobuf_vtable_addrs));
    if (protobuf_vtable == NULL) {
        _sys_printf("failed to find vtable\n");
        return NULL;
    }

    // subtract 8 (since GetTypeName is the third entry) and we've got it!
    uint32_t protobuf_vtable_addr = (uint32_t)protobuf_vtable - 8;
    return (sc_protobuf_vtable_t *)protobuf_vtable_addr;
}

uint32_t SCUtils_GetFirstBranchTargetAfterInstruction(uint32_t start_address, uint32_t instruction, int scan_length)
{
    uint32_t *func_to_scan = (uint32_t *)start_address;
    bool waiting_for_instruction = instruction == 0 ? false : true;
    uint32_t branch_instruction = 0;
    uint32_t instruction_address = 0;

    // scan through the function to check for the first bl (or a prerequisite instruction, *then* the first bl after that)
    for (int i = 0; i < scan_length; i++)
    {
        uint32_t current_insr = func_to_scan[i];
        if (waiting_for_instruction) {
            if (current_insr == instruction) {
                _sys_printf("hit instruction we were trying to check for\n");
                waiting_for_instruction = false;
            }
        } else {
            // check if it's a linked branch
            if ((current_insr >> 26) == 18 && (current_insr & 0x48000001) == 0x48000001) {
                _sys_printf("hit bl %08x\n", current_insr);
                branch_instruction = current_insr;
                instruction_address = (uint32_t)(func_to_scan + i);
                break;
            }
        }
    }

    // nope out if we don't have an instruction
    if (branch_instruction == 0)
    {
        _sys_printf("failed to find branch instruction (e%i) after %i\n", waiting_for_instruction, scan_length);
        return 0;
    }

    // decode the branch instruction by getting the 24 bits necessary
    int branch_offset = (branch_instruction & 0x3FFFFFC);
    // check signedness
    if ((branch_offset & 0x2000000) == 0x2000000)
        branch_offset -= 0x4000000;
    
    return instruction_address + branch_offset;
}

uint32_t SCUtils_FindCCMInterfaceConnect()
{
    // find the pointer to the psn log message string
    const char *psn_string = "User is on PSN (%s), now connecting to Steam";
    void *psn_string_ptr = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, psn_string, strlen(psn_string));
    if (psn_string_ptr == NULL) {
        _sys_printf("failed to find psn string\n");
        return 0;
    }
    uint32_t psn_string_addr = (uint32_t)psn_string_ptr;

    // build the part of the function that references this string and scan for it
    uint32_t on_psn_notif_instrs[3];
    on_psn_notif_instrs[0] = 0x3C800000 + ((psn_string_addr >> 16) & 0xFFFF); // lis r4, ?
    on_psn_notif_instrs[1] = 0x30610134; // addic r3, r1, 0x134
    on_psn_notif_instrs[2] = 0x30840000 + (psn_string_addr & 0xFFFF); // addic/subic r4, r4, ?
    // if the lower halfword of the address is 0x8000 then it's a subic and will be subtracting off the first one
    if ((psn_string_addr & 0xFFFF) >= 0x8000)
        on_psn_notif_instrs[0] += 1;
    void *on_psn_notif_midfunc = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, on_psn_notif_instrs, sizeof(on_psn_notif_instrs));
    if (on_psn_notif_midfunc == NULL) {
        _sys_printf("failed to find psn notif midfunc\n");
        return NULL;
    }
    uint32_t on_psn_notif_addr = (uint32_t)on_psn_notif_midfunc;

    // find the branch to CCMInterface::Connect (preceded by ori r3, r31, 0x0)
    uint32_t ccminterface_connect = SCUtils_GetFirstBranchTargetAfterInstruction(on_psn_notif_addr, 0x63e30000, 26);

    return ccminterface_connect;
}

// TODO(Emma): make a more generic function finder function so you can find functions with just one function
uint32_t SCUtils_FindCSteamEngineInitCDNCache()
{
    // find part of the CSteamEngine::InitCDNCache function
    uint32_t init_cdn_cache_instrs[4] = {
        0x38800000, // li r4, 0
        0x98830000, // stb r4, 0x0(r3)
        0x30630001, // addic r3, r3, 0x1
        0x4800000c, // b 0xC - we shouldn't be doing this! but it's the same across all binaries, so it's okay
    };
    void *init_cdn_cache_midfunc = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, init_cdn_cache_instrs, sizeof(init_cdn_cache_instrs));
    if (init_cdn_cache_midfunc == NULL) {
        _sys_printf("can't find init cdn cache midfunc\n");
        return 0;
    }
    uint32_t *init_cdn_cache_midfunc_arr = (uint32_t *)init_cdn_cache_midfunc;
    // reverse backwards in the array to find the start of the function
    for (int i = 0; i < 40; i++) {
        // stdu r1, -0x280(r1) - stack size is the same between all binaries
        if (init_cdn_cache_midfunc_arr[-i] == 0xf821fd81)
            return (uint32_t)&(init_cdn_cache_midfunc_arr[-i]);
    }
    _sys_printf("failed to find start of init cdn cache\n");
    return 0;
}

uint32_t SCUtils_FindCUserLogOn() {
    // find the start of the CUser::GetConsoleSteamID
    // technically this isn't good because it's checking a hardcoded offset in CUser
    uint32_t get_console_steamid_instrs[4] = {
        0x88a40090, // lbz r5, 0x90(r4)
        0x88c40091, // lbz r6, 0x91(r4)
        0x88e40092, // lbz r7, 0x92(r4)
        0x89040093, // lbz r8, 0x93(r4)
    };
    void *get_console_steamid_func = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, get_console_steamid_instrs, sizeof(get_console_steamid_instrs));
    uint32_t get_console_steamid_addr = (uint32_t)get_console_steamid_func;

    // get the toc entry for this function
    uint32_t get_console_steamid_toc_entry[1] = { get_console_steamid_addr };
    void *get_console_steamid_toc = own_memmem((void *)steamclient_seg[1].addr, steamclient_seg[1].length, get_console_steamid_toc_entry, sizeof(get_console_steamid_toc_entry));
    uint32_t get_console_steamid_toc_addr = (uint32_t)get_console_steamid_toc;

    // find the CUser vtable from that toc entry
    uint32_t get_console_steamid_vt_entry[1] = { get_console_steamid_toc_addr };
    void *get_console_steamid_vt_pos = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, get_console_steamid_vt_entry, sizeof(get_console_steamid_vt_entry));
    uint32_t get_console_steamid_vt_addr = (uint32_t)get_console_steamid_vt_pos;

    // -0xC is CUser::LogOn in the CUser vtable
    return get_console_steamid_vt_addr - 0xC;
}

uint32_t SCUtils_FindSteamEngine()
{
    // find part of the CAppInfoCache::WriteToDisk function
    uint32_t write_to_disk_instrs[4] = {
        0x607f0000, // ori r31, r3, 0x0
        0x38800400, // li r4, 0x400
        0x63830000, // ori r3, r28, 0x0
        0x38a00400, // li r5, 0x400
    };
    void *write_to_disk_midfunc = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, write_to_disk_instrs, sizeof(write_to_disk_instrs));
    if (write_to_disk_midfunc == NULL) {
        _sys_printf("can't find write to disk midfunc");
        return 0;
    }
    uint32_t *write_to_disk_midfunc_arr = (uint32_t *)write_to_disk_midfunc;
    // run through the instructions until we find an instruction that happens right after the call to ISteamEngine::GetConnectedUniverse
    int i = 0;
    for (i = 0; i < 30; i++) {
        // rldicl r4, r3, 0, 32
        if (write_to_disk_midfunc_arr[i] == 0x78640020)
            break;
    }
    if (i >= 29) {
        _sys_printf("can't find rldicl after getconnecteduniverse\n");
        return 0;
    }
    // then go back to before that call to find where r3 gets set
    int j = 0;
    uint32_t g_pSteamEngineAddr = 0;
    for (j = 1; j < 6; j++) {
        // check if it's "lis r3, 0x????"
        if ((write_to_disk_midfunc_arr[i - j] & 0xFFFF0000) == 0x3c600000)
        {
            // extract the address reference from this
            uint16_t hi = (uint16_t)(write_to_disk_midfunc_arr[i - j] & 0xFFFF); // lis
            uint16_t lo = (uint16_t)(write_to_disk_midfunc_arr[i - j + 1] & 0xFFFF); // addic/subic
            if (lo >= 0x8000) hi--; // remove 1 from hi if it's a subic
            g_pSteamEngineAddr = (hi << 16) | lo;
            break;
        }
    }

    return g_pSteamEngineAddr;
}
