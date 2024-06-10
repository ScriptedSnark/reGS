#include "quakedef.h"

#include "BaseSystemModule.h"
#include "IBaseSystem.h"

bool BaseSystemModule::Init( IBaseSystem* system, int serial, char* name )
{
	if( !system )
		return false;

	m_State = SYSMODSTATE_INIT;

	m_System = system;
	m_Serial = serial;
	m_SystemTime = 0;

	if( name )
	{
		strncpy( m_Name, name, ARRAYSIZE( m_Name ) - 1 );
		m_Name[ ARRAYSIZE( m_Name ) - 1 ] = '\0';
	}

	m_Listener.Init();

	return true;
}

void BaseSystemModule::RunFrame( double time )
{
	m_SystemTime = time;
}

void BaseSystemModule::ReceiveSignal( ISystemModule* module, unsigned int signal, void* data )
{
	m_System->DPrintf(
		"WARNING! Unhandled signal (%i) from module %s.\n",
		signal,
		module->GetName()
	);
}

void BaseSystemModule::ExecuteCommand( int commandID, char* commandLine )
{
	m_System->DPrintf( "WARNING! Undeclared ExecuteCommand().\n" );
}

void BaseSystemModule::RegisterListener( ISystemModule* module )
{
	for( auto pObject = reinterpret_cast<ISystemModule*>( m_Listener.GetFirst() );
		 pObject;
		 pObject = reinterpret_cast<ISystemModule*>( m_Listener.GetNext() ) )
	{
		if( pObject->GetSerial() == module->GetSerial() )
		{
			m_System->DPrintf( "WARNING! BaseSystemModule::RegisterListener: module %s already added.\n", module->GetName() );
			return;
		}
	}

	m_Listener.Add( module );
}

void BaseSystemModule::RemoveListener( ISystemModule* module )
{
	auto pObject = reinterpret_cast<ISystemModule*>( m_Listener.GetFirst() );

	if( !pObject )
		return;

	for( ;
		 pObject;
		 pObject = reinterpret_cast<ISystemModule*>( m_Listener.GetNext() ) )
	{
		if( pObject->GetSerial() == module->GetSerial() )
			break;
	}

	if( pObject )
		m_Listener.Remove( pObject );
}

IBaseSystem* BaseSystemModule::GetSystem()
{
	return m_System;
}

unsigned int BaseSystemModule::GetSerial()
{
	return m_Serial;
}

char* BaseSystemModule::GetStatusLine()
{
	return "No status available.\n";
}

char* BaseSystemModule::GetType()
{
	return "GenericModule";
}

char* BaseSystemModule::GetName()
{
	return m_Name;
}

unsigned int BaseSystemModule::GetState()
{
	return m_State;
}

int BaseSystemModule::GetVersion()
{
	return COM_BuildNumber();
}

void BaseSystemModule::ShutDown()
{
	if( m_State == SYSMODSTATE_SHUTDOWN )
		return;

	m_Listener.Clear( false );

	m_State = SYSMODSTATE_SHUTDOWN;

	if( !m_System->RemoveModule( this ) )
	{
		m_System->DPrintf( "ERROR! BaseSystemModule::ShutDown: failed to remove module %s.\n",
						   m_Name );
	}
}

char* BaseSystemModule::COM_GetBaseDir()
{
	return "";
}

void BaseSystemModule::FireSignal( unsigned int signal, void* data )
{
	for( auto pObject = reinterpret_cast<ISystemModule*>( m_Listener.GetFirst() );
		 pObject;
		 pObject = reinterpret_cast<ISystemModule*>( m_Listener.GetNext() ) )
	{
		pObject->ReceiveSignal( this, signal, data );
	}
}