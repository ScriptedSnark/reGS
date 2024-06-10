#ifndef ENGINE_SYSTEMWRAPPER_H
#define ENGINE_SYSTEMWRAPPER_H

#include "interface.h"

#include "BaseSystemModule.h"
#include "IBaseSystem.h"
#include "IEngineWrapper.h"
#include "ObjectList.h"

class EngineWrapper : public IEngineWrapper, public BaseSystemModule
{
public:
	EngineWrapper() = default;
	virtual ~EngineWrapper() = default;

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

	bool GetViewOrigin( float* origin ) override;
	bool GetViewAngles( float* angles ) override;

	int GetTraceEntity() override;

	float GetCvarFloat( char* szName ) override;
	char* GetCvarString( char* szName ) override;
	void SetCvar( char* szName, char* szValue ) override;
	void Cbuf_AddText( char* text ) override;
	
	void DemoUpdateClientData( client_data_t* cdat ) override;
	void CL_QueueEvent( int flags, int index, float delay, event_args_t* pargs ) override;
	void HudWeaponAnim( int iAnim, int body ) override;
	void CL_DemoPlaySound( int channel, char* sample, float attenuation, float volume, int flags, int pitch ) override;
	void ClientDLL_ReadDemoBuffer( int size, byte* buffer ) override;

private:
};

class SystemWrapper : public IBaseSystem, public BaseSystemModule
{
public:
	enum LocalCommandIDs
	{
		CMD_ID_MODULES = 1,
		CMD_ID_LOADMODULE,
		CMD_ID_UNLOADMODULE,
	};

private:
	struct command_t
	{
		char name[ 32 ];
		int commandID;
		ISystemModule* module;
	};

	struct library_t
	{
		char name[ 4096 ];
		CSysModule* handle;
		CreateInterfaceFn createInterfaceFn;
	};

public:
	SystemWrapper() = default;
	virtual ~SystemWrapper() = default;

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

	double GetTime() override;
	unsigned int GetTick() override;

	void SetFPS( float fps ) override;

	void Printf( char* fmt, ... ) override;
	void DPrintf( char* fmt, ... ) override;

	void RedirectOutput( char* buffer, int maxSize ) override;

	IFileSystem* GetFileSystem() override;

	byte* LoadFile( const char* name, int* length ) override;
	void FreeFile( byte* fileHandle ) override;

	void SetTitle( char* text ) override;

	void SetStatusLine( char* text ) override;

	void ShowConsole( bool visible ) override;
	void LogConsole( char* filename ) override;

	bool InitVGUI( IVGuiModule* module ) override;

	bool RegisterCommand( char* name, ISystemModule* module, int commandID ) override;
	void GetCommandMatches( char* string, ObjectList* pMatchList ) override;

	void ExecuteString( char* commands ) override;
	void ExecuteFile( char* filename ) override;

	void Errorf( char* fmt, ... ) override;

	char* CheckParam( char* param ) override;

	bool AddModule( ISystemModule* module, char* name ) override;
	ISystemModule* GetModule( char* interfacename, char* library, char* instancename ) override;
	bool RemoveModule( ISystemModule* module ) override;

	void Stop() override;

	char* COM_GetBaseDir() override;

	void CMD_Modules();

	void CMD_LoadModule( char* cmdLine );

	void CMD_UnloadModule( char* cmdLine );

	bool DispatchCommand( char* command );

	ISystemModule* FindModule( char* type, char* name );

	library_t* GetLibrary( char* name );

private:
	ObjectList m_Modules;
	ObjectList m_Libraries;
	ObjectList m_Commands;

	unsigned int m_SerialCounter;
	unsigned int m_Tick;
	double m_LastTime;

	IEngineWrapper* m_EngineWrapper;
};

extern SystemWrapper gSystemWrapper;

void SystemWrapper_Init();

void SystemWrapper_ShutDown();

void SystemWrapper_RunFrame( double time );

void SystemWrapper_ExecuteString( char* command );

int SystemWrapper_LoadModule( char* interfacename, char* library, char* instancename );

#endif //ENGINE_SYSTEMWRAPPER_H
