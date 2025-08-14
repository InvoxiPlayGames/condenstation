#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cell/http.h>
#include <cell/http/util.h>
#include <cell/sysmodule.h>
#include <netex/net.h>

#include "condenstation_logger.h"
#include "cellHttpHelper.h"
#include "SteamAuthentication.h"

int SteamAuthenticationRPC(int method_type, const char *rpc_name, int rpc_version, uint8_t *input, size_t size_input, uint8_t *output, size_t *size_output)
{
    int r;

    // build a URI to the IAuthenticationService method
    char uri_path_buf[128];
    snprintf(uri_path_buf, sizeof(uri_path_buf), "/IAuthenticationService/%s/v%i", rpc_name, rpc_version);
    // manually build the URI to save on allocations
    CellHttpUri uri = {
        .scheme = "https",
        .hostname = "api.steampowered.com",
        .port = 443,
        .path = uri_path_buf
    };
    cdst_log("built path: https://api.steampowered.com:443/%s\n", uri_path_buf);
    // build a request body by base64 encoding the input
    char req_body[2080];
    char req_b64[2048];
    char req_b64uri[2048];
    if (((size_input + 2) / 3 * 4) > sizeof(req_b64)) {
        cdst_log("input size too long\n");
        return -1;
    }
    cellHttpUtilBase64Encoder(req_b64, input, size_input);
    size_t required_uriencode = 0;
    cellHttpUtilFormUrlEncode(req_b64uri, sizeof(req_b64uri), req_b64, strlen(req_b64), &required_uriencode);
    snprintf(req_body, sizeof(req_body), "input_protobuf_encoded=%s", req_b64uri);
    cdst_log("built body: %s\n", req_body);
    // append request body to uri for GET requests
    char uri_get_path_buf[512];
    if (method_type == STEAMAUTH_GET) {
        snprintf(uri_get_path_buf, sizeof(uri_get_path_buf), "%s?%s", uri_path_buf, req_body);
        uri.path = uri_get_path_buf;
        cdst_log("full GET path: https://api.steampowered.com:443/%s\n", uri_get_path_buf);
    }
    // create a transaction
    CellHttpTransId trans;
    r = cellHttpCreateTransaction(&trans, current_cellHttp_client,
        method_type == STEAMAUTH_POST ? CELL_HTTP_METHOD_POST : CELL_HTTP_METHOD_GET,
        &uri);
    if (r < 0)
    {
        cdst_log("cellHttpCreateTransaction failed %08x\n", (uint32_t)r);
        return r;
    }
    cdst_log("transaction made\n");
    // set the headers
    if (method_type == STEAMAUTH_POST) {
        cellHttpRequestSetContentLength(trans, strlen(req_body));
        CellHttpHeader contenttype = {
            .name = "Content-Type",
            .value = "application/x-www-form-urlencoded"
        };
        cdst_log("adding header\n");
        cellHttpRequestAddHeader(trans, &contenttype);
    }
    // send the request
    cdst_log("sending request\n");
    size_t sent_bytes = 0;
    r = cellHttpSendRequest(trans, req_body, strlen(req_body), &sent_bytes);
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
