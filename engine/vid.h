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
#ifndef ENGINE_VID_H
#define ENGINE_VID_H

/**
*	@file
*
*	Video driver defs
*/

#include "tier0/platform.h"

// a pixel can be one, two, or four bytes
typedef byte pixel_t;

struct viddef_t
{
	pixel_t			*buffer;		// invisible buffer
	pixel_t			*colormap;		// 256 * VID_GRADES size
	unsigned short	*colormap16;	// 256 * VID_GRADES size
	int				fullbright;		// index of first fullbright color
	unsigned		rowbytes;	// may be > width if displayed in a window
	unsigned		width;
	unsigned		height;
	float			aspect;		// width / height -- < 0 is taller than wide
	int				numpages;
	int				recalc_refdef;	// if true, recalc vid-based stuff
	pixel_t			*conbuffer;
	int				conrowbytes;
	unsigned		conwidth;
	unsigned		conheight;
	int				maxwarpwidth;
	int				maxwarpheight;
	pixel_t			*direct;		// direct drawing to framebuffer, if not
									//  NULL
};

extern	viddef_t	vid;				// global video state

#endif //ENGINE_VID_H
