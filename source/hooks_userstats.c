#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <cell/pad.h>

#include "condenstation_logger.h"
#include "ps3_utilities.h"
#include "ISteamUserStats_c.h"

ISteamUserStats_t *steamUserStats = NULL;

static const char *FixStatName(const char *statname)
{
    if (memcmp(statname, "PS3.", 4) == 0)
    {
        const char *newstat = statname + 4; // skip the PS3 part
        // we only want to do this for stuff we can make a good assumption is safe to share with PC
        // you probably don't want CFG to be overridden for instance, or whatever is in "bitfield"
        if (memcmp(newstat, "SP.", 3) == 0 || // singleplayer progress
            memcmp(newstat, "GI.", 3) == 0 || // "general instructions"? whether to display tutorials
            memcmp(newstat, "ACH.", 4) == 0 || // achievement progress and steamids
            memcmp(newstat, "GAME.", 5) == 0) // those stats you see in co op on how much you walk
        {
            cdst_log("fixing stat '%s' -> '%s'\n", statname, newstat);
            return newstat;
        }
    }
    return statname;
}

ISteamUserStats_GetStatInt_t ISteamUserStats_GetStatInt;
bool ISteamUserStats_GetStatInt_Hook(void *thisobj, const char *pchName, int32 *pData)
{
    const char *fixedname = FixStatName(pchName);
    bool r = ISteamUserStats_GetStatInt(thisobj, fixedname, pData);
    printf("ISteamUserStats::GetStat(%p, \"%s\", %p -> %i ) = %s\n", thisobj, fixedname, pData, *pData, r ? "true" : "false");
    return r;
}

ISteamUserStats_GetStatFloat_t ISteamUserStats_GetStatFloat;
bool ISteamUserStats_GetStatFloat_Hook(void *thisobj, const char *pchName, float *pData)
{
    const char *fixedname = FixStatName(pchName);
    bool r = ISteamUserStats_GetStatFloat(thisobj, fixedname, pData);
    printf("ISteamUserStats::GetStat(%p, \"%s\", %p -> %f ) = %s\n", thisobj, fixedname, pData, *pData, r ? "true" : "false");
    return r;
}

ISteamUserStats_SetStatInt_t ISteamUserStats_SetStatInt;
bool ISteamUserStats_SetStatInt_Hook(void *thisobj, const char *pchName, int32 pData)
{
    const char *fixedname = FixStatName(pchName);
    bool r = ISteamUserStats_SetStatInt(thisobj, fixedname, pData);
    printf("ISteamUserStats::SetStat(%p, \"%s\", %i ) = %s\n", thisobj, fixedname, pData, r ? "true" : "false");
    return r;
}

ISteamUserStats_SetStatFloat_t ISteamUserStats_SetStatFloat;
bool ISteamUserStats_SetStatFloat_Hook(void *thisobj, const char *pchName, float pData)
{
    const char *fixedname = FixStatName(pchName);
    bool r = ISteamUserStats_SetStatFloat(thisobj, fixedname, pData);
    printf("ISteamUserStats::SetStat(%p, \"%s\", %f ) = %s\n", thisobj, fixedname, pData, r ? "true" : "false");
    return r;
}

void ApplyUserStatsHooks(ISteamUserStats_t *sus)
{
    if (sus != NULL) {
        steamUserStats = sus;
        ISteamUserStats_GetStatInt = sus->vt->GetStatInt;
        ISteamUserStats_GetStatFloat = sus->vt->GetStatFloat;
        ISteamUserStats_SetStatInt = sus->vt->SetStatInt;
        ISteamUserStats_SetStatFloat = sus->vt->SetStatFloat;
        PS3_Write32(&(sus->vt->GetStatInt), (uint32_t)ISteamUserStats_GetStatInt_Hook);
        PS3_Write32(&(sus->vt->GetStatFloat), (uint32_t)ISteamUserStats_GetStatFloat_Hook);
        PS3_Write32(&(sus->vt->SetStatInt), (uint32_t)ISteamUserStats_SetStatInt_Hook);
        PS3_Write32(&(sus->vt->SetStatFloat), (uint32_t)ISteamUserStats_SetStatFloat_Hook);
    }
}
