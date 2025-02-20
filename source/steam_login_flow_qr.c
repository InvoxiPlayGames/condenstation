#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <cell/cell_fs.h>

#include "cellHttpHelper.h"
#include "SteamAuthentication.h"
#include "steammessages_auth.steamclient.pb-c.h"

extern char SteamUsername[0x80];
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
    
    r = condenstation_init_cellHttp();
    _sys_printf("condenstation_init_cellHttp = %i\n", r);

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
