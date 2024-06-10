#ifndef COMMON_ISYSTEMMODULE_H
#define COMMON_ISYSTEMMODULE_H

#include "interface.h"

class IBaseSystem;

#define SYSMODSTATE_CONSTRUCTED 0U
#define SYSMODSTATE_INIT 1U
#define SYSMODSTATE_RUNNING 3U
#define SYSMODSTATE_SHUTDOWN 4U

/**
*	Interface for system modules.
*/
class ISystemModule : public IBaseInterface
{
public:
	virtual bool Init( IBaseSystem* system, int serial, char* name ) = 0;

	virtual void RunFrame( double time ) = 0;

	virtual void ReceiveSignal( ISystemModule* module, unsigned int signal, void* data ) = 0;

	virtual void ExecuteCommand( int commandID, char* commandLine ) = 0;

	virtual void RegisterListener( ISystemModule* module ) = 0;
	virtual void RemoveListener( ISystemModule* module ) = 0;

	virtual IBaseSystem* GetSystem() = 0;

	virtual unsigned int GetSerial() = 0;

	virtual char* GetStatusLine() = 0;

	virtual char* GetType() = 0;

	virtual char* GetName() = 0;

	virtual unsigned int GetState() = 0;

	virtual int GetVersion() = 0;

	virtual void ShutDown() = 0;
};

#endif //COMMON_ISYSTEMMODULE_H
