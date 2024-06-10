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
#ifndef ENGINE_NET_H
#define ENGINE_NET_H

#include <cstdlib>

#include "tier0/platform.h"
#include "netadr.h"

/**
*	Maximum size for a fragment buffer.
*/
//TODO: unrelated to actual net constants, remove - Solokiller
const size_t NET_MAX_FRAG_BUFFER = 1400;

#define	MAX_MSGLEN		4000		// max length of a reliable message
#define	MAX_DATAGRAM	4000		// max length of unreliable message

extern sizebuf_t net_message;

void NET_Config( bool multiplayer );

void NET_Shutdown();

void NET_Init();

#endif //ENGINE_NET_H
