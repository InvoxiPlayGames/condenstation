#include <stdint.h>
#include <sys/prx.h>
#include <string.h>

#include "memmem.h"
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
    // TODO(Emma): THIS DOESN'T WORK!
    int srclen = strlen(source) + 1;
    int dstlen = strlen(destination) + 1;
    for (uint32_t i = 0; i < steamclient_seg[0].length - dstlen; i++)
    {
        if (memcmp((void *)(steamclient_seg[0].addr + i), source, srclen) == 0)
        {
            memcpy((void *)(steamclient_seg[0].addr + i), destination, dstlen);
            return;
        }
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
                _sys_printf("hit blr %08x\n", current_insr);
                branch_instruction = current_insr;
                instruction_address = (uint32_t)(func_to_scan + i);
                break;
            }
        }
    }

    // nope out if we don't have an instruction
    if (branch_instruction == 0)
    {
        _sys_printf("failed to find branch instruction (e%i)\n", waiting_for_instruction);
        return 0;
    }

    // decode the branch instruction by getting the 24 bits necessary
    int branch_offset = (branch_instruction & 0x3FFFFFC);
    // check signedness
    if ((branch_offset & 0x2000000) == 0x2000000)
        branch_offset -= 0x4000000;
    
    return instruction_address + branch_offset;
}
