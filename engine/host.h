#ifndef ENGINE_HOST_H
#define ENGINE_HOST_H

#include <csetjmp>

#include <tier0/platform.h>

#include "server.h"

extern jmp_buf host_abortserver;

extern cvar_t developer;

extern double host_frametime;

extern client_t* host_client;

int Host_Frame( float time, int iState, int* stateInfo );

bool Host_IsServerActive();

bool Host_IsSinglePlayerGame();

void Host_GetHostInfo( float* fps, int* nActive, int* unused, int* nMaxPlayers, char* pszMap );

void SV_DropClient( client_t* cl, qboolean crash, const char* fmt, ... );

void SV_ClearResourceLists( client_t* cl );

void Host_CheckDyanmicStructures();

void SV_ClearClientStates();

void Host_ClearMemory( bool bQuiet );

#endif //ENGINE_HOST_H
