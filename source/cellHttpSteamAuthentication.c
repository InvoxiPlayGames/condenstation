#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cell/http.h>
#include <cell/http/util.h>
#include <cell/sysmodule.h>
#include <netex/net.h>

#include "SteamAuthentication.h"


extern int _sys_printf(char *fmt, ...);
extern int _sys_sprintf(char *buf, char *fmt, ...);

static uint8_t cell_http_memory_pool[256 * 0x400]; // 256KB
static uint8_t cell_ssl_memory_pool[256 * 0x400]; // 256KB

static int ssl_callback(uint32_t verifyErr, const CellSslCert *sslCerts, int certNum, const char *hostname, CellHttpSslId id, void *userArg)
{
    _sys_printf("ssl_callback verifyErr = %08x\n", verifyErr);
    // for testing just assume all is fine
    // URGENT TODO(Emma): THIS IS UNSAFE
    return verifyErr;
}

// DigiCert High Assurance EV Root CA
// used by api.steampowered.com
static char valve_cacert[] __attribute__((aligned(64))) =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n"
    "ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n"
    "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n"
    "LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n"
    "RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n"
    "+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n"
    "PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n"
    "xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n"
    "Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n"
    "hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n"
    "EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n"
    "MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n"
    "FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n"
    "nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n"
    "eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n"
    "hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n"
    "Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n"
    "vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n"
    "+OkuE6N36B9K\n"
    "-----END CERTIFICATE-----\n";

static CellHttpClientId client;

int SteamAuthenticationInit()
{
    int r = 0;

    // initialise the network
    cellSysmoduleLoadModule(CELL_SYSMODULE_HTTPS);
    sys_net_initialize_network();

    // initialise the HTTP library if it hasn't been done already
    r = cellHttpInit(cell_http_memory_pool, sizeof(cell_http_memory_pool));
    if (r < 0 && r != CELL_HTTP_ERROR_ALREADY_INITIALIZED) {
        _sys_printf("cellHttpInit failed 0x%08x\n", (uint32_t)r);
        return r;
    }
    if (r == CELL_HTTP_ERROR_ALREADY_INITIALIZED)
        _sys_printf("cellHttp already initialised. this is good!\n");
    
    // initialise the SSL library if it hasn't been done already
    r = cellSslInit(cell_ssl_memory_pool, sizeof(cell_ssl_memory_pool));
    if (r < 0 && r != CELL_SSL_ERROR_ALREADY_INITIALIZED) {
        _sys_printf("cellSslInit failed 0x%08x\n", (uint32_t)r);
        return r;
    }
    if (r == CELL_SSL_ERROR_ALREADY_INITIALIZED)
        _sys_printf("cellSsl already initialised. this is good!\n");

    // initialise HTTPS with the Valve cert
    CellHttpsData cer = {
        .ptr = valve_cacert,
        .size = sizeof(valve_cacert)
    };
    r = cellHttpsInit(1, &cer);
    if (r < 0) {
        _sys_printf("cellHttpsInit failed 0x%08x\n", (uint32_t)r);
        return r;
    }

    // create our HTTP client
    r = cellHttpCreateClient(&client);
    if (r < 0) {
        _sys_printf("cellHttpCreateClient failed 0x%08x\n", (uint32_t)r);
        return r;
    }
    cellHttpClientSetVersion(client, 1, 1);
    // TODO(Emma): fill in these values properly
    cellHttpClientSetUserAgent(client, "condenstation/1.0 PS3/4.91 Portal2/1.04-EUR-DG");
    cellHttpClientSetSslCallback(client, ssl_callback, NULL);

    return 0;
}

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
    _sys_printf("built path: https://api.steampowered.com:443/%s\n", uri_path_buf);
    // build a request body by base64 encoding the input
    char req_body[2080];
    char req_b64[2048];
    char req_b64uri[2048];
    if (((size_input + 2) / 3 * 4) > sizeof(req_b64)) {
        _sys_printf("input size too long\n");
        return -1;
    }
    cellHttpUtilBase64Encoder(req_b64, input, size_input);
    size_t required_uriencode = 0;
    cellHttpUtilFormUrlEncode(req_b64uri, sizeof(req_b64uri), req_b64, strlen(req_b64), &required_uriencode);
    snprintf(req_body, sizeof(req_body), "input_protobuf_encoded=%s", req_b64uri);
    _sys_printf("built body: %s\n", req_body);
    // append request body to uri for GET requests
    char uri_get_path_buf[512];
    if (method_type == STEAMAUTH_GET) {
        snprintf(uri_get_path_buf, sizeof(uri_get_path_buf), "%s?%s", uri_path_buf, req_body);
        uri.path = uri_get_path_buf;
        _sys_printf("full GET path: https://api.steampowered.com:443/%s\n", uri_get_path_buf);
    }
    // create a transaction
    CellHttpTransId trans;
    r = cellHttpCreateTransaction(&trans, client,
        method_type == STEAMAUTH_POST ? CELL_HTTP_METHOD_POST : CELL_HTTP_METHOD_GET,
        &uri);
    if (r < 0)
    {
        _sys_printf("cellHttpCreateTransaction failed %08x\n", (uint32_t)r);
        return r;
    }
    _sys_printf("transaction made\n");
    // set the headers
    if (method_type == STEAMAUTH_POST) {
        cellHttpRequestSetContentLength(trans, strlen(req_body));
        CellHttpHeader contenttype = {
            .name = "Content-Type",
            .value = "application/x-www-form-urlencoded"
        };
        _sys_printf("adding header\n");
        cellHttpRequestAddHeader(trans, &contenttype);
    }
    // send the request
    _sys_printf("sending request\n");
    size_t sent_bytes = 0;
    r = cellHttpSendRequest(trans, req_body, strlen(req_body), &sent_bytes);
    if (r < 0) {
        _sys_printf("cellHttpSendRequest failed %08x\n", (uint32_t)r);
        return r;
    }
    _sys_printf("sent request %i bytes\n", sent_bytes);
    // recieve the status code
    int rcode = 0;
    r = cellHttpResponseGetStatusCode(trans, &rcode);
    if (r < 0) {
        _sys_printf("cellHttpResponseGetStatusCode failed %08x\n", (uint32_t)r);
        return r;
    }
    _sys_printf("server returned HTTP %i\n", rcode);
    // recieve the response
    uint64_t recv_bytes;
    r = cellHttpResponseGetContentLength(trans, &recv_bytes);
    if (r < 0) {
        _sys_printf("cellHttpResponseGetContentLength failed %08x\n", (uint32_t)r);
        return r;
    }
    _sys_printf("content-length: %llu\n", recv_bytes);
    // TODO(Emma): at this point cellHttp has our headers, we can check X-eresult here or something..?
    size_t outbuf_size = *size_output;
    size_t recv_size = 0;
    size_t total_recvd = 0;
    while (total_recvd < recv_bytes) {
        r = cellHttpRecvResponse(trans, output + total_recvd, outbuf_size - total_recvd, &recv_size);
        if (r < 0) {
            _sys_printf("cellHttpRecvResponse failed %08x\n", (uint32_t)r);
            break;
        }
        _sys_printf("recieved %i bytes\n", recv_size);
        total_recvd += recv_size;
    }
    *size_output = total_recvd;
    // clean up
    cellHttpDestroyTransaction(trans);

    return 0;
}
