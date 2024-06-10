#ifndef ENGINE_EVENTAPI_H
#define ENGINE_EVENTAPI_H

#include "event_api.h"
#include "pm_defs.h"
#include "pmtrace.h"

const int EVENT_MAX_EVENTS = 256;

struct event_t
{
	unsigned short index;
	const char* filename;
	int filesize;
	const char* pszScript;
};

void EV_PlaySound( int ent, float* origin, int channel, const char* sample,
				   float volume, float attenuation, int fFlags, int pitch );

void EV_StopSound( int ent, int channel, const char* sample );

int EV_FindModelIndex( const char* pmodel );

int EV_IsLocal( int playernum );

int EV_LocalPlayerDucking();

void EV_LocalPlayerViewheight( float* viewheight );

void EV_LocalPlayerBounds( int hull, float* mins, float* maxs );

int EV_IndexFromTrace( pmtrace_t* pTrace );

physent_t* EV_GetPhysent( int idx );

void EV_SetUpPlayerPrediction( int dopred, int bIncludeLocalClient );

void EV_SetTraceHull( int hull );

void EV_PlayerTrace( float* start, float* end, int traceFlags, int ignore_pe, pmtrace_t* tr );

void EV_WeaponAnimation( int sequence, int body );

unsigned short EV_PrecacheEvent( int type, const char* psz );

void EV_PlaybackEvent( int flags, const edict_t* pInvoker, unsigned short eventindex, float delay, float* origin, float* angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );

void EV_KillEvents( int entnum, const char* eventname );

#endif //ENGINE_EVENTAPI_H
