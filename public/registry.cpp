#ifdef WIN32
#include "winheaders.h"
#else

#endif

#include "registry.h"

namespace
{
static CRegistry g_Registry;
}

IRegistry* registry = &g_Registry;

#ifdef WIN32
static const char* GameName()
{
	return "Half-Life";
}

void CRegistry::Init()
{
	char SubKey[ 1024 ];

	wsprintfA( SubKey, "Software\\Valve\\%s\\Settings\\", GameName() );

	DWORD dwDisposition;

	auto result = RegCreateKeyExA( 
		HKEY_CURRENT_USER, SubKey, 
		0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, 
		&m_hKey, &dwDisposition );

	if( ERROR_SUCCESS != result )
	{
		m_bInitialized = false;
	}
	else
	{
		m_bInitialized = true;
	}
}

void CRegistry::Shutdown()
{
	if( m_bInitialized )
	{
		m_bInitialized = false;
		RegCloseKey( m_hKey );
	}
}

int CRegistry::ReadInt( const char* key, int defaultValue )
{
	if( !m_bInitialized )
		return defaultValue;

	DWORD type;
	DWORD data;
	DWORD dataSize = sizeof( data );

	if( ERROR_SUCCESS == RegQueryValueExA( 
		m_hKey, key, 
		nullptr, &type, reinterpret_cast<BYTE*>( &data ), &dataSize ) )
	{
		if( type == REG_DWORD )
			return data;
	}

	return defaultValue;
}

void CRegistry::WriteInt( const char* key, int value )
{
	if( !m_bInitialized )
		return;

	RegSetValueExA( m_hKey, key, 
					0, REG_DWORD, 
					reinterpret_cast<const BYTE*>( &value ), sizeof( value ) );
}

const char* CRegistry::ReadString( const char* key, const char* defaultValue )
{
	if( !m_bInitialized )
		return defaultValue;

	static char Data[ 512 ];

	DWORD type;
	DWORD dataSize = sizeof( Data );

	if( ERROR_SUCCESS == RegQueryValueExA(
		m_hKey, key,
		nullptr, &type, reinterpret_cast<BYTE*>( Data ), &dataSize ) )
	{
		if( type == REG_SZ )
			return Data;
	}

	return defaultValue;
}

void CRegistry::WriteString( const char* key, const char* string )
{
	if( !m_bInitialized )
		return;

	RegSetValueExA( 
		m_hKey, key, 
		0, REG_SZ, 
		reinterpret_cast<const BYTE*>( string ), strlen( string ) + 1 );
}
#else
static const char* GameName()
{
	return "hl";
}

void CRegistry::Init()
{
	LoadKeyValuesFromDisk();
}

void CRegistry::Shutdown()
{
	//Nothing
}

int CRegistry::ReadInt( const char* key, int defaultValue )
{
	GetKeyValues();

	if( m_vecValues.Count() > 0 )
	{
		const auto uiLength = strlen( key );

		for( int i = 0; i < m_vecValues.Count(); ++i )
		{
			if( !strnicmp( key, m_vecValues[ i ].key, uiLength ) )
			{
				return atoi( m_vecValues[ i ].value );
			}
		}
	}

	return defaultValue;
}

void CRegistry::WriteInt( const char* key, int value )
{
	LoadKeyValuesFromDisk();

	char szVal[ 32 ];
	snprintf( szVal, ARRAYSIZE( szVal ), "%d", value );

	int i;

	for( i = 0; i < m_vecValues.Count(); ++i )
	{
		if( !strnicmp( key, m_vecValues[ i ].key, strlen( key ) ) )
		{
			strncpy( m_vecValues[ i ].value, szVal, ARRAYSIZE( m_vecValues[ i ].value ) );
			break;
		}
	}

	if( i == m_vecValues.Count() )
	{
		auto& val = m_vecValues[ m_vecValues.AddToTail() ];

		strncpy( val.key, key, ARRAYSIZE( val.key ) );
		strncpy( val.value, szVal, ARRAYSIZE( val.value ) );
	}

	if( m_vecValues.Count() )
		WriteKeyValuesToDisk();
}

const char* CRegistry::ReadString( const char* key, const char* defaultValue )
{
	GetKeyValues();

	if( m_vecValues.Count() > 0 )
	{
		const auto uiLength = strlen( key );

		for( int i = 0; i < m_vecValues.Count(); ++i )
		{
			if( !strnicmp( key, m_vecValues[ i ].key, uiLength ) )
			{
				return m_vecValues[ i ].value;
			}
		}
	}

	return defaultValue;
}

void CRegistry::WriteString( const char* key, const char* string )
{
	LoadKeyValuesFromDisk();

	int i;

	for( i = 0; i < m_vecValues.Count(); ++i )
	{
		if( !strnicmp( key, m_vecValues[ i ].key, strlen( key ) ) )
		{
			strncpy( m_vecValues[ i ].value, string, ARRAYSIZE( m_vecValues[ i ].value ) );
			break;
		}
	}

	if( i == m_vecValues.Count() )
	{
		auto& val = m_vecValues[ m_vecValues.AddToTail() ];

		strncpy( val.key, key, ARRAYSIZE( val.key ) );
		strncpy( val.value, string, ARRAYSIZE( val.value ) );
	}

	if( m_vecValues.Count() )
		WriteKeyValuesToDisk();
}

void CRegistry::LoadKeyValuesFromDisk()
{
	if( m_fConfig )
		fclose( m_fConfig );

	m_vecValues.Purge();

	char szFileName[ MAX_PATH ];
	snprintf( szFileName, ARRAYSIZE( szFileName ), "%s.conf", "hl" );

	m_fConfig = fopen( szFileName, "r" );

	if( m_fConfig )
	{
		char szLine[ 256 ];
		KV val;

		char* pszSeparator;
		char* pszValue;
		char* pszEnd;

		if( !feof( m_fConfig ) && fgets( szLine, ARRAYSIZE( szLine ), m_fConfig ) )
		{
			pszSeparator = strchr( szLine, '=' );

			if( pszSeparator )
			{
				pszValue = pszSeparator + 1;
				*pszSeparator = '\0';

				pszEnd = &pszValue[ strlen( pszValue ) ];

				//Strip newline
				if( *pszEnd == '\n' )
					*pszEnd = '\0';

				strncpy( val.key, szLine, ARRAYSIZE( val.key ) );
				strncpy( val.value, pszValue, ARRAYSIZE( val.value ) );

				m_vecValues.AddToTail( val );
			}
		}

		fclose( m_fConfig );
		m_fConfig = nullptr;
	}
}

void CRegistry::WriteKeyValuesToDisk()
{
	char szFileName[ MAX_PATH ];
	snprintf( szFileName, ARRAYSIZE( szFileName ), "%s.conf", "hl" );

	//TODO: check if file failed to open - Solokiller
	m_fConfig = fopen( szFileName, "w+" );

	for( int i = 0; i < m_vecValues.Count(); ++i )
	{
		fprintf( m_fConfig, "%s=%s\n", m_vecValues[ i ].key, m_vecValues[ i ].value );
	}

	fclose( m_fConfig );
	m_fConfig = nullptr;
}

void CRegistry::GetKeyValues()
{
	if( !m_vecValues.Count() )
		LoadKeyValuesFromDisk();
}
#endif
