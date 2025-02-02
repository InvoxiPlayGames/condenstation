#ifndef STEAM_API_H_
#define STEAM_API_H_
#include <stdint.h>
#include <stdbool.h>
#include "steamclient_steamps3params.h"

typedef int32_t HSteamPipe;
typedef int32_t HSteamUser;
typedef uint64_t SteamAPICall_t;

typedef bool (*SteamAPI_Init_t)(SteamPS3Params_t *pParams);
typedef HSteamUser (*SteamAPI_GetHSteamUser_t)();
typedef HSteamPipe (*SteamAPI_GetHSteamPipe_t)();
// also SteamGameServer_RunCallbacks
typedef void (*SteamAPI_RunCallbacks_t)();
// also SteamGameServer_Shutdown
typedef void (*SteamAPI_Shutdown_t)();
typedef void (*SteamAPI_RegisterCallback_t)(void *pCallback, int iCallback);
typedef void (*SteamAPI_UnregisterCallback_t)(void *pCallback);
typedef void (*SteamAPI_RegisterCallResult_t)(void *pCallback, SteamAPICall_t hAPICall);
typedef void (*SteamAPI_UnregisterCallResult_t)(void *pCallback, SteamAPICall_t hAPICall);

typedef bool (*SteamGameServer_Init)(SteamPS3Params_t *ps3Params, uint32_t unIP, uint16_t usSteamPort, uint16_t usGamePort, uint16_t usQueryPort, int eServerMode, const char *pchVersionString);

// for SteamApps, SteamClient, SteamHTTP, etc
typedef void *(*SteamAPI_ClassAccessor_t)();

#endif // STEAM_API_H_
