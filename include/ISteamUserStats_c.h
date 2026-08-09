#ifndef ISTEAMUSERSTATS_C_H_
#define ISTEAMUSERSTATS_C_H_
#include <stdint.h>
#include <stdbool.h>
#include "steam_api.h"

typedef uint32_t SteamLeaderboard_t;
typedef uint32_t SteamLeaderboardEntries_t;
typedef uint32_t LeaderboardEntry_t;

// based off STEAMUSERSTATS_INTERFACE_VERSION009

typedef bool (*ISteamUserStats_RequestCurrentStats_t)(void *thisobj);
typedef bool (*ISteamUserStats_GetStatInt_t)(void *thisobj, const char *pchName, int32 *pData );
typedef bool (*ISteamUserStats_GetStatFloat_t)(void *thisobj, const char *pchName, float *pData );
typedef bool (*ISteamUserStats_SetStatInt_t)(void *thisobj, const char *pchName, int32 nData );
typedef bool (*ISteamUserStats_SetStatFloat_t)(void *thisobj, const char *pchName, float fData );
typedef bool (*ISteamUserStats_UpdateAvgRateStat_t)(void *thisobj, const char *pchName, float flCountThisSession, double dSessionLength );
typedef bool (*ISteamUserStats_GetAchievement_t)(void *thisobj, const char *pchName, bool *pbAchieved );
typedef bool (*ISteamUserStats_SetAchievement_t)(void *thisobj, const char *pchName );
typedef bool (*ISteamUserStats_ClearAchievement_t)(void *thisobj, const char *pchName );
typedef bool (*ISteamUserStats_GetAchievementAndUnlockTime_t)(void *thisobj, const char *pchName, bool *pbAchieved, uint32 *punUnlockTime );
typedef bool (*ISteamUserStats_StoreStats_t)(void *thisobj);
typedef int (*ISteamUserStats_GetAchievementIcon_t)(void *thisobj, const char *pchName );
typedef const char *(*ISteamUserStats_GetAchievementDisplayAttribute_t)(void *thisobj, const char *pchName, const char *pchKey );
typedef bool (*ISteamUserStats_IndicateAchievementProgress_t)(void *thisobj, const char *pchName, uint32 nCurProgress, uint32 nMaxProgress );
typedef SteamAPICall_t (*ISteamUserStats_RequestUserStats_t)(void *thisobj, CSteamID steamIDUser );
typedef bool (*ISteamUserStats_GetUserStatInt_t)(void *thisobj, CSteamID steamIDUser, const char *pchName, int32 *pData );
typedef bool (*ISteamUserStats_GetUserStatFloat_t)(void *thisobj, CSteamID steamIDUser, const char *pchName, float *pData );
typedef bool (*ISteamUserStats_GetUserAchievement_t)(void *thisobj, CSteamID steamIDUser, const char *pchName, bool *pbAchieved );
typedef bool (*ISteamUserStats_GetUserAchievementAndUnlockTime_t)(void *thisobj, CSteamID steamIDUser, const char *pchName, bool *pbAchieved, uint32 *punUnlockTime );
typedef bool (*ISteamUserStats_ResetAllStats_t)(void *thisobj, bool bAchievementsToo );
typedef SteamAPICall_t (*ISteamUserStats_FindOrCreateLeaderboard_t)(void *thisobj, const char *pchLeaderboardName, int eLeaderboardSortMethod, int eLeaderboardDisplayType );
typedef SteamAPICall_t (*ISteamUserStats_FindLeaderboard_t)(void *thisobj, const char *pchLeaderboardName );
typedef const char *(*ISteamUserStats_GetLeaderboardName_t)(void *thisobj, SteamLeaderboard_t hSteamLeaderboard );
typedef int (*ISteamUserStats_GetLeaderboardEntryCount_t)(void *thisobj, SteamLeaderboard_t hSteamLeaderboard );
typedef int (*ISteamUserStats_GetLeaderboardSortMethod_t)(void *thisobj, SteamLeaderboard_t hSteamLeaderboard );
typedef int (*ISteamUserStats_GetLeaderboardDisplayType_t)(void *thisobj, SteamLeaderboard_t hSteamLeaderboard );
typedef SteamAPICall_t (*ISteamUserStats_DownloadLeaderboardEntries_t)(void *thisobj, SteamLeaderboard_t hSteamLeaderboard, int eLeaderboardDataRequest, int nRangeStart, int nRangeEnd );
typedef SteamAPICall_t (*ISteamUserStats_DownloadLeaderboardEntriesForUsers_t)(void *thisobj, SteamLeaderboard_t hSteamLeaderboard, CSteamID *prgUsers, int cUsers );
typedef bool (*ISteamUserStats_GetDownloadedLeaderboardEntry_t)(void *thisobj, SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t *pLeaderboardEntry, int32 *pDetails, int cDetailsMax );
typedef SteamAPICall_t (*ISteamUserStats_UploadLeaderboardScore_t)(void *thisobj, SteamLeaderboard_t hSteamLeaderboard, int eLeaderboardUploadScoreMethod, int32 nScore, const int32 *pScoreDetails, int cScoreDetailsCount );
typedef SteamAPICall_t (*ISteamUserStats_AttachLeaderboardUGC_t)(void *thisobj, SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC );
typedef SteamAPICall_t (*ISteamUserStats_GetNumberOfCurrentPlayers_t)(void *thisobj);
typedef bool (*ISteamUserStats_InstallPS3Trophies_t)(void *thisobj);
typedef uint64 (*ISteamUserStats_GetTrophySpaceRequiredBeforeInstall_t)(void *thisobj);
typedef bool (*ISteamUserStats_SetUserStatsData_t)(void *thisobj, const void *pvData, uint32 cubData );
typedef bool (*ISteamUserStats_GetUserStatsData_t)(void *thisobj, void *pvData, uint32 cubData, uint32 *pcubWritten );

typedef struct _ISteamUserStats_vt {
    ISteamUserStats_RequestCurrentStats_t RequestCurrentStats;
    ISteamUserStats_GetStatInt_t GetStatInt;
    ISteamUserStats_GetStatFloat_t GetStatFloat;
    ISteamUserStats_SetStatInt_t SetStatInt;
    ISteamUserStats_SetStatFloat_t SetStatFloat;
    ISteamUserStats_UpdateAvgRateStat_t UpdateAvgRateStat;
    ISteamUserStats_GetAchievement_t GetAchievement;
    ISteamUserStats_SetAchievement_t SetAchievement;
    ISteamUserStats_ClearAchievement_t ClearAchievement;
    ISteamUserStats_GetAchievementAndUnlockTime_t GetAchievementAndUnlockTime;
    ISteamUserStats_StoreStats_t StoreStats;
    ISteamUserStats_GetAchievementIcon_t GetAchievementIcon;
    ISteamUserStats_GetAchievementDisplayAttribute_t GetAchievementDisplayAttribute;
    ISteamUserStats_IndicateAchievementProgress_t IndicateAchievementProgress;
    ISteamUserStats_RequestUserStats_t RequestUserStats;
    ISteamUserStats_GetUserStatInt_t GetUserStatInt;
    ISteamUserStats_GetUserStatFloat_t GetUserStatFloat;
    ISteamUserStats_GetUserAchievement_t GetUserAchievement;
    ISteamUserStats_GetUserAchievementAndUnlockTime_t GetUserAchievementAndUnlockTime;
    ISteamUserStats_ResetAllStats_t ResetAllStats;
    ISteamUserStats_FindOrCreateLeaderboard_t FindOrCreateLeaderboard;
    ISteamUserStats_FindLeaderboard_t FindLeaderboard;
    ISteamUserStats_GetLeaderboardName_t GetLeaderboardName;
    ISteamUserStats_GetLeaderboardEntryCount_t GetLeaderboardEntryCount;
    ISteamUserStats_GetLeaderboardSortMethod_t GetLeaderboardSortMethod;
    ISteamUserStats_GetLeaderboardDisplayType_t GetLeaderboardDisplayType;
    ISteamUserStats_DownloadLeaderboardEntries_t DownloadLeaderboardEntries;
    ISteamUserStats_DownloadLeaderboardEntriesForUsers_t DownloadLeaderboardEntriesForUsers;
    ISteamUserStats_GetDownloadedLeaderboardEntry_t GetDownloadedLeaderboardEntry;
    ISteamUserStats_UploadLeaderboardScore_t UploadLeaderboardScore;
    ISteamUserStats_AttachLeaderboardUGC_t AttachLeaderboardUGC;
    ISteamUserStats_GetNumberOfCurrentPlayers_t GetNumberOfCurrentPlayers;
    ISteamUserStats_InstallPS3Trophies_t InstallPS3Trophies;
    ISteamUserStats_GetTrophySpaceRequiredBeforeInstall_t GetTrophySpaceRequiredBeforeInstall;
    ISteamUserStats_SetUserStatsData_t SetUserStatsData;
    ISteamUserStats_GetUserStatsData_t GetUserStatsData;
} ISteamUserStats_vt;

typedef struct _ISteamUserStats_t {
    ISteamUserStats_vt *vt;
} ISteamUserStats_t;

#endif // ISTEAMUSERSTATS_C_H_
