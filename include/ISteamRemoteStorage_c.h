#ifndef ISTEAMREMOTESTORAGE_C_H_
#define ISTEAMREMOTESTORAGE_C_H_
#include <stdint.h>
#include <stdbool.h>
#include "steam_api.h"

// based off STEAMREMOTESTORAGE_INTERFACE_VERSION004

typedef bool (*ISteamRemoteStorage_FileWrite_t)(void *thisobj, const char *pchFile, const void *pvData, int32 cubData );
typedef int32 (*ISteamRemoteStorage_FileRead_t)(void *thisobj, const char *pchFile, void *pvData, int32 cubDataToRead );
typedef bool (*ISteamRemoteStorage_FileForget_t)(void *thisobj, const char *pchFile );
typedef bool (*ISteamRemoteStorage_FileDelete_t)(void *thisobj, const char *pchFile );
typedef SteamAPICall_t (*ISteamRemoteStorage_FileShare_t)(void *thisobj, const char *pchFile );
typedef bool (*ISteamRemoteStorage_SetSyncPlatforms_t)(void *thisobj, const char *pchFile, int eRemoteStoragePlatform );
typedef bool (*ISteamRemoteStorage_FileExists_t)(void *thisobj, const char *pchFile );
typedef bool (*ISteamRemoteStorage_FilePersisted_t)(void *thisobj, const char *pchFile );
typedef int32 (*ISteamRemoteStorage_GetFileSize_t)(void *thisobj, const char *pchFile );
typedef int64 (*ISteamRemoteStorage_GetFileTimestamp_t)(void *thisobj, const char *pchFile );
typedef int (*ISteamRemoteStorage_GetSyncPlatforms_t)(void *thisobj, const char *pchFile );
typedef int32 (*ISteamRemoteStorage_GetFileCount_t)(void *thisobj);
typedef const char *(*ISteamRemoteStorage_GetFileNameAndSize_t)(void *thisobj, int iFile, int32 *pnFileSizeInBytes );
typedef bool (*ISteamRemoteStorage_GetQuota_t)(void *thisobj, int32 *pnTotalBytes, int32 *puAvailableBytes );
typedef bool (*ISteamRemoteStorage_IsCloudEnabledForAccount_t)(void *thisobj);
typedef bool (*ISteamRemoteStorage_IsCloudEnabledForApp_t)(void *thisobj);
typedef void (*ISteamRemoteStorage_SetCloudEnabledForApp_t)(void *thisobj, bool bEnabled );
typedef SteamAPICall_t (*ISteamRemoteStorage_UGCDownload_t)(void *thisobj, UGCHandle_t hContent );
typedef bool (*ISteamRemoteStorage_GetUGCDetails_t)(void *thisobj, UGCHandle_t hContent, AppId_t *pnAppID, char **ppchName, int32 *pnFileSizeInBytes, CSteamID *pSteamIDOwner );
typedef int32 (*ISteamRemoteStorage_UGCRead_t)(void *thisobj, UGCHandle_t hContent, void *pvData, int32 cubDataToRead );
typedef int32 (*ISteamRemoteStorage_GetCachedUGCCount_t)(void *thisobj);
typedef bool (*ISteamRemoteStorage_SynchronizeToClient_t)(void *thisobj);
typedef bool (*ISteamRemoteStorage_SynchronizeToServer_t)(void *thisobj);
typedef bool (*ISteamRemoteStorage_ResolveSyncConflict_t)(void *thisobj, int eResolveConflict );

typedef struct _ISteamRemoteStorage_vt {
    ISteamRemoteStorage_FileWrite_t FileWrite;
    ISteamRemoteStorage_FileRead_t FileRead;
    ISteamRemoteStorage_FileForget_t FileForget;
    ISteamRemoteStorage_FileDelete_t FileDelete;
    ISteamRemoteStorage_FileShare_t FileShare;
    ISteamRemoteStorage_SetSyncPlatforms_t SetSyncPlatforms;
    ISteamRemoteStorage_FileExists_t FileExists;
    ISteamRemoteStorage_FilePersisted_t FilePersisted;
    ISteamRemoteStorage_GetFileSize_t GetFileSize;
    ISteamRemoteStorage_GetFileTimestamp_t GetFileTimestamp;
    ISteamRemoteStorage_GetSyncPlatforms_t GetSyncPlatforms;
    ISteamRemoteStorage_GetFileCount_t GetFileCount;
    ISteamRemoteStorage_GetFileNameAndSize_t GetFileNameAndSize;
    ISteamRemoteStorage_GetQuota_t GetQuota;
    ISteamRemoteStorage_IsCloudEnabledForAccount_t IsCloudEnabledForAccount;
    ISteamRemoteStorage_IsCloudEnabledForApp_t IsCloudEnabledForApp;
    ISteamRemoteStorage_SetCloudEnabledForApp_t SetCloudEnabledForApp;
    ISteamRemoteStorage_UGCDownload_t UGCDownload;
    ISteamRemoteStorage_GetUGCDetails_t GetUGCDetails;
    ISteamRemoteStorage_UGCRead_t UGCRead;
    ISteamRemoteStorage_GetCachedUGCCount_t GetCachedUGCCount;
    ISteamRemoteStorage_SynchronizeToClient_t SynchronizeToClient;
    ISteamRemoteStorage_SynchronizeToServer_t SynchronizeToServer;
    ISteamRemoteStorage_ResolveSyncConflict_t ResolveSyncConflict;
} ISteamRemoteStorage_vt;

typedef struct _ISteamRemoteStorage_t {
    ISteamRemoteStorage_vt *vt;
} ISteamRemoteStorage_t;

#endif // ISTEAMREMOTESTORAGE_C_H_
