#include <cstdarg>

#include "quakedef.h"
#include "buildnum.h"
#include "FilePaths.h"
#include "SystemWrapper.h"
#include "TokenLine.h"

void SystemWrapperCommandForwarder();

void COM_RemoveEvilChars( char* string )
{
	for( char* i = string; i && *i; ++i )
	{
		if( *i == '%' || *i < ' ' )
			*i = ' ';
	}
}

SystemWrapper gSystemWrapper;

bool EngineWrapper::Init( IBaseSystem* system, int serial, char* name )
{
	return BaseSystemModule::Init( system, serial, name );
}

void EngineWrapper::RunFrame( double time )
{
	BaseSystemModule::RunFrame( time );
}

void EngineWrapper::ReceiveSignal( ISystemModule* module, unsigned int signal, void* data )
{
	BaseSystemModule::ReceiveSignal( module, signal, data );
}

void EngineWrapper::ExecuteCommand( int commandID, char* commandLine )
{
	BaseSystemModule::ExecuteCommand( commandID, commandLine );
}

void EngineWrapper::RegisterListener( ISystemModule* module )
{
	BaseSystemModule::RegisterListener( module );
}

void EngineWrapper::RemoveListener( ISystemModule* module )
{
	BaseSystemModule::RemoveListener( module );
}

IBaseSystem* EngineWrapper::GetSystem()
{
	return BaseSystemModule::GetSystem();
}

unsigned int EngineWrapper::GetSerial()
{
	return BaseSystemModule::GetSerial();
}

char* EngineWrapper::GetStatusLine()
{
	return "No status available.\n";
}

char* EngineWrapper::GetType()
{
	return "enginewrapper001";
}

char* EngineWrapper::GetName()
{
	return BaseSystemModule::GetName();
}

unsigned int EngineWrapper::GetState()
{
	return BaseSystemModule::GetState();
}

int EngineWrapper::GetVersion()
{
	return BaseSystemModule::GetVersion();
}

void EngineWrapper::ShutDown()
{
	BaseSystemModule::ShutDown();
}

bool EngineWrapper::GetViewOrigin( float* origin )
{
	//TODO: implement - Solokiller
	return true;
}

bool EngineWrapper::GetViewAngles( float* angles )
{
	//TODO: implement - Solokiller
	return true;
}

int EngineWrapper::GetTraceEntity()
{
	//Nothing
	return 0;
}

float EngineWrapper::GetCvarFloat( char* szName )
{
	auto pCvar = Cvar_FindVar( szName );

	if( !pCvar )
		return 0;

	return pCvar->value;
}

char* EngineWrapper::GetCvarString( char* szName )
{
	auto pCvar = Cvar_FindVar( szName );

	if( !pCvar )
		return nullptr;

	return pCvar->string;
}

void EngineWrapper::SetCvar( char* szName, char* szValue )
{
	Cvar_Set( szName, szValue );
}

void EngineWrapper::Cbuf_AddText( char* text )
{
	Cbuf_AddText( text );
}

void EngineWrapper::DemoUpdateClientData( client_data_t* cdat )
{
	//TODO: implement - Solokiller
}

void EngineWrapper::CL_QueueEvent( int flags, int index, float delay, event_args_t* pargs )
{
	//TODO: implement - Solokiller
}

void EngineWrapper::HudWeaponAnim( int iAnim, int body )
{
	//TODO: implement - Solokiller
}

void EngineWrapper::CL_DemoPlaySound( int channel, char* sample, float attenuation, float volume, int flags, int pitch )
{
	//TODO: implement - Solokiller
}

void EngineWrapper::ClientDLL_ReadDemoBuffer( int size, byte* buffer )
{
	//TODO: implement - Solokiller
}

bool SystemWrapper::Init( IBaseSystem* system, int serial, char* name )
{
	BaseSystemModule::Init( system, serial, name );

	m_Commands.Init();
	m_Modules.Init();
	m_Libraries.Init();
	m_Listener.Init();

	auto pEngineWrapper = new EngineWrapper();
	m_EngineWrapper = pEngineWrapper;

	AddModule( pEngineWrapper, "enginewrapper001" );

	m_State = SYSMODSTATE_RUNNING;

	return true;
}

void SystemWrapper::RunFrame( double time )
{
	m_SystemTime += time;
	++m_Tick;

	if( m_State == SYSMODSTATE_RUNNING )
	{
		for( auto pObject = reinterpret_cast<ISystemModule*>( m_Listener.GetFirst() );
			 pObject;
			 pObject = reinterpret_cast<ISystemModule*>( m_Listener.GetNext() ) )
		{
			if( m_State == SYSMODSTATE_SHUTDOWN )
				break;

			pObject->RunFrame( m_SystemTime );
		}

		m_LastTime = m_SystemTime;
	}
}

void SystemWrapper::ReceiveSignal( ISystemModule* module, unsigned int signal, void* data )
{
	BaseSystemModule::ReceiveSignal( module, signal, data );
}

void SystemWrapper::ExecuteCommand( int commandID, char* commandLine )
{
	switch( commandID )
	{
	case CMD_ID_MODULES:
		CMD_Modules();
		break;

	case CMD_ID_LOADMODULE:
		CMD_LoadModule( commandLine );
		break;

	case CMD_ID_UNLOADMODULE:
		CMD_UnloadModule( commandLine );
		break;

	default:
		Printf( "SystemWrapper::ExecuteCommand: unknown command ID %i.\n", commandID );
		break;
	}
}

void SystemWrapper::RegisterListener( ISystemModule* module )
{
	BaseSystemModule::RegisterListener( module );
}

void SystemWrapper::RemoveListener( ISystemModule* module )
{
	BaseSystemModule::RemoveListener( module );
}

IBaseSystem* SystemWrapper::GetSystem()
{
	return BaseSystemModule::GetSystem();
}

unsigned int SystemWrapper::GetSerial()
{
	return BaseSystemModule::GetSerial();
}

char* SystemWrapper::GetStatusLine()
{
	return "No status available.\n";
}

char* SystemWrapper::GetType()
{
	return "basesystem002";
}

char* SystemWrapper::GetName()
{
	return BaseSystemModule::GetName();
}

unsigned int SystemWrapper::GetState()
{
	return BaseSystemModule::GetState();
}

int SystemWrapper::GetVersion()
{
	return BaseSystemModule::GetVersion();
}

void SystemWrapper::ShutDown()
{
	m_Listener.Clear( false );
	m_Commands.Clear( true );

	while( true )
	{
		//The module will remove itself on shutdown.
		auto pObject = reinterpret_cast<ISystemModule*>( m_Modules.GetFirst() );

		if( !pObject )
			break;

		pObject->ShutDown();
	}

	while( true )
	{
		auto pLib = reinterpret_cast<library_t*>( m_Libraries.RemoveTail() );

		if( !pLib )
			break;

		if( pLib->handle )
			Sys_UnloadModule( pLib->handle );

		Mem_Free( pLib );
	}

	if( m_EngineWrapper )
		m_EngineWrapper->ShutDown();

	Cmd_RemoveWrapperCmds();

	m_State = SYSMODSTATE_SHUTDOWN;
}

double SystemWrapper::GetTime()
{
	return m_SystemTime;
}

unsigned int SystemWrapper::GetTick()
{
	return m_Tick;
}

void SystemWrapper::SetFPS( float fps )
{
	//Nothing
}

void SystemWrapper::Printf( char* fmt, ... )
{
	static char string[ 8192 ];

	va_list va;

	va_start( va, fmt );
	vsnprintf( string, ARRAYSIZE( string ), fmt, va );
	va_end( va );

	Con_Printf( "%s", string );
}

void SystemWrapper::DPrintf( char* fmt, ... )
{
	static char string[ 8192 ];

	va_list va;

	va_start( va, fmt );
	vsnprintf( string, ARRAYSIZE( string ), fmt, va );
	va_end( va );

	//TODO: shouldn't directly pass this in - Solokiller
	Con_Printf( string );
}

void SystemWrapper::RedirectOutput( char* buffer, int maxSize )
{
	//Nothing
	Con_Printf( "WARNIG! SystemWrapper::RedirectOutput: not implemented.\n" );
}

IFileSystem* SystemWrapper::GetFileSystem()
{
	return g_pFileSystem;
}

byte* SystemWrapper::LoadFile( const char* name, int* length )
{
	return COM_LoadFile( name, 5, length );
}

void SystemWrapper::FreeFile( byte* fileHandle )
{
	COM_FreeFile( fileHandle );
}

void SystemWrapper::SetTitle( char* text )
{
	//Nothing
	Con_Printf( "TODO: SystemWrapper::SetTitle ?\n" );
}

void SystemWrapper::SetStatusLine( char* text )
{
	//Nothing
	Con_Printf( "TODO: SystemWrapper::SetStatusLine ?\n" );
}

void SystemWrapper::ShowConsole( bool visible )
{
	//Nothing
}

void SystemWrapper::LogConsole( char* filename )
{
	if( filename )
		Cmd_ExecuteString( "log on", src_command );
	else
		Cmd_ExecuteString( "log off", src_command );
}

bool SystemWrapper::InitVGUI( IVGuiModule* module )
{
	//Nothing
	return false;
}

bool SystemWrapper::RegisterCommand( char* name, ISystemModule* module, int commandID )
{
	for( auto pObject = reinterpret_cast<command_t*>( m_Commands.GetFirst() );
		 pObject;
		 pObject = reinterpret_cast<command_t*>( m_Commands.GetNext() ) )
	{
		if( !stricmp( pObject->name, name ) )
		{
			Printf( "WARNING! System::RegisterCommand: command \"%s\" already exists.\n", name );
			return false;
		}
	}

	auto pCmd = reinterpret_cast<command_t*>( Mem_ZeroMalloc( sizeof( command_t ) ) );

	strncpy( pCmd->name, name, ARRAYSIZE( pCmd->name ) );
	pCmd->name[ ARRAYSIZE( pCmd->name ) - 1 ] = '\0';

	pCmd->module = module;
	pCmd->commandID = commandID;

	m_Commands.Add( pCmd );

	Cmd_AddWrapperCommand( pCmd->name, &SystemWrapperCommandForwarder );

	return true;
}

void SystemWrapper::GetCommandMatches( char* string, ObjectList* pMatchList )
{
	pMatchList->Clear( true );
}

void SystemWrapper::ExecuteString( char* commands )
{
	if( !commands || !( *commands ) )
		return;

	//Remove format characters to block format string attacks.
	COM_RemoveEvilChars( commands );

	char singleCmd[ 256 ];

	char* pszDest;

	const char* pszSource = commands;

	bool bInQuote = false;

	while( *pszSource )
	{
		//Parse out single commands and execute them.
		pszDest = singleCmd;

		int i;
		for( i = 0; i < ARRAYSIZE( singleCmd ); ++i )
		{
			char c = *pszSource++;

			*pszDest++ = c;

			if( c == '"' )
			{
				bInQuote = !bInQuote;
			}
			else if( c == ';' && !bInQuote )
			{
				//End of command and not in a quoted string.
				break;
			}
		}

		if( i >= ARRAYSIZE( singleCmd ) )
		{
			Printf( "WARNING! System::ExecuteString: Command token too long.\n" );
			return;
		}

		*pszDest = '\0';

		char* pszCmd = singleCmd;

		while( *pszCmd == ' ' )
		{
			++pszCmd;
		}

		DispatchCommand( pszCmd );
	}
}

void SystemWrapper::ExecuteFile( char* filename )
{
	char cmd[ 1024 ];

	snprintf( cmd, ARRAYSIZE( cmd ) - 1, "exec %s\n", filename );
	cmd[ ARRAYSIZE( cmd ) - 1 ] = '\0';

	Cmd_ExecuteString( cmd, src_command );
}

void SystemWrapper::Errorf( char* fmt, ... )
{
	static char string[ 1024 ];

	va_list va;

	va_start( va, fmt );
	vsnprintf( string, ARRAYSIZE( string ), fmt, va );
	va_end( va );

	Printf( "***** FATAL ERROR *****\n" );
	Printf( "%s", string );
	Printf( "*** STOPPING SYSTEM ***\n" );

	Stop();
}

char* SystemWrapper::CheckParam( char* param )
{
	const int index = COM_CheckParm( param );

	if( !index )
		return nullptr;

	if( index + 1 >= com_argc )
		return "";

	return const_cast<char*>( com_argv[ index + 1 ] );
}

bool SystemWrapper::AddModule( ISystemModule* module, char* name )
{
	if( !module )
		return false;

	if( !module->Init( this, m_SerialCounter, name ) )
	{
		Printf( "ERROR! System::AddModule: couldn't initialize module %s.\n", name );
		return false;
	}

	m_Modules.AddHead( module );

	++m_SerialCounter;

	return true;
}

ISystemModule* SystemWrapper::GetModule( char* interfacename, char* library, char* instancename )
{
	auto pModule = FindModule( interfacename, instancename );

	if( pModule )
		return pModule;

	auto pLib = GetLibrary( library );

	if( !pLib )
		return nullptr;

	pModule = static_cast<ISystemModule*>( pLib->createInterfaceFn( interfacename, nullptr ) );

	if( pModule )
	{
		AddModule( pModule, instancename );
		return pModule;
	}

	Printf( "ERROR! System::GetModule: interface \"%s\" not found in library %s.\n",
			interfacename, pLib->name );

	return nullptr;
}

bool SystemWrapper::RemoveModule( ISystemModule* module )
{
	if( !module )
		return true;

	module->ShutDown();

	//Remove all commands.
	for( auto pObject = reinterpret_cast<command_t*>( m_Commands.GetFirst() );
		 pObject;
		 pObject = reinterpret_cast<command_t*>( m_Commands.GetNext() ) )
	{
		if( pObject->module->GetSerial() == module->GetSerial() )
		{
			m_Commands.Remove( pObject );
			Mem_Free( pObject );
		}
	}

	for( auto pObject = reinterpret_cast<ISystemModule*>( m_Modules.GetFirst() );
		 pObject;
		 pObject = reinterpret_cast<ISystemModule*>( m_Modules.GetNext() ) )
	{
		if( pObject->GetSerial() == module->GetSerial() )
		{
			m_Modules.Remove( pObject );
			return true;
		}
	}

	return false;
}

void SystemWrapper::Stop()
{
	m_State = SYSMODSTATE_SHUTDOWN;
}

char* SystemWrapper::COM_GetBaseDir()
{
	return BaseSystemModule::COM_GetBaseDir();
}

void SystemWrapper::CMD_Modules()
{
	for( auto pObject = reinterpret_cast<ISystemModule*>( m_Modules.GetFirst() );
		 pObject;
		 pObject = reinterpret_cast<ISystemModule*>( m_Modules.GetNext() ) )
	{
		Printf( "%s(%s):%s",
				pObject->GetName(),
				pObject->GetType(),
				pObject->GetStatusLine()
		);
	}

	Printf( "--- %i modules in total ---\n", m_Modules.CountElements() );
}

void SystemWrapper::CMD_LoadModule( char* cmdLine )
{
	TokenLine params( cmdLine );

	if( params.CountToken() <= 1 )
	{
		Printf( "Syntax: loadmodule <module> [<library>] [<name>]\n" );
		return;
	}

	char* pszModuleName = params.GetToken( 1 );
	char* pszLibName;
	char* pszName = nullptr;

	if( params.CountToken() == 2 )
	{
		pszLibName = pszModuleName;
	}
	else
	{
		pszLibName = params.GetToken( 2 );

		if( params.CountToken() != 3 )
		{
			pszName = params.GetToken( 3 );
		}
	}

	GetModule( pszModuleName, pszLibName, pszName );
}

void SystemWrapper::CMD_UnloadModule( char* cmdLine )
{
	TokenLine params( cmdLine );

	if( params.CountToken() <= 1 )
	{
		Printf( "Syntax: unloadmodule <module> [<name>]\n" );
		return;
	}

	char* pszType = params.GetToken( 1 );
	char* pszName = nullptr;

	if( params.CountToken() == 2 || params.CountToken() == 3 )
	{
		if( params.CountToken() == 3 )
		{
			pszName = params.GetToken( 2 );
		}

		auto pModule = FindModule( pszType, pszName );

		if( pModule )
		{
			RemoveModule( pModule );
			return;
		}
	}

	Printf( "Module not found.\n" );
}

bool SystemWrapper::DispatchCommand( char* command )
{
	if( !command || !( *command ) )
		return true;

	for( auto pObject = reinterpret_cast<command_t*>( m_Commands.GetFirst() );
		 pObject;
		 pObject = reinterpret_cast<command_t*>( m_Commands.GetNext() ) )
	{
		if( !strnicmp( pObject->name, command, strlen( pObject->name ) ) )
		{
			pObject->module->ExecuteCommand( pObject->commandID, command );
			return true;
		}
	}

	Cmd_ExecuteString( command, src_command );
	return true;
}

ISystemModule* SystemWrapper::FindModule( char* type, char* name )
{
	if( !type || !( *type ) )
		return nullptr;

	for( auto pObject = reinterpret_cast<ISystemModule*>( m_Modules.GetFirst() );
		 pObject;
		 pObject = reinterpret_cast<ISystemModule*>( m_Modules.GetNext() ) )
	{
		if( !stricmp( type, pObject->GetType() ) )
		{
			if( !name || !stricmp( name, pObject->GetName() ) )
			{
				return pObject;
			}
		}
	}

	return nullptr;
}

SystemWrapper::library_t* SystemWrapper::GetLibrary( char* name )
{
	char fixedname[ MAX_PATH ];

	strncpy( fixedname, name, ARRAYSIZE( fixedname ) - 1 );
	//TODO: no null termination - Solokiller

	COM_FixSlashes( fixedname );

	for( auto pObject = reinterpret_cast<library_t*>( m_Libraries.GetFirst() );
		 pObject;
		 pObject = reinterpret_cast<library_t*>( m_Libraries.GetNext() ) )
	{
		if( !stricmp( name, pObject->name ) )
		{
			return pObject;
		}
	}

	auto pLib = reinterpret_cast<library_t*>( Mem_ZeroMalloc( sizeof( library_t ) ) );

	if( !pLib )
	{
		DPrintf( "ERROR! System::GetLibrary: out of memory (%s).\n",
				 name
		);

		return nullptr;
	}

	snprintf( pLib->name, ARRAYSIZE( pLib->name ), "%s" DEFAULT_SO_EXT, fixedname );

	FS_GetLocalCopy( pLib->name );
	pLib->handle = Sys_LoadModule( pLib->name );

	if( pLib->handle )
	{
		pLib->createInterfaceFn = Sys_GetFactory( pLib->handle );

		if( pLib->createInterfaceFn )
		{
			m_Libraries.Add( pLib );

			DPrintf( "Loaded library %s.\n", pLib->name );

			return pLib;
		}

		DPrintf( "ERROR! System::GetLibrary: coulnd't get object factory(%s).\n", pLib->name );

		//TODO: unload library - Solokiller
	}

	DPrintf( "ERROR! System::GetLibrary: coulnd't load library (%s).\n", pLib->name );

	Mem_Free( pLib );

	return nullptr;
}

int COM_BuildNumber()
{
	return build_number();
}

void SystemWrapper_Init()
{
	gSystemWrapper.Init( &gSystemWrapper, 0, "SystemWrapper" );
}

void SystemWrapper_ShutDown()
{
	gSystemWrapper.ShutDown();
}

void SystemWrapper_RunFrame( double time )
{
	gSystemWrapper.RunFrame( time );
}

void SystemWrapper_ExecuteString( char* command )
{
	if( command && *command )
	{
		gSystemWrapper.ExecuteString( command );
	}
}

void SystemWrapperCommandForwarder()
{
	char cmd[ 1024 ];

	strcpy( cmd, Cmd_Argv( 0 ) );

	//Add command args
	if( Cmd_Argc() > 1 )
	{
		const size_t uiLength = strlen( cmd );

		//TODO: not safe - Solokiller
		cmd[ uiLength ] = ' ';
		cmd[ uiLength + 1 ] = '\0';
		strcat( cmd, Cmd_Args() );
	}

	cmd[ ARRAYSIZE( cmd ) - 1 ] = '\0';

	SystemWrapper_ExecuteString( cmd );
}

int SystemWrapper_LoadModule( char* interfacename, char* library, char* instancename )
{
	return gSystemWrapper.GetModule( interfacename, library, instancename ) != nullptr;
}
