#ifndef _CONDENSTATION_CONFIG_H
#define _CONDENSTATION_CONFIG_H

#include <stdbool.h>

extern bool HasConfigLoaded;
extern char SteamAccountName[128];
extern char SteamAccessToken[1024];
extern char SteamGuardData[1024];

bool load_auth_config();
void save_auth_config();

#endif // _CONDENSTATION_CONFIG_H
