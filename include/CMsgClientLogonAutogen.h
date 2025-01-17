#ifndef CMSGCLIENTLOGONAUTOGEN_H_
#define CMSGCLIENTLOGONAUTOGEN_H_

#include <stdint.h>
#include <stdbool.h>
#include "std_string.h"
#include "steamclient_protobuf_vtable.h"

// Portal 2 1.04 has this as a 0x90 byte sized object
//     seems to match Feb 7 2011
//     https://github.com/SteamRE/SteamKit/blob/533ceb4f278547f26dc6380b47179b148cc30915/Resources/Protobufs/steammessages_clientserver.proto
// CSGO 1.01 steamclient_ps3.sprx has this as a 0xA8 byte sized object
//     seems to match(?) Sep 9 2011
//     https://github.com/SteamRE/SteamKit/blob/3623e0839e3a325bb8ac80e0732297db0e1aedd5/Resources/Protobufs/steamclient/steammessages_clientserver.proto
//     we don't care about any of the new fields *but* we care about the enabled fields bitmask (for now)
typedef struct _CMsgClientLogonAutogen {
    sc_protobuf_vtable_t *vtable; // 0x0
    int length; // 0x4
    int protocol_version; // 0x8
    uint32_t obfustucated_private_ip; // 0xc
    int cell_id; // 0x10
    int last_session_id; // 0x14
    int client_package_version; // 0x18
    std_basic_string *client_language; // 0x1c
    int client_os_type; // 0x20
    bool should_remember_password; // 0x24
    std_basic_string *wine_version; // 0x28
    uint32_t public_ip; // 0x2C
    int qos_level; // 0x30
    int unknown1; // 0x34
    uint64_t client_supplied_steamid; // 0x38
    std_basic_string *machine_id; // 0x40
    std_basic_string *steam2_auth_ticket; // 0x44
    std_basic_string *email_address; // 0x48
    uint32_t rtime32_account_creation; // 0x4C
    std_basic_string *account_name; // 0x50
    std_basic_string *password; // 0x54
    std_basic_string *login_key; // 0x58
    std_basic_string *sony_psn_ticket; // 0x5C
    std_basic_string *sony_psn_service_id; // 0x60
    bool create_new_psn_linked_account_if_needed; // 0x64
    std_basic_string *sony_psn_name; // 0x68
    bool was_converted_deprecated_msg; // 0x6C
    std_basic_string *anon_user_target_account_name; // 0x70
    int unknown2; // 0x74
    uint64_t resolved_user_steam_id; // 0x78
    int eresult_sentryfile; // 0x80
    std_basic_string *sha_sentryfile; // 0x84
    std_basic_string *auth_code; // 0x88
#ifndef NEW_CLIENTLOGON
    uint32_t enabled_fields; // 0x8C
#else
    int32_t otp_type; // 0x8C
    uint32_t otp_value; // 0x90
    std_basic_string *otp_identifier; // 0x94
    bool steam2_ticket_request; // 0x98
    int32_t game_server_app_id; // 0x9C
    uint32_t enabled_fields; // 0xA0
    uint32_t enabled_fields_2; // 0xA4
#endif
} CMsgClientLogonAutogen;

// for CSGO support we just detect if we are using the up to date SteamClient
typedef struct _CMsgClientLogonAutogenV2 {
    sc_protobuf_vtable_t *vtable; // 0x0
    int length; // 0x4
    int protocol_version; // 0x8
    uint32_t obfustucated_private_ip; // 0xc
    int cell_id; // 0x10
    int last_session_id; // 0x14
    int client_package_version; // 0x18
    std_basic_string *client_language; // 0x1c
    int client_os_type; // 0x20
    bool should_remember_password; // 0x24
    std_basic_string *wine_version; // 0x28
    uint32_t public_ip; // 0x2C
    int qos_level; // 0x30
    int unknown1; // 0x34
    uint64_t client_supplied_steamid; // 0x38
    std_basic_string *machine_id; // 0x40
    std_basic_string *steam2_auth_ticket; // 0x44
    std_basic_string *email_address; // 0x48
    uint32_t rtime32_account_creation; // 0x4C
    std_basic_string *account_name; // 0x50
    std_basic_string *password; // 0x54
    std_basic_string *login_key; // 0x58
    std_basic_string *sony_psn_ticket; // 0x5C
    std_basic_string *sony_psn_service_id; // 0x60
    bool create_new_psn_linked_account_if_needed; // 0x64
    std_basic_string *sony_psn_name; // 0x68
    bool was_converted_deprecated_msg; // 0x6C
    std_basic_string *anon_user_target_account_name; // 0x70
    int unknown2; // 0x74
    uint64_t resolved_user_steam_id; // 0x78
    int eresult_sentryfile; // 0x80
    std_basic_string *sha_sentryfile; // 0x84
    std_basic_string *auth_code; // 0x88
#ifdef OLD_CLIENTLOGON
    uint32_t enabled_fields; // 0x8C
#else
    int32_t otp_type; // 0x8C
    uint32_t otp_value; // 0x90
    std_basic_string *otp_identifier; // 0x94
    bool steam2_ticket_request; // 0x98
    int32_t game_server_app_id; // 0x9C
    uint32_t enabled_fields; // 0xA0
    uint32_t enabled_fields_2; // 0xA4
#endif
} CMsgClientLogonAutogenV2;

// in theory we could generate some mapping for this at runtime by parsing SerializeWithCachedSizes

#endif // CMSGCLIENTLOGONAUTOGEN_H_
