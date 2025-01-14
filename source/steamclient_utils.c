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

    sys_prx_id_t prxId = sys_prx_get_module_id_by_name("steamclient_ps3", 0, NULL);
    
    sys_prx_get_module_info(prxId, 0, &modInfo);

    steamclient_seg[0].addr = (uint32_t)modInfo.segments[0].base;
    steamclient_seg[0].length = (uint32_t)modInfo.segments[0].memsz;
    steamclient_seg[1].addr = (uint32_t)modInfo.segments[1].base;
    steamclient_seg[1].length = (uint32_t)modInfo.segments[1].memsz;
    _sys_printf("steamclient_ps3(0): 0x%08x\n", steamclient_seg[0].addr);
    _sys_printf("steamclient_ps3(1): 0x%08x\n", steamclient_seg[1].addr);
}

sc_protobuf_vtable_t *SCUtils_GetProtobufVtable(const char *protobuf_name)
{
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
    uint32_t proto_string_addr = (uint32_t)proto_string + 1;

    uint32_t get_type_name_instrs[3] = {
        0xF8010080, // std r0, 0x80(r1)
        0x3C800000 + ((proto_string_addr >> 16) & 0xFFFF) + 1, // lis r4, ?
        0x30840000 + (proto_string_addr & 0xFFFF), // subic r4, r4, ?
    };
    
    void *get_type_name_midfunc = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, get_type_name_instrs, sizeof(get_type_name_instrs));
    if (get_type_name_midfunc == NULL) {
        _sys_printf("failed to find type name midfunc\n");
        return NULL;
    }

    uint32_t get_type_name_addr = (uint32_t)get_type_name_midfunc - 8;

    uint32_t get_type_name_toc_entry[1] = { get_type_name_addr };
    void *get_type_name_toc = own_memmem((void *)steamclient_seg[1].addr, steamclient_seg[1].length, get_type_name_toc_entry, sizeof(get_type_name_toc_entry));
    if (get_type_name_toc == NULL) {
        _sys_printf("failed to find type name toc entry\n");
        return NULL;
    }

    uint32_t get_type_name_toc_addr = (uint32_t)get_type_name_toc;
    uint32_t protobuf_vtable_addrs[1] = { get_type_name_toc_addr };
    void *protobuf_vtable = own_memmem((void *)steamclient_seg[0].addr, steamclient_seg[0].length, protobuf_vtable_addrs, sizeof(protobuf_vtable_addrs));
    if (protobuf_vtable == NULL) {
        _sys_printf("failed to find vtable\n");
        return NULL;
    }

    uint32_t protobuf_vtable_addr = (uint32_t)protobuf_vtable - 8;

    return (sc_protobuf_vtable_t *)protobuf_vtable_addr;
}
