#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <cell/cell_fs.h>

#include "condenstation_logger.h"
#include "cellHttpHelper.h"
#include "SteamAuthentication.h"
#include "steammessages_auth.steamclient.pb-c.h"

extern char SteamAccountName[0x80];
extern char SteamAccessToken[0x400];

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

void test_steamauthentication_username_and_password() {
    int r = 0;
    
    r = condenstation_init_cellHttp();
    cdst_log("condenstation_init_cellHttp = %i\n", r);

    uint8_t msg_buffer[0x400];
    size_t msg_size = 0;
    uint8_t out_buffer[0x800];
    size_t out_size = sizeof(out_buffer);

    uint8_t shitalloc_buffer[0x800];
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

    CAuthenticationGetPasswordRSAPublicKeyRequest pubkeyreq;
    cauthentication__get_password_rsapublic_key__request__init(&pubkeyreq);
    pubkeyreq.account_name = "xxxxxx";

    msg_size = cauthentication__get_password_rsapublic_key__request__get_packed_size(&pubkeyreq);
    cdst_log("msg_size = %i\n", msg_size);

    cauthentication__get_password_rsapublic_key__request__pack(&pubkeyreq, msg_buffer);
    out_size = sizeof(out_buffer);
    SteamAuthenticationRPC(STEAMAUTH_GET, "GetPasswordRSAPublicKey", 1, msg_buffer, msg_size, out_buffer, &out_size);

    cdst_log("out_size = %i\n", out_size);
    hexdump(out_buffer, out_size);

    CAuthenticationGetPasswordRSAPublicKeyResponse *resp = 
        cauthentication__get_password_rsapublic_key__response__unpack(&shitalloc, out_size, out_buffer);
    
    if (resp != NULL) {
        if (resp->has_timestamp)    
            cdst_log("timestamp: %llu\n", resp->timestamp);
        if (resp->publickey_exp != NULL)
            cdst_log("publickey_exp: %s\n", resp->publickey_exp);
        if (resp->publickey_mod != NULL)
            cdst_log("publickey_mod: %s\n", resp->publickey_mod);
    }
}
