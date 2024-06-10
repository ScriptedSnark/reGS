#ifndef COMMON_BASESYSTEMMODULE_H
#define COMMON_BASESYSTEMMODULE_H

#include "ISystemModule.h"
#include "ObjectList.h"

class IBaseSystem;
class ISystemModule;

/**
*	Base class for system modules.
*/
class BaseSystemModule : public ISystemModule
{
public:
	BaseSystemModule() = default;
	virtual ~BaseSystemModule() = default;

	bool Init( IBaseSystem* system, int serial, char* name ) override;

	void RunFrame( double time ) override;

	void ReceiveSignal( ISystemModule* module, unsigned int signal, void* data ) override;

	void ExecuteCommand( int commandID, char* commandLine ) override;

	void RegisterListener( ISystemModule* module ) override;
	void RemoveListener( ISystemModule* module ) override;

	IBaseSystem* GetSystem() override;

	unsigned int GetSerial() override;

	char* GetStatusLine() override;

	char* GetType() override;

	char* GetName() override;

	unsigned int GetState() override;

	int GetVersion() override;

	void ShutDown() override;

	virtual char* COM_GetBaseDir();

	void FireSignal( unsigned int signal, void* data );

protected:
	IBaseSystem* m_System;

	ObjectList m_Listener;

	char m_Name[ 255 ];

	unsigned int m_State = SYSMODSTATE_CONSTRUCTED;
	unsigned int m_Serial;

	double m_SystemTime;

private:
	BaseSystemModule( const BaseSystemModule& ) = delete;
	BaseSystemModule& operator=( const BaseSystemModule& ) = delete;
};

int COM_BuildNumber();

#endif //COMMON_BASESYSTEMMODULE_H
