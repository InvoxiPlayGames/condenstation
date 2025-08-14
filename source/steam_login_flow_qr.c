#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <sysutil/sysutil_msgdialog.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_sysparam.h>
#include <cell/cell_fs.h>

#include "condenstation_logger.h"
#include "condenstation_config.h"
#include "cellHttpHelper.h"
#include "SteamAuthentication.h"
#include "steammessages_auth.steamclient.pb-c.h"

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

bool auth_session_running = false;
bool auth_session_success = false;

// hooks_overlay.c
void QRoverlay_start_displaying_qr(const char *url);
void QRoverlay_stop_displaying_qr();

// condenstation.c
void dispatch_pending_logon(bool fail);

void blankcb_formsgbox(int type, void *data) {}

void qr_code_auth_thread() {
    int r = 0;
    
    cellMsgDialogAbort();
    cellMsgDialogOpen2(CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL |
        CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE |
        CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON,
        "Connecting to Steam, please wait...", blankcb_formsgbox, NULL, NULL);

    r = condenstation_init_cellHttp();
    cdst_log("condenstation_init_cellHttp = %i\n", r);

    uint8_t msg_buffer[0x400];
    size_t msg_size = 0;
    uint8_t out_buffer[0x800];
    size_t out_size = sizeof(out_buffer);

    auth_session_success = false;

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

    char steam_device_friendly_name[162];
    char ps3_name[128] = "PS3";
    cellSysutilGetSystemParamString(CELL_SYSUTIL_SYSTEMPARAM_ID_NICKNAME, ps3_name, sizeof(ps3_name));
    snprintf(steam_device_friendly_name, sizeof(steam_device_friendly_name), "condenstation (PS3): %s", ps3_name);

    CAuthenticationDeviceDetails details;
    cauthentication__device_details__init(&details);
    details.device_friendly_name = steam_device_friendly_name;
    details.has_os_type = true;
    details.os_type = 20; // Windows 11 - for PS3, it's -300.
    details.has_platform_type = true;
    details.platform_type = EAUTH_TOKEN_PLATFORM_TYPE__k_EAuthTokenPlatformType_SteamClient;
    
    CAuthenticationBeginAuthSessionViaQRRequest req;
    cauthentication__begin_auth_session_via_qr__request__init(&req);
    req.device_friendly_name = steam_device_friendly_name;
    req.has_platform_type = true;
    req.platform_type = EAUTH_TOKEN_PLATFORM_TYPE__k_EAuthTokenPlatformType_SteamClient;
    req.website_id = "Client";
    req.device_details = &details;

    msg_size = cauthentication__begin_auth_session_via_qr__request__get_packed_size(&req);
    cdst_log("msg_size = %i\n", msg_size);

    cauthentication__begin_auth_session_via_qr__request__pack(&req, msg_buffer);

    out_size = sizeof(out_buffer);
    SteamAuthenticationRPC(STEAMAUTH_POST, "BeginAuthSessionViaQR", 1, msg_buffer, msg_size, out_buffer, &out_size);
    // TODO(Emma): handle request failure
    
    cdst_log("out_size = %i\n", out_size);
    hexdump(out_buffer, out_size);

    CAuthenticationBeginAuthSessionViaQRResponse *resp = 
        cauthentication__begin_auth_session_via_qr__response__unpack(&shitalloc, out_size, out_buffer);

    if (resp->has_client_id)
        cdst_log("client id = %llu\n", resp->client_id);
    if (resp->challenge_url != NULL) {
        cdst_log("challenge url = %s\n", resp->challenge_url);
        QRoverlay_start_displaying_qr(resp->challenge_url);
    }
    if (resp->has_request_id) {
        cdst_log("request id = ");
        hexdump(resp->request_id.data, resp->request_id.len);
    }
    if (resp->has_interval)
        cdst_log("interval = %.2f\n", resp->interval);
    if (resp->has_version)
        cdst_log("version = %i\n", resp->version);
    if (resp->n_allowed_confirmations > 0) {
        for (int i = 0; i < resp->n_allowed_confirmations; i++) {
            cdst_log("allowed confirmation [%i] type = %i\n", i, resp->allowed_confirmations[i]->confirmation_type);
        }
    }

    cellMsgDialogClose(0.5);

    uint64_t current_client_id = resp->client_id;
    uint8_t current_request_id[0x20];
    size_t request_id_size = resp->request_id.len;
    memcpy(current_request_id, resp->request_id.data, request_id_size);

    // clear our shitalloc
    resp = NULL;
    memset(shitalloc_buffer, 0, sizeof(shitalloc_buffer));
    allocdata.allocated = 0;

    auth_session_running = true;

    int timeout = 0;

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
        cdst_log("msg_size = %i\n", msg_size);

        msg_size = cauthentication__poll_auth_session_status__request__pack(&pollreq, msg_buffer);
        // if the buffer has a trailing zero then get rid of it
        if (msg_buffer[msg_size - 1] == 0)
            msg_size -= 1;

        out_size = sizeof(out_buffer);
        SteamAuthenticationRPC(STEAMAUTH_POST, "PollAuthSessionStatus", 1, msg_buffer, msg_size, out_buffer, &out_size);
        // TODO(Emma): handle request failure

        cdst_log("out_size = %i\n", out_size);
        //hexdump(out_buffer, out_size);

        CAuthenticationPollAuthSessionStatusResponse *pollresp = 
            cauthentication__poll_auth_session_status__response__unpack(&shitalloc, out_size, out_buffer);
        
        if (pollresp->has_new_client_id) {
            cdst_log("new client id: %llu\n", pollresp->new_client_id);
            current_client_id = pollresp->new_client_id;
        }
        if (pollresp->new_challenge_url != NULL) {
            cdst_log("new challenge url: %s\n", pollresp->new_challenge_url);
            QRoverlay_start_displaying_qr(pollresp->new_challenge_url);
        }
        if (pollresp->account_name != NULL && pollresp->access_token != NULL) {
            cdst_log("we are LOGGED IN CHAT!!\n");
            HasConfigLoaded = true;
            strncpy(SteamAccountName, pollresp->account_name, sizeof(SteamAccountName));
            strncpy(SteamAccessToken, pollresp->refresh_token, sizeof(SteamAccessToken));
            memset(SteamGuardData, 0, sizeof(SteamGuardData));
            auth_session_running = false;
            auth_session_success = true;
            save_auth_config();
        }
        timeout++;
        // timeout after ~90 seconds
        if (timeout > 18)
            auth_session_running = false;
    }
    if (!auth_session_success) {
        cdst_log("auth session failed\n");
    }
    QRoverlay_stop_displaying_qr();
}

void cancel_qr_auth_session() {
    auth_session_running = false;
    dispatch_pending_logon(!auth_session_success);
}
