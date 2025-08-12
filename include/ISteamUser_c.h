#ifndef ISTEAMUSER_C_H_
#define ISTEAMUSER_C_H_
#include <stdint.h>
#include <stdbool.h>
#include "IClientUser_c.h"

typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint32_t HSteamUser;
typedef uint32_t EVoiceResult;
typedef uint32_t HAuthTicket;
typedef uint32_t EBeginAuthSessionResult;
typedef uint32_t EUserHasLicenseForAppResult;
typedef uint32_t SteamAPICall_t;
typedef void *CSteamID;

// based off SteamUser015

typedef HSteamUser (*ISteamUser_GetHSteamUser_t)(void *thisobj);
typedef bool (*ISteamUser_BLoggedOn_t)(void *thisobj);
typedef CSteamID (*ISteamUser_GetSteamID_t)(void *thisobj);
typedef int (*ISteamUser_InitiateGameConnection_t)(void *thisobj, void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure );
typedef void (*ISteamUser_TerminateGameConnection_t)(void *thisobj, uint32 unIPServer, uint16 usPortServer );
typedef void (*ISteamUser_TrackAppUsageEvent_t)(void *thisobj, uint32_t gameID, int eAppUsageEvent, const char *pchExtraInfo );
typedef bool (*ISteamUser_GetUserDataFolder_t)(void *thisobj, char *pchBuffer, int cubBuffer );
typedef void (*ISteamUser_StartVoiceRecording_t)(void *thisobj);
typedef void (*ISteamUser_StopVoiceRecording_t)(void *thisobj);
typedef EVoiceResult (*ISteamUser_GetAvailableVoice_t)(void *thisobj,uint32 *pcbCompressed, uint32 *pcbUncompressed);
typedef EVoiceResult (*ISteamUser_GetVoice_t)(void *thisobj, bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed, void *pUncompressedDestBuffer, uint32 cbUncompressedDestBufferSize, uint32 *nUncompressBytesWritten);
typedef EVoiceResult (*ISteamUser_DecompressVoice_t)(void *thisobj, const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, uint32 nDesiredSampleRate );
typedef uint32 (*ISteamUser_GetVoiceOptimalSampleRate_t)(void *thisobj);
typedef HAuthTicket (*ISteamUser_GetAuthSessionTicket_t)(void *thisobj, void *pTicket, int cbMaxTicket, uint32 *pcbTicket );
typedef EBeginAuthSessionResult (*ISteamUser_BeginAuthSession_t)(void *thisobj, const void *pAuthTicket, int cbAuthTicket, CSteamID steamID );
typedef void (*ISteamUser_EndAuthSession_t)(void *thisobj, CSteamID steamID );
typedef void (*ISteamUser_CancelAuthTicket_t)(void *thisobj, HAuthTicket hAuthTicket );
typedef EUserHasLicenseForAppResult (*ISteamUser_UserHasLicenseForApp_t)(void *thisobj, CSteamID steamID, uint32_t appID );
typedef bool (*ISteamUser_BIsBehindNAT_t)(void *thisobj);
typedef void (*ISteamUser_AdvertiseGame_t)(void *thisobj, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer );
typedef SteamAPICall_t (*ISteamUser_RequestEncryptedAppTicket_t)(void *thisobj, void *pDataToInclude, int cbDataToInclude );
typedef bool (*ISteamUser_GetEncryptedAppTicket_t)(void *thisobj, void *pTicket, int cbMaxTicket, uint32 *pcbTicket );
typedef void (*ISteamUser_LogOn_t)(void *thisobj, bool bInteractive );
typedef void (*ISteamUser_LogOnAndLinkSteamAccountToPSN_t)(void *thisobj, bool bInteractive, const char *pchUserName, const char *pchPassword );
typedef void (*ISteamUser_LogOnAndCreateNewSteamAccountIfNeeded_t)(void *thisobj, bool bInteractive );
typedef CSteamID (*ISteamUser_GetConsoleSteamID_t)(void *thisobj);

typedef struct _ISteamUser_vt {
    ISteamUser_GetHSteamUser_t GetHSteamUser;
    ISteamUser_BLoggedOn_t BLoggedOn;
    ISteamUser_GetSteamID_t GetSteamID;
    ISteamUser_InitiateGameConnection_t InitiateGameConnection;
    ISteamUser_TerminateGameConnection_t TerminateGameConnection;
    ISteamUser_TrackAppUsageEvent_t TrackAppUsageEvent;
    ISteamUser_GetUserDataFolder_t GetUserDataFolder;
    ISteamUser_StartVoiceRecording_t StartVoiceRecording;
    ISteamUser_StopVoiceRecording_t StopVoiceRecording;
    ISteamUser_GetAvailableVoice_t GetAvailableVoice;
    ISteamUser_GetVoice_t GetVoice;
    ISteamUser_DecompressVoice_t DecompressVoice;
    ISteamUser_GetVoiceOptimalSampleRate_t GetVoiceOptimalSampleRate;
    ISteamUser_GetAuthSessionTicket_t GetAuthSessionTicket;
    ISteamUser_BeginAuthSession_t BeginAuthSession;
    ISteamUser_EndAuthSession_t EndAuthSession;
    ISteamUser_CancelAuthTicket_t CancelAuthTicket;
    ISteamUser_UserHasLicenseForApp_t UserHasLicenseForApp;
    ISteamUser_BIsBehindNAT_t BIsBehindNAT;
    ISteamUser_AdvertiseGame_t AdvertiseGame;
    ISteamUser_RequestEncryptedAppTicket_t RequestEncryptedAppTicket;
    ISteamUser_GetEncryptedAppTicket_t GetEncryptedAppTicket;
    ISteamUser_LogOn_t LogOn;
    ISteamUser_LogOnAndLinkSteamAccountToPSN_t LogOnAndLinkSteamAccountToPSN;
    ISteamUser_LogOnAndCreateNewSteamAccountIfNeeded_t LogOnAndCreateNewSteamAccountIfNeeded;
    ISteamUser_GetConsoleSteamID_t GetConsoleSteamID;
} ISteamUser_vt;

typedef struct _ISteamUser_t {
    ISteamUser_vt *vt;
    IClientUser_t *clientUser;
} ISteamUser_t;

#endif // ISTEAMUSER_C_H_
