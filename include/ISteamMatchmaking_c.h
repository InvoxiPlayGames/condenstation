#ifndef ISTEAMMATCHMAKING_C_H_
#define ISTEAMMATCHMAKING_C_H_
#include <stdint.h>
#include <stdbool.h>

// based off SteamMatchMaking009
typedef int (*ISteamMatchmaking_RequestLobbyList_t)(void *thisobj);

typedef struct _ISteamMatchmaking_vt {
    void *GetFavoriteGameCount;
    void *GetFavoriteGame;
    void *AddFavoriteGame;
    void *RemoveFavoriteGame;
    ISteamMatchmaking_RequestLobbyList_t RequestLobbyList;
} ISteamMatchmaking_vt;

typedef struct _ISteamMatchmaking_t {
    ISteamMatchmaking_vt *vt;
} ISteamMatchmaking_t;

#endif // ISTEAMMATCHMAKING_C_H_
