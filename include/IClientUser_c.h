#ifndef ICLIENTUSER_C_H_
#define ICLIENTUSER_C_H_
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t (*IClientUser_GetHSteamUser_t)(void *thisobj);
typedef uint32_t (*IClientUser_LogOn_t)(void *thisobj, bool bInteractive);
typedef uint32_t (*IClientUser_LogOnWithPassword_t)(void *thisobj, bool bInteractive, const char * pchLogin, const char * pchPassword);

typedef struct _IClientUser_vt {
    IClientUser_GetHSteamUser_t GetHSteamUser;
    IClientUser_LogOn_t LogOn;
    IClientUser_LogOnWithPassword_t LogOnWithPassword;
} IClientUser_vt;
typedef struct _IClientUser_t {
    IClientUser_vt *vt;
} IClientUser_t;

#endif // ICLIENTUSER_C_H
