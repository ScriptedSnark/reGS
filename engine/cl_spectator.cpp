#include "quakedef.h"
#include "client.h"
#include "cl_ents.h"
#include "cl_main.h"
#include "cl_pred.h"
#include "cl_spectator.h"
#include "pm_movevars.h"
#include "pmove.h"

overviewInfo_t gDevOverview = {};
local_state_t spectatorState = {};

cvar_t dev_overview = { "dev_overview", "0" };

int CL_IsSpectateOnly()
{
	g_engdstAddrs.IsSpectateOnly();

	return cls.spectator;
}

bool CL_IsDevOverviewMode()
{
	return dev_overview.value && ( allow_cheats || cls.spectator );
}

void CL_CalculateDevOverviewParameters()
{
	//TODO: make sure these constants are defined and adjusted for maximum map size - Solokiller
	float xSize = -64000.0;
	float ySize = -64000.0;

	gDevOverview.z_min = -31999;
	gDevOverview.z_max = 31999;
	gDevOverview.origin[ 0 ] = 0;
	gDevOverview.origin[ 1 ] = 0;
	gDevOverview.origin[ 2 ] = 0;

	if( cl.worldmodel->numvertexes > 0 )
	{
		float xMax = -32000.0;
		float yMax = -32000.0;
		float zMax = -32000.0;

		float xMin = 32000.0;
		float yMin = 32000.0;
		float zMin = 32000.0;
		
		for( int i = 0; i < cl.worldmodel->numvertexes; ++i )
		{
			const auto& vertex = cl.worldmodel->vertexes[ i ];

			xMax = max( xMax, vertex.position[ 0 ] );
			yMax = max( yMax, vertex.position[ 1 ] );
			zMax = max( zMax, vertex.position[ 2 ] );

			xMin = min( xMin, vertex.position[ 0 ] );
			yMin = min( yMin, vertex.position[ 1 ] );
			zMin = min( zMin, vertex.position[ 2 ] );
		}

		ySize = yMax - yMin;
		xSize = xMax - xMin;

		gDevOverview.z_min = zMax + 1.0;
		gDevOverview.z_max = zMin - 1.0;

		gDevOverview.origin[ 0 ] = ( xMax + xMin ) * 0.5;
		gDevOverview.origin[ 1 ] = ( yMax + yMin ) * 0.5;
		gDevOverview.origin[ 2 ] = ( zMax + zMin ) * 0.5;
	}

	if( ySize > xSize )
	{
		gDevOverview.rotated = false;
		gDevOverview.zoom = min( 8192.0 / ( xSize * 1.3333 ), 8192.0 / ySize );
	}
	else
	{
		gDevOverview.rotated = true;
		gDevOverview.zoom = min( 8192.0 / ( ySize * 1.3333 ), 8192.0 / xSize );
	}
}

void CL_InitSpectator()
{
	Q_memset( &spectatorState, 0, sizeof( spectatorState ) );
	Q_memset( &gDevOverview, 0, sizeof( gDevOverview ) );

	CL_CalculateDevOverviewParameters();

	if( cls.spectator )
	{
		spectatorState.playerstate.friction = 1;
		spectatorState.playerstate.gravity = 1;
		spectatorState.playerstate.number = cl.playernum + 1;
		spectatorState.playerstate.usehull = 1;
		spectatorState.playerstate.movetype = 8;
		spectatorState.client.maxspeed = movevars.spectatormaxspeed;
	}
}

bool CL_AddEntityToPhysList( int entIndex )
{
	if( pmove->numphysent >= MAX_PHYSENTS )
	{
		Con_DPrintf( "CL_AddEntityToPhysList:  pmove->numphysent >= MAX_PHYSENTS\n" );
		return false;
	}

	auto pEnt = &cl_entities[ entIndex ];

	if( pEnt->curstate.modelindex )
	{
		CL_AddStateToEntlist( &pmove->physents[ pmove->numphysent ], &pEnt->curstate );
		++pmove->numphysent;
		return true;
	}

	return false;
}

void CL_MoveSpectatorCamera()
{
	if( cls.state == ca_active && cls.spectator )
	{
		CL_SetUpPlayerPrediction( false, true );
		CL_SetSolidPlayers( cl.playernum );

		double time = cl.time;

		CL_RunUsercmd( &spectatorState, &spectatorState, &cl.cmd, true, &time, static_cast<long long>( 100.0 * time ) );
			
		cl.simvel[ 0 ] = spectatorState.client.velocity[ 0 ];
		cl.simvel[ 1 ] = spectatorState.client.velocity[ 1 ];
		cl.simvel[ 2 ] = spectatorState.client.velocity[ 2 ];

		cl.simorg[ 0 ] = spectatorState.playerstate.origin[ 0 ];
		cl.simorg[ 1 ] = spectatorState.playerstate.origin[ 1 ];
		cl.simorg[ 2 ] = spectatorState.playerstate.origin[ 2 ];

		cl.punchangle[ 0 ] = spectatorState.client.punchangle[ 0 ];
		cl.punchangle[ 1 ] = spectatorState.client.punchangle[ 1 ];
		cl.punchangle[ 2 ] = spectatorState.client.punchangle[ 2 ];

		cl.viewheight[ 0 ] = spectatorState.client.view_ofs[ 0 ];
		cl.viewheight[ 1 ] = spectatorState.client.view_ofs[ 1 ];
		cl.viewheight[ 2 ] = spectatorState.client.view_ofs[ 2 ];
	}
}

void CL_SetDevOverView( refdef_t* refdef )
{
	gDevOverview.origin[ 0 ] -= cl.cmd.sidemove / 128.0;
	gDevOverview.origin[ 1 ] -= cl.cmd.forwardmove / 128.0;

	if( cl.cmd.upmove > 0.0 )
		gDevOverview.z_min += 1.0;

	if( cl.cmd.upmove < 0.0 )
		gDevOverview.z_min -= 1.0;

	if( cl.cmd.buttons & IN_DUCK )
		gDevOverview.z_max -= 1.0;

	if( cl.cmd.buttons & IN_JUMP )
		gDevOverview.z_max += 1.0;

	if( cl.cmd.buttons & IN_ATTACK )
		gDevOverview.zoom += 0.01;

	if( cl.cmd.buttons & IN_ATTACK2 )
		gDevOverview.zoom -= 0.01;

	refdef->vieworg[ 0 ] = gDevOverview.origin[ 0 ];
	refdef->vieworg[ 1 ] = gDevOverview.origin[ 1 ];
	refdef->vieworg[ 2 ] = 16000;

	refdef->onlyClientDraws = false;

	refdef->viewangles[ 0 ] = 90;
	refdef->viewangles[ 1 ] = gDevOverview.rotated ? 90 : 0;
	refdef->viewangles[ 2 ] = 0;

	if( dev_overview.value < 2 )
	{
		Con_Printf(
			" Overview: Zoom %.2f, Map Origin (%.2f, %.2f, %.2f), Z Min %.2f, Z Max %.2f, Rotated %i\n",
			gDevOverview.zoom,
			gDevOverview.origin[ 0 ],
			gDevOverview.origin[ 1 ],
			gDevOverview.origin[ 2 ],
			gDevOverview.z_min,
			gDevOverview.z_max,
			gDevOverview.rotated );
	}
}
