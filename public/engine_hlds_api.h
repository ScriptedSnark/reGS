#ifndef PUBLIC_ENGINE_HLDS_API_H
#define PUBLIC_ENGINE_HLDS_API_H

#include "interface.h"

/**
*	Interface exported by the engine for use by the dedicated server launcher
*/
class IDedicatedServerAPI : public IBaseInterface
{
public:

	virtual bool Init( char* basedir, char* cmdline, CreateInterfaceFn launcherFactory, CreateInterfaceFn filesystemFactory ) = 0;

	virtual int Shutdown() = 0;

	virtual bool RunFrame() = 0;

	virtual void AddConsoleText( char* text ) = 0;

	virtual void UpdateStatus( float* fps, int* nActive, int* nMaxPlayers, char* pszMap ) = 0;
};

#define ENGINE_HLDS_INTERFACE_VERSION "VENGINE_HLDS_API_VERSION002"

#endif //PUBLIC_ENGINE_HLDS_API_H
