#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cell/http.h>
#include <cell/http/util.h>
#include <cell/sysmodule.h>
#include <netex/net.h>

extern int _sys_printf(char *fmt, ...);
extern int _sys_sprintf(char *buf, char *fmt, ...);

static uint8_t cell_http_memory_pool[256 * 0x400]; // 256KB
static uint8_t cell_ssl_memory_pool[256 * 0x400]; // 256KB

static int ssl_callback(uint32_t verifyErr, const CellSslCert *sslCerts, int certNum, const char *hostname, CellHttpSslId id, void *userArg)
{
    _sys_printf("ssl_callback verifyErr = %08x\n", verifyErr);
    return verifyErr;
}

// DigiCert High Assurance EV Root CA
// used by api.steampowered.com as of 03-Feb-2025
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

CellHttpClientId current_cellHttp_client;
bool is_cellHttp_init = false;

int condenstation_init_cellHttp()
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
    r = cellHttpCreateClient(&current_cellHttp_client);
    if (r < 0) {
        _sys_printf("cellHttpCreateClient failed 0x%08x\n", (uint32_t)r);
        return r;
    }
    cellHttpClientSetVersion(current_cellHttp_client, 1, 1);
    // TODO(Emma): fill in these values properly
    cellHttpClientSetUserAgent(current_cellHttp_client, "condenstation/1.0 PS3/4.91 Portal2/1.04-EUR-DG");
    cellHttpClientSetSslCallback(current_cellHttp_client, ssl_callback, NULL);

    is_cellHttp_init = true;

    return 0;
}
