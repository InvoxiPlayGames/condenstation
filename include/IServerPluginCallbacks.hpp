#ifndef ISERVERPLUGINCALLBACKS_HPP_
#define ISERVERPLUGINCALLBACKS_HPP_

#include "valve_ps3.h"

class IServerPluginCallbacks
{
public:
	virtual bool Load(CreateInterface_t interfaceFactory, CreateInterface_t gameServerFactory) = 0;
	virtual void Unload(void) = 0;
	virtual void Pause(void) = 0;
	virtual void UnPause(void) = 0;
	virtual const char *GetPluginDescription(void) = 0;
	virtual void LevelInit(char const *pMapName) = 0;
	virtual void ServerActivate(void *pEdictList, int edictCount, int clientMax) = 0;
	virtual void GameFrame(bool simulating) = 0;
	virtual void LevelShutdown(void) = 0;
	virtual void ClientActive(void *pEntity) = 0;
	virtual void ClientDisconnect(void *pEntity) = 0;
	virtual void ClientPutInServer(void *pEntity, char const *playername) = 0;
	virtual void SetCommandClient(int index) = 0;
	virtual void ClientSettingsChanged(void *pEdict) = 0;
	virtual int	ClientConnect(bool *bAllowConnect, void *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen) = 0;
	virtual int	ClientCommand(void *pEntity, const void* &args) = 0;
	virtual int	NetworkIDValidated(const char *pszUserName, const char *pszNetworkID) = 0;
};

#endif // ISERVERPLUGINCALLBACKS_HPP_
