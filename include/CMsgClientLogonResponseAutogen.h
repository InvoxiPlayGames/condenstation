#ifndef CMSGCLIENTLOGONRESPONSEAUTOGEN_H_
#define CMSGCLIENTLOGONRESPONSEAUTOGEN_H_

#include <stdint.h>
#include <stdbool.h>
#include "std_string.h"
#include "steamclient_protobuf_vtable.h"

// Portal 2 1.04 has this as a 0x34 byte sized object
//     seems to match Feb 7 2011
//     https://github.com/SteamRE/SteamKit/blob/533ceb4f278547f26dc6380b47179b148cc30915/Resources/Protobufs/steammessages_clientserver.proto
// CSGO 1.01 steamclient_ps3.sprx has this as a 0x3c byte sized object
//     seems to match(?) Sep 9 2011
//     https://github.com/SteamRE/SteamKit/blob/3623e0839e3a325bb8ac80e0732297db0e1aedd5/Resources/Protobufs/steamclient/steammessages_clientserver.proto
//     we don't care about any of the new fields *but* we care about the enabled fields bitmask (for now)
typedef struct _CMsgClientLogonResponseAutogen {
    sc_protobuf_vtable_t *vtable; // 0x0
    int length; // 0x4
    int eresult; // 0x8
    int out_of_game_heartbeat_seconds; // 0xc
    int in_game_heartbeat_seconds; // 0x10
    uint32_t public_ip; // 0x14
    int rtime32_server_time; // 0x18
    uint32_t account_flags; // 0x1c
    uint32_t cell_id; // 0x20
#ifndef NEW_CLIENTLOGON
    int unknown1; // 0x24
    uint64_t client_supplied_steamid; // 0x28
    int enabled_fields; // 0x30
#else
    std_basic_string *email_domain; // 0x24
    std_basic_string *steam2_ticket; // 0x28
    int eresult_extended; // 0x2C
    uint64_t client_supplied_steamid; // 0x30
    int enabled_fields; // 0x38
#endif
} CMsgClientLogonResponseAutogen;

// for CSGO support we just detect if we are using the up to date SteamClient
typedef struct _CMsgClientLogonResponseAutogenV2 {
    sc_protobuf_vtable_t *vtable; // 0x0
    int length; // 0x4
    int eresult; // 0x8
    int out_of_game_heartbeat_seconds; // 0xc
    int in_game_heartbeat_seconds; // 0x10
    uint32_t public_ip; // 0x14
    int rtime32_server_time; // 0x18
    uint32_t account_flags; // 0x1c
    uint32_t cell_id; // 0x20
#ifdef OLD_CLIENTLOGON
    int unknown1; // 0x24
    uint64_t client_supplied_steamid; // 0x28
    int enabled_fields; // 0x30
#else
    std_basic_string *email_domain; // 0x24
    std_basic_string *steam2_ticket; // 0x28
    int eresult_extended; // 0x2C
    uint64_t client_supplied_steamid; // 0x30
    int enabled_fields; // 0x38
#endif
} CMsgClientLogonResponseAutogenV2;

// in theory we could generate some mapping for this at runtime by parsing SerializeWithCachedSizes

#endif // CMSGCLIENTLOGONRESPONSEAUTOGEN_H_
