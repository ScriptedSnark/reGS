#ifndef COMMON_IBASESYSTEM_H
#define COMMON_IBASESYSTEM_H

#include "tier0/platform.h"

class IFileSystem;
class ISystemModule;
class IVGuiModule;
class ObjectList;

/**
*	Interface for systems
*/
class IBaseSystem
{
public:
	virtual ~IBaseSystem() {}

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

	virtual double GetTime() = 0;
	virtual unsigned int GetTick() = 0;

	virtual void SetFPS( float fps ) = 0;

	virtual void Printf( char* fmt, ... ) = 0;
	virtual void DPrintf( char* fmt, ... ) = 0;

	virtual void RedirectOutput( char* buffer, int maxSize ) = 0;

	virtual IFileSystem* GetFileSystem() = 0;

	virtual byte* LoadFile( const char* name, int* length ) = 0;
	virtual void FreeFile( byte* fileHandle ) = 0;

	virtual void SetTitle( char* text ) = 0;

	virtual void SetStatusLine( char* text ) = 0;

	virtual void ShowConsole( bool visible ) = 0;
	virtual void LogConsole( char* filename ) = 0;

	virtual bool InitVGUI( IVGuiModule* module ) = 0;

	virtual bool RegisterCommand( char* name, ISystemModule* module, int commandID ) = 0;
	virtual void GetCommandMatches( char* string, ObjectList* pMatchList ) = 0;

	virtual void ExecuteString( char* commands ) = 0;
	virtual void ExecuteFile( char* filename ) = 0;

	virtual void Errorf( char* fmt, ... ) = 0;

	virtual char* CheckParam( char* param ) = 0;

	virtual bool AddModule( ISystemModule* module, char* name ) = 0;
	virtual ISystemModule* GetModule( char* interfacename, char* library, char* instancename ) = 0;
	virtual bool RemoveModule( ISystemModule* module ) = 0;

	virtual void Stop() = 0;

	virtual char* COM_GetBaseDir() = 0;
};

#endif //COMMON_IBASESYSTEM_H
