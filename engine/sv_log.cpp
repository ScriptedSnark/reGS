#include <cstdarg>
#include <cstdio>
#include <ctime>

#include "quakedef.h"
#include "buildnum.h"
#include "server.h"
#include "sv_log.h"

const int MAX_DAILY_LOG_FILES = 1000;

struct LOGLIST_T
{
	server_log_t log;
	LOGLIST_T *next;
};

static LOGLIST_T* firstLog = nullptr;

cvar_t sv_log_onefile = { "sv_log_onefile", "0" };
cvar_t sv_log_singleplayer = { "sv_log_singleplayer", "0" };
cvar_t sv_logsecret = { "sv_logsecret", "0" };
cvar_t mp_logecho = { "mp_logecho", "1" };
cvar_t mp_logfile = { "mp_logfile", "1", FCVAR_SERVER };

void Log_Printf( const char* fmt, ... )
{
	va_list va;

	va_start( va, fmt );

	if( svs.log.net_log || firstLog || svs.log.active )
	{
		static char string[ 1024 ];

		time_t ltime;
		time( &ltime );
		tm* pTime = localtime( &ltime );

		snprintf(
			string,
			sizeof( string ),
			"L %02i/%02i/%04i - %02i:%02i:%02i: ",
			pTime->tm_mon + 1,
			pTime->tm_mday,
			pTime->tm_year + 1900,
			pTime->tm_hour,
			pTime->tm_min,
			pTime->tm_sec );

		const size_t uiLength = Q_strlen( string );

		vsnprintf( &string[ uiLength ], sizeof( string ) - uiLength, fmt, va );

		if( svs.log.net_log )
		{
			//TODO: implement - Solokiller
			/*
			Netchan_OutOfBandPrint( NS_SERVER, svs.log.net_address, "log %s", string );
			*/
		}

		for( auto pLog = firstLog; pLog; pLog = pLog->next )
		{
			if( !sv_logsecret.value )
			{
				//TODO: implement - Solokiller
				//Netchan_OutOfBandPrint( NS_SERVER, pLog->log.net_address, "log %s", string );
			}
			else
			{
				//TODO: implement - Solokiller
				//Netchan_OutOfBandPrint( NS_SERVER, pLog->log.net_address, "%c%s%s", 83, sv_logsecret.string, string );
			}
		}

		if( svs.log.active && ( svs.maxclients > 1 || sv_log_singleplayer.value != 0.0 ) )
		{
			if( mp_logecho.value != 0.0 )
				Con_Printf( "%s", string );

			if( svs.log.file != FILESYSTEM_INVALID_HANDLE )
			{
				if( mp_logfile.value != 0.0 )
					FS_FPrintf( svs.log.file, "%s", string );
			}
		}
	}

	va_end( va );
}

void Log_PrintServerVars()
{
	if( svs.log.active )
	{
		Log_Printf( "Server cvars start\n" );
		for( cvar_t* i = cvar_vars; i; i = i->next )
		{
			if( ( i->flags & FCVAR_SERVER ) )
				Log_Printf( "Server cvar \"%s\" = \"%s\"\n", i->name, i->string );
		}

		Log_Printf( "Server cvars end\n" );
	}
}

void Log_Close()
{
	if( svs.log.file )
	{
		Log_Printf( "Log file closed\n" );
		FS_Close( svs.log.file );
	}
	svs.log.file = FILESYSTEM_INVALID_HANDLE;
}

void Log_Open()
{
	if( svs.log.active && ( !sv_log_onefile.value || !svs.log.file ) )
	{
		if( !mp_logfile.value )
		{
			Con_Printf( "Server logging data to console.\n" );
		}
		else
		{
			Log_Close();

			time_t ltime;
			time( &ltime );
			auto pTm = localtime( &ltime );
			auto pszLogsDir = Cvar_VariableString( "logsdir" );

			char szFileBase[ MAX_PATH ];

			if( !pszLogsDir ||
				Q_strlen( pszLogsDir ) <= 0 ||
				Q_strstr( pszLogsDir, ":" ) ||
				Q_strstr( pszLogsDir, ".." ) )
			{
				snprintf( szFileBase, ARRAYSIZE( szFileBase ), "logs/L%02i%02i", pTm->tm_mon + 1, pTm->tm_mday );
			}
			else
			{
				snprintf( szFileBase, ARRAYSIZE( szFileBase ), "%s/L%02i%02i", pszLogsDir, pTm->tm_mon + 1, pTm->tm_mday );
			}

			char szTestFile[ MAX_PATH ];

			int i;

			for( i = 0; i < MAX_DAILY_LOG_FILES; ++i )
			{
				snprintf( szTestFile, ARRAYSIZE( szTestFile ), "%s%03i.log", szFileBase, i );
				
				COM_FixSlashes( szTestFile );
				COM_CreatePath( szTestFile );

				auto hFile = FS_OpenPathID( szTestFile, "r", "GAMECONFIG" );
				if( !hFile )
					break;

				FS_Close( hFile );
			}

			if( i < MAX_DAILY_LOG_FILES )
			{
				COM_CreatePath( szTestFile );

				auto hFile = FS_OpenPathID( szTestFile, "wt", "GAMECONFIG" );

				if( hFile )
				{
					Con_Printf( "Server logging data to file %s\n", szTestFile );
					svs.log.file = hFile;

					Log_Printf(
						"Log file started (file \"%s\") (game \"%s\") (version \"%i/%s/%d\")\n",
						szTestFile,
						Info_ValueForKey( Info_Serverinfo(), "*gamedir" ),
						PROTOCOL_VERSION,
						gpszVersionString,
						build_number()
					);
					return;
				}
			}

			Con_Printf( "Unable to open logfiles under %s\nLogging disabled\n", szFileBase );
			svs.log.active = false;
		}
	}
}
