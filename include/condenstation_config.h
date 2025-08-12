#ifndef _CONDENSTATION_CONFIG_H
#define _CONDENSTATION_CONFIG_H

#include <stdbool.h>

extern bool HasConfigLoaded;
extern char SteamAccountName[128];
extern char SteamAccessToken[1024];

bool load_config();
void save_config();

#endif // _CONDENSTATION_CONFIG_H
