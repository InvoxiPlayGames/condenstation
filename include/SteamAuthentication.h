#include <stdint.h>

typedef enum _SAMethodType {
    STEAMAUTH_GET,
    STEAMAUTH_POST
} SAMethodType;

int SteamAuthenticationInit();
int SteamAuthenticationRPC(int method_type, const char *rpc_name, int rpc_version, uint8_t *input, size_t size_input, uint8_t *output, size_t *size_output);
