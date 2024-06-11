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
*	model loading and caching
*
*	models are the only shared resource between a client and server running
*	on the same machine.
*/
#include "quakedef.h"
#include "qgl.h"
#include "gl_mesh.h"
#include "gl_model.h"

/*
==============================================================================

ALIAS MODELS

==============================================================================
*/

aliashdr_t	*pheader;

stvert_t	stverts[ MAXALIASVERTS ];
mtriangle_t	triangles[ MAXALIASTRIS ];

// a pose is a single set of vertexes.  a frame may be
// an animating sequence of poses
trivertx_t	*poseverts[ MAXALIASFRAMES ];
int			posenum;

char* wadpath = nullptr;

void Mod_ClearAll()
{
	//TODO: implement - Solokiller
}

byte* Mod_DecompressVis(byte* in, model_t* model)
{
	// #define MAX_MAP_LEAFS 32767
	// i just copypasted this thing from quake - xWhitey
	static byte	decompressed[32767 / 8];
	int		c;
	byte* out;
	int		row;

	row = (model->numleafs + 7) >> 3;
	out = decompressed;

#if 0
	memcpy(out, in, row);
#else
	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}

		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);
#endif

	return decompressed;
}

void Mod_UnloadSpriteTextures(model_t* pModel)
{
	void* pvVar1;
	int param4;
	int iVar2;
	char name[256];

	if (pModel->type == mod_sprite) {
		pvVar1 = (pModel->cache).data;
		pModel->needload = true;
		if ((pvVar1 != (void*)0x0) && (0 < *(int*)((int)pvVar1 + 0xc))) {
			param4 = 0;
			do {
				iVar2 = param4 + 1;
				snprintf(name, 0x100, "%s_%i", pModel->name, param4);
				// TODO: impl - xWhitey
				//GL_UnloadTexture(name);
				param4 = iVar2;
			} while (*(int*)((int)pvVar1 + 0xc) != iVar2 && iVar2 <= *(int*)((int)pvVar1 + 0xc));
		}
	}
	return;
}