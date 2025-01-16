#ifndef STEAMCLIENT_INTERNAL_H_
#define STEAMCLIENT_INTERNAL_H_

#include <stdint.h>
#include <stdbool.h>
#include "steam_ps3_nids.h"

// different from steamps3params.h in the Steamworks SDK, has internal stuff and matches CSGO-1.01

#define STEAM_PS3_PATH_MAX 1055
#define STEAM_PS3_SERVICE_ID_MAX 32
#define STEAM_PS3_COMMUNICATION_ID_MAX 10
#define STEAM_PS3_COMMUNICATION_SIG_MAX 160
#define STEAM_PS3_LANGUAGE_MAX 64
#define STEAM_PS3_REGION_CODE_MAX 16

#define STEAM_PS3_PORTAL2_PARAMS_VER 1
#define STEAM_PS3_CSGO_PARAMS_VER 4

typedef struct _Ps3netInit_t {
    bool m_bNeedInit;
    void *m_pMemory;
    int m_nMemorySize;
    int m_flags;
} Ps3netInit_t;

// Ps3jpgInit_t / Ps3pngInit_t / Ps3sysutilUserInfo_t
typedef struct _Ps3sysmoduleInit_t {
    bool m_bNeedInit;
} Ps3sysmoduleInit_t;

typedef struct _SteamPS3ParamsInternal_t {
    int m_nVersion;
    int m_eUniverse;
    const char *m_pchCMForce;
    bool m_bAutoReloadVGUIResources;
} SteamPS3ParamsInternal_t;

typedef struct _SteamPS3Params_t {
    uint32_t m_unVersion;
    SteamPS3ParamsInternal_t *pInternal;
    uint32_t m_nAppId;
    char m_rgchInstallationPath[STEAM_PS3_PATH_MAX];
    char m_rgchSystemCache[STEAM_PS3_PATH_MAX];
    char m_rgchGameData[STEAM_PS3_PATH_MAX];
    char m_rgchNpServiceID[STEAM_PS3_SERVICE_ID_MAX];
    char m_rgchNpCommunicationID[STEAM_PS3_COMMUNICATION_ID_MAX];
    char m_rgchNpCommunicationSig[STEAM_PS3_COMMUNICATION_SIG_MAX];
    char m_rgchSteamLanguage[STEAM_PS3_LANGUAGE_MAX];
    char m_rgchRegionCode[STEAM_PS3_REGION_CODE_MAX];
    uint32_t m_cSteamInputTTY;
    Ps3netInit_t m_sysNetInitInfo;
    Ps3sysmoduleInit_t m_sysJpgInitInfo;
    Ps3sysmoduleInit_t m_sysPngInitInfo;
    Ps3sysmoduleInit_t m_sysSysUtilUserInfo;
    bool m_bIncludeNewsPage; // m_unVersion >= 2
    bool m_bPersonaStateOffline; // m_unVersion == 4
} SteamPS3Params_t;

#define NID_steamclient_GetSteamPS3Params NID_steamclient_0xE2C6386D
typedef SteamPS3Params_t *(*steamclient_GetSteamPS3Params_t)();

#define NID_steamclient_SetSteamPS3Params NID_steamclient_0xAC6BB824
typedef bool (*steamclient_SetSteamPS3Params_t)(SteamPS3Params_t *params);

#endif // STEAMCLIENT_INTERNAL_H_
