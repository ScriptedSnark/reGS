#ifndef COMMON_IENGINEWRAPPER_H
#define COMMON_IENGINEWRAPPER_H

#include "cdll_int.h"
#include "event_args.h"

class IBaseSystem;
class ISystemModule;

class IEngineWrapper
{
public:
	virtual ~IEngineWrapper() {}

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

	virtual bool GetViewOrigin( float* origin ) = 0;
	virtual bool GetViewAngles( float* angles ) = 0;

	virtual int GetTraceEntity() = 0;

	virtual float GetCvarFloat( char* szName ) = 0;
	virtual char* GetCvarString( char* szName ) = 0;
	virtual void SetCvar( char* szName, char* szValue ) = 0;
	virtual void Cbuf_AddText( char* text ) = 0;

	virtual void DemoUpdateClientData( client_data_t* cdat ) = 0;
	virtual void CL_QueueEvent( int flags, int index, float delay, event_args_t* pargs ) = 0;
	virtual void HudWeaponAnim( int iAnim, int body ) = 0;
	virtual void CL_DemoPlaySound( int channel, char* sample, float attenuation, float volume, int flags, int pitch ) = 0;
	virtual void ClientDLL_ReadDemoBuffer( int size, byte* buffer ) = 0;
};

#endif //COMMON_IENGINEWRAPPER_H
