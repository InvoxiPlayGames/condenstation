#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cell/http.h>
#include <cell/http/util.h>
#include <cell/sysmodule.h>
#include <netex/net.h>

#include "condenstation_logger.h"
#include "cellHttpHelper.h"

int SteamGetCMList(uint8_t *output, size_t *size_output)
{
    int r;

    // build the URI
    CellHttpUri uri = {
        .scheme = "https",
        .hostname = "api.steampowered.com",
        .port = 443,
        .path = "/ISteamDirectory/GetCMList/v1?cellid=0"
    };
    cdst_log("connecting to https://api.steampowered.com:443/ISteamDirectory/GetCMList/v1?cellid=0\n");
    // create a transaction
    CellHttpTransId trans;
    r = cellHttpCreateTransaction(&trans, current_cellHttp_client, CELL_HTTP_METHOD_GET, &uri);
    if (r < 0)
    {
        cdst_log("cellHttpCreateTransaction failed %08x\n", (uint32_t)r);
        return r;
    }
    cdst_log("transaction made\n");
    // send the request
    cdst_log("sending request\n");
    size_t sent_bytes = 0;
    r = cellHttpSendRequest(trans, NULL, 0, &sent_bytes);
    if (r < 0) {
        cdst_log("cellHttpSendRequest failed %08x\n", (uint32_t)r);
        return r;
    }
    cdst_log("sent request %i bytes\n", sent_bytes);
    // recieve the status code
    int rcode = 0;
    r = cellHttpResponseGetStatusCode(trans, &rcode);
    if (r < 0) {
        cdst_log("cellHttpResponseGetStatusCode failed %08x\n", (uint32_t)r);
        return r;
    }
    cdst_log("server returned HTTP %i\n", rcode);
    // recieve the response
    uint64_t recv_bytes;
    r = cellHttpResponseGetContentLength(trans, &recv_bytes);
    if (r < 0) {
        cdst_log("cellHttpResponseGetContentLength failed %08x\n", (uint32_t)r);
        return r;
    }
    cdst_log("content-length: %llu\n", recv_bytes);
    // TODO(Emma): at this point cellHttp has our headers, we can check X-eresult here or something..?
    size_t outbuf_size = *size_output;
    size_t recv_size = 0;
    size_t total_recvd = 0;
    while (total_recvd < recv_bytes) {
        r = cellHttpRecvResponse(trans, output + total_recvd, outbuf_size - total_recvd, &recv_size);
        if (r < 0) {
            cdst_log("cellHttpRecvResponse failed %08x\n", (uint32_t)r);
            break;
        }
        cdst_log("recieved %i bytes\n", recv_size);
        total_recvd += recv_size;
    }
    *size_output = total_recvd;
    // clean up
    cellHttpDestroyTransaction(trans);

    return 0;
}

void set_fatal_error(const char *text);
void SetCMNetadr(const char *in_ip_addr);

void get_cm_thread() {
    int r = condenstation_init_cellHttp();
    cdst_log("condenstation_init_cellHttp = %i\n", r);

    // make the web request
    char cm_req_body[8192] = {0};
    int req_body_size = sizeof(cm_req_body);
    r = SteamGetCMList(cm_req_body, &req_body_size);
    if (r < 0) {
        set_fatal_error("Couldn't get Steam connection manager:\nCouldn't connect to the Steam API.");
        return;
    }
    if (req_body_size <= 50) {
        cdst_log("invalid response length %i\n", req_body_size);
        set_fatal_error("Couldn't get Steam connection manager:\nThe response was too short.");
        return;
    }
    // JSON parsing is expensive and the response format is mostly sane...
    // I'm sorry.
    if (strncmp(cm_req_body, "{\"response\":{\"serverlist\":[\"", 28) != 0) {
        cdst_log("invalid response");
        set_fatal_error("Couldn't get Steam connection manager:\nThe response was invalid.");
        return;
    }
    char splitIP[24];
    int j = 0;
    // 28 = strlen("{\"response\":{\"serverlist\":[\"")
    for (int i = 28; i < strlen(cm_req_body); i++) {
        // break on ending quote
        if (cm_req_body[i] == '"')
            break;
        // copy the byte to the split IP buffer
        splitIP[j] = cm_req_body[i];
        j++;
        if (j >= 23)
            break;
    }
    cdst_log("Got CM IP address: %s\n", splitIP);
    SetCMNetadr(splitIP);
}
