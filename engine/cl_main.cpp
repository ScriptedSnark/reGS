/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "quakedef.h"
#include "client.h"
#include "cl_main.h"
#include "cl_parse.h"
#include "cl_spectator.h"
#include "cl_tent.h"
#include "com_custom.h"
#include "gl_rmain.h"
#include "hashpak.h"
#include "host.h"
#include "pmove.h"
#include "tmessage.h"

client_static_t cls;
client_state_t	cl;

cl_entity_t* cl_entities = nullptr;

// TODO, allocate dynamically
efrag_t cl_efrags[ MAX_EFRAGS ] = {};
dlight_t cl_dlights[ MAX_DLIGHTS ] = {};
dlight_t cl_elights[ MAX_ELIGHTS ] = {};
lightstyle_t cl_lightstyle[ MAX_LIGHTSTYLES ] = {};

//TODO: implement API and add here - Solokiller
playermove_t g_clmove;

float g_LastScreenUpdateTime = 0;

cvar_t cl_mousegrab = { "cl_mousegrab", "1", FCVAR_ARCHIVE };
cvar_t m_rawinput = { "m_rawinput", "1", FCVAR_ARCHIVE };
cvar_t rate = { "rate", "30000", FCVAR_USERINFO };
cvar_t fs_perf_warnings = { "fs_perf_warnings", "0" };
cvar_t cl_lw = { "cl_lw", "1", FCVAR_ARCHIVE | FCVAR_USERINFO };

static int g_iCurrentTiming = 0;

startup_timing_t g_StartupTimings[ MAX_STARTUP_TIMINGS ] = {};

void SetupStartupTimings()
{
	g_iCurrentTiming = 0;
	g_StartupTimings[ g_iCurrentTiming ].name = "Startup";
	g_StartupTimings[ g_iCurrentTiming ].time = Sys_FloatTime();
}

void AddStartupTiming( const char* name )
{
	++g_iCurrentTiming;
	g_StartupTimings[ g_iCurrentTiming ].name = name;
	g_StartupTimings[ g_iCurrentTiming ].time = Sys_FloatTime();
}

void PrintStartupTimings()
{
	Con_Printf( "Startup timings (%.2f total)\n", g_StartupTimings[ g_iCurrentTiming ].time - g_StartupTimings[ 0 ].time );
	Con_Printf( "    0.00    Startup\n" );

	//Print the relative time between each system startup
	for( int i = 1; i < g_iCurrentTiming; ++i )
	{
		Con_Printf( "    %.2f    %s\n",
					g_StartupTimings[ i ].time - g_StartupTimings[ i - 1 ].time,
					g_StartupTimings[ i ].name
		);
	}
}

void CL_ShutDownClientStatic()
{
	//TODO: implement - Solokiller
}

void CL_Shutdown()
{
	//TODO: implement - Solokiller
	TextMessageShutdown();
	//TODO: implement - Solokiller
}

void CL_Init()
{
	//TODO: implement - Solokiller
	TextMessageInit();
	//TODO: implement - Solokiller
	Cvar_RegisterVariable( &rate );
	Cvar_RegisterVariable( &cl_lw );
	Cvar_RegisterVariable( &dev_overview );
	Cvar_RegisterVariable( &cl_mousegrab );
	Cvar_RegisterVariable( &m_rawinput );
	//TODO: implement - Solokiller
}

dlight_t* CL_AllocDlight( int key )
{
	//TODO: implement - Solokiller
	return nullptr;
}

dlight_t* CL_AllocElight( int key )
{
	//TODO: implement - Solokiller
	return nullptr;
}

model_t* CL_GetModelByIndex( int index )
{
	//TODO: implement - Solokiller
	return nullptr;
}

void CL_GetPlayerHulls()
{
	for( int i = 0; i < 4; ++i )
	{
		if( !ClientDLL_GetHullBounds( i, player_mins[ i ], player_maxs[ i ] ) )
			break;
	}
}

bool UserIsConnectedOnLoopback()
{
	//TODO: implement - Solokiller
	return false;
}

void SetPal( int i )
{
	//Nothing
}

void GetPos( vec3_t origin, vec3_t angles )
{
	origin[ 0 ] = r_refdef.vieworg[ 0 ];
	origin[ 1 ] = r_refdef.vieworg[ 1 ];
	origin[ 2 ] = r_refdef.vieworg[ 2 ];

	angles[ 0 ] = r_refdef.viewangles[ 0 ];
	angles[ 1 ] = r_refdef.viewangles[ 1 ];
	angles[ 2 ] = r_refdef.viewangles[ 2 ];

	if( Cmd_Argc() == 2 )
	{
		if( Q_atoi( Cmd_Argv( 1 ) ) == 2 && cls.state == ca_active )
		{
			origin[ 0 ] = cl.frames[ cl.parsecountmod ].playerstate[ cl.playernum ].origin[ 0 ];
			origin[ 1 ] = cl.frames[ cl.parsecountmod ].playerstate[ cl.playernum ].origin[ 1 ];
			origin[ 2 ] = cl.frames[ cl.parsecountmod ].playerstate[ cl.playernum ].origin[ 2 ];

			angles[ 0 ] = cl.frames[ cl.parsecountmod ].playerstate[ cl.playernum ].angles[ 0 ];
			angles[ 1 ] = cl.frames[ cl.parsecountmod ].playerstate[ cl.playernum ].angles[ 1 ];
			angles[ 2 ] = cl.frames[ cl.parsecountmod ].playerstate[ cl.playernum ].angles[ 2 ];
		}
	}
}

const char* CL_CleanFileName( const char* filename )
{
	if( filename && *filename && *filename == '!' )
		return "customization";

	return filename;
}

void CL_ClearCaches()
{
	for( int i = 1; i < ARRAYSIZE( cl.event_precache ) && cl.event_precache[ i ].pszScript; ++i )
	{
		Mem_Free( const_cast<char*>( cl.event_precache[ i ].pszScript ) );
		Mem_Free( const_cast<char*>( cl.event_precache[ i ].filename ) );

		Q_memset( &cl.event_precache[ i ], 0, sizeof( cl.event_precache[ i ] ) );
	}
}

void CL_ClearClientState()
{
	for( int i = 0; i < CL_UPDATE_BACKUP; ++i )
	{
		if( cl.frames[ i ].packet_entities.entities )
		{
			Mem_Free( cl.frames[ i ].packet_entities.entities );
		}

		cl.frames[ i ].packet_entities.entities = nullptr;
		cl.frames[ i ].packet_entities.num_entities = 0;
	}

	CL_ClearResourceLists();

	for( int i = 0; i < MAX_CLIENTS; ++i )
	{
		COM_ClearCustomizationList( &cl.players[ i ].customdata, false );
	}

	CL_ClearCaches();

	Q_memset( &cl, 0, sizeof( cl ) );

	cl.resourcesneeded.pPrev = &cl.resourcesneeded;
	cl.resourcesneeded.pNext = &cl.resourcesneeded;
	cl.resourcesonhand.pPrev = &cl.resourcesonhand;
	cl.resourcesonhand.pNext = &cl.resourcesonhand;

	CL_CreateResourceList();
}

void CL_ClearState( bool bQuiet )
{
	if( !Host_IsServerActive() )
		Host_ClearMemory( bQuiet );

	CL_ClearClientState();

	//TODO: implement - Solokiller
	//SZ_Clear( &cls.netchan.message );

	// clear other arrays
	Q_memset( cl_efrags, 0, sizeof( cl_efrags ) );
	Q_memset( cl_dlights, 0, sizeof( cl_dlights ) );
	Q_memset( cl_elights, 0, sizeof( cl_elights ) );
	Q_memset( cl_lightstyle, 0, sizeof( cl_lightstyle ) );

	CL_TempEntInit();

	//
	// allocate the efrags and chain together into a free list
	//
	cl.free_efrags = cl_efrags;

	int i;
	for( i = 0; i < MAX_EFRAGS - 1; ++i )
	{
		cl.free_efrags[ i ].entnext = &cl.free_efrags[ i + 1 ];
	}

	cl.free_efrags[ i ].entnext = nullptr;
}

void CL_CreateResourceList()
{
	if( cls.state != ca_dedicated )
	{
		HPAK_FlushHostQueue();

		cl.num_resources = 0;

		char szFileName[ MAX_PATH ];
		snprintf( szFileName, ARRAYSIZE( szFileName ), "tempdecal.wad" );

		byte rgucMD5_hash[ 16 ];
		Q_memset( rgucMD5_hash, 0, sizeof( rgucMD5_hash ) );

		auto hFile = FS_Open( szFileName, "rb" );

		if( FILESYSTEM_INVALID_HANDLE != hFile )
		{
			const auto uiSize = FS_Size( hFile );

			MD5_Hash_File( rgucMD5_hash, szFileName, false, false, nullptr );

			if( uiSize )
			{
				if( cl.num_resources > 1279 )
					Sys_Error( "Too many resources on client." );

				auto pResource = &cl.resourcelist[ cl.num_resources ];

				++cl.num_resources;

				Q_strncpy( pResource->szFileName, szFileName, ARRAYSIZE( pResource->szFileName ) );
				pResource->type = t_decal;
				pResource->nDownloadSize = uiSize;
				pResource->nIndex = 0;
				pResource->ucFlags |= RES_CUSTOM;

				Q_memcpy( pResource->rgucMD5_hash, rgucMD5_hash, sizeof( rgucMD5_hash ) );

				HPAK_AddLump( false, "custom.hpk", pResource, nullptr, hFile );
			}

			FS_Close( hFile );
		}
	}
}
