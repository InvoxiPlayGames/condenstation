#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <cell/pad.h>

#include "condenstation_logger.h"
#include "ps3_utilities.h"
#include "ISteamRemoteStorage_c.h"

ISteamRemoteStorage_t *steamRemoteStorage = NULL;

bool ISteamRemoteStorage_IsCloudEnabledForApp_Stub(void *thisobj)
{
    cdst_log("ISteamRemoteStorage::IsCloudEnabledForApp stubbed\n");
    return false;
}

bool ISteamRemoteStorage_SynchronizeToClient_Stub(void *thisobj)
{
    cdst_log("ISteamRemoteStorage::SynchronizeToClient stubbed\n");
    return false;
}

bool ISteamRemoteStorage_SynchronizeToServer_Stub(void *thisobj)
{
    // TODO: this actually wants a callback returned in response to it even if the call itself fails
    //       and i currently don't feel like writing callback injection code :/
    cdst_log("ISteamRemoteStorage::SynchronizeToServer stubbed\n");
    return false;
}

void ISteamRemoteStorage_SetCloudEnabledForApp_Stub(void *thisobj, bool bEnabled )
{
    cdst_log("ISteamRemoteStorage::SetCloudEnabledForApp stubbed\n");
    return;
}

void ApplyRemoteStorageHooks(ISteamRemoteStorage_t *remote)
{
    if (remote != NULL) {
        steamRemoteStorage = remote;
        PS3_Write32(&(remote->vt->IsCloudEnabledForApp), (uint32_t)ISteamRemoteStorage_IsCloudEnabledForApp_Stub);
        PS3_Write32(&(remote->vt->SynchronizeToClient), (uint32_t)ISteamRemoteStorage_SynchronizeToClient_Stub);
        PS3_Write32(&(remote->vt->SynchronizeToServer), (uint32_t)ISteamRemoteStorage_SynchronizeToServer_Stub);
        PS3_Write32(&(remote->vt->SetCloudEnabledForApp), (uint32_t)ISteamRemoteStorage_SetCloudEnabledForApp_Stub);
    }
}
