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
#ifndef ENGINE_QUAKEDEF_H
#define ENGINE_QUAKEDEF_H
//For backwards compatibility only, SDK headers use it - Solokiller
#define QUAKEDEF_H

/**
*	@file
*
*	primary header for client
*/

#include <cmath>

#define MAX_NUM_ARGVS	50

#if defined(_M_IX86)
#define __i386__	1
#endif

#if defined __i386__ // && !defined __sun__
#define id386	1
#else
#define id386	0
#endif

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2

#define	MAX_LIGHTSTYLES	64
#define	MAX_MODELS		512			// these are sent over the net as bytes
#define	MAX_SOUNDS		512			// so they cannot be blindly increased
#define	MAX_STYLESTRING	64

//TODO: tidy these includes - Solokiller
#include "tier0/platform.h"
#include "commonmacros.h"
#include "filesystem.h"
#include "cvar.h"
#include "cmd.h"
#include "mem.h"
#include "zone.h"
#include "common.h"
#include "net.h"
#include "mathlib.h"
#include "const.h"
#include "console.h"
#include "sv_main.h"
#include "sys.h"
#include "strtools.h"
#include "protocol.h"
#include "sv_log.h"
#include "com_model.h"
#include "info.h"
#include "sv_steam3.h"
#include "keys.h"

/**
*	the host system specifies the base of the directory tree, the
*	command line parms passed to the program, and the amount of memory
*	available for the program to use
*/
struct quakeparms_t
{
	const char* basedir;
	const char* cachedir;			// for development over ISDN lines
	int argc;
	const char** argv;
	void *membase;
	int memsize;
};

extern quakeparms_t host_parms;

/**
*	true if into command execution
*/
extern bool host_initialized;

/**
*	not bounded in any way, changed at
*	start of every frame, never reset
*/
extern double realtime;

void Host_Error( const char* error, ... );

bool Host_Init( quakeparms_t* parms );
void Host_Shutdown();

/**
*	Loads the server dll if needed.
*/
void Host_InitializeGameDLL();

void Host_ClearSaveDirectory();

#endif //ENGINE_QUAKEDEF_H
