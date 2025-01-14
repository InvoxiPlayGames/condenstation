#ifndef CCONDENSTATIONSERVERPLUGIGN_HPP_
#define CCONDENSTATIONSERVERPLUGIGN_HPP_

#include "IServerPluginCallbacks.hpp"

class CCondenstationServerPlugin : public IServerPluginCallbacks {
public:
    CCondenstationServerPlugin();
    ~CCondenstationServerPlugin();
	virtual bool Load(CreateInterface_t interfaceFactory, CreateInterface_t gameServerFactory);
	virtual void Unload(void);
	virtual void Pause(void);
	virtual void UnPause(void);
	virtual const char *GetPluginDescription(void);
	virtual void LevelInit(char const *pMapName);
	virtual void ServerActivate(void *pEdictList, int edictCount, int clientMax);
	virtual void GameFrame(bool simulating);
	virtual void LevelShutdown(void);
	virtual void ClientActive(void *pEntity);
	virtual void ClientDisconnect(void *pEntity);
	virtual void ClientPutInServer(void *pEntity, char const *playername);
	virtual void SetCommandClient(int index);
	virtual void ClientSettingsChanged(void *pEdict);
	virtual int	ClientConnect(bool *bAllowConnect, void *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen);
	virtual int	ClientCommand(void *pEntity, const void* &args);
	virtual int	NetworkIDValidated(const char *pszUserName, const char *pszNetworkID);
};

#endif // CCONDENSTATIONSERVERPLUGIGN_HPP_
