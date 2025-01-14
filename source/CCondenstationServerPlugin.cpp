#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "CCondenstationServerPlugin.hpp"

CCondenstationServerPlugin::CCondenstationServerPlugin() { }
CCondenstationServerPlugin::~CCondenstationServerPlugin() { }
bool CCondenstationServerPlugin::Load(CreateInterface_t interfaceFactory, CreateInterface_t gameServerFactory) { return true; }
void CCondenstationServerPlugin::Unload() { }
void CCondenstationServerPlugin::Pause() { }
void CCondenstationServerPlugin::UnPause() { }
const char * CCondenstationServerPlugin::GetPluginDescription() { return "Condenstation"; }
void CCondenstationServerPlugin::LevelInit(const char *pMapName) { }
void CCondenstationServerPlugin::ServerActivate(void *pEdictList, int edictCount, int clientMax) { }
void CCondenstationServerPlugin::GameFrame(bool simulating) { }
void CCondenstationServerPlugin::LevelShutdown() { }
void CCondenstationServerPlugin::ClientActive(void *pEntity) { }
void CCondenstationServerPlugin::ClientDisconnect(void *pEntity) { }
void CCondenstationServerPlugin::ClientPutInServer(void *pEntity, char const *playername) { }
void CCondenstationServerPlugin::SetCommandClient(int index) { }
void CCondenstationServerPlugin::ClientSettingsChanged(void *pEdict) { }
int CCondenstationServerPlugin::ClientConnect(bool *bAllowConnect, void *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen) { return 0; }
int CCondenstationServerPlugin::ClientCommand(void *pEntity, const void* &args) { return 0; }
int CCondenstationServerPlugin::NetworkIDValidated(const char *pszUserName, const char *pszNetworkID) { return 0; }

static CCondenstationServerPlugin *g_condenstationServerPlugin = NULL;

extern "C" {
    void *get_CCondenstationServerPlugin() {
        if (g_condenstationServerPlugin == NULL)
            g_condenstationServerPlugin = new CCondenstationServerPlugin();
        return g_condenstationServerPlugin;
    }
}
