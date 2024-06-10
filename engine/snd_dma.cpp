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
/**
*	@file
*
*	main control for any streaming sound output device
*/

#include "quakedef.h"
#include "sound.h"

cvar_t suitvolume = { "suitvolume", "0.25", FCVAR_ARCHIVE };

void S_Init()
{
	//TODO: implement - Solokiller
}

void S_Shutdown()
{
	//TODO: implement - Solokiller
}

sfx_t* S_PrecacheSound( const char* name )
{
	//TODO: implement - Solokiller
	return nullptr;
}

void S_StartDynamicSound( int entnum, int entchannel, sfx_t* sfx, vec3_t origin, float fvol, float attenuation, int flags, int pitch )
{
	//TODO: implement - Solokiller
}

void S_StopSound( int entnum, int entchannel )
{
	//TODO: implement - Solokiller
}

void S_StopAllSounds( bool clear )
{
	//TODO: implement  - Solokiller
}
