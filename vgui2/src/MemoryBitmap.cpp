#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

#include "MemoryBitmap.h"

namespace vgui2
{
MemoryBitmap::MemoryBitmap( byte* texture, int wide, int tall )
	: _texture( texture )
	, _w( wide )
	, _h( tall )
{
	if( tall )
		ForceUpload();
}

MemoryBitmap::~MemoryBitmap()
{
}

void MemoryBitmap::Paint()
{
	if( !_valid )
		return;

	if( NULL_HANDLE == _id )
	{
		ForceUpload();
	}

	if( !_uploaded )
		return;

	surface()->DrawSetTexture( _id );

	surface()->DrawSetColor(
		_color[ 0 ],
		_color[ 1 ],
		_color[ 2 ],
		_color[ 3 ]
	);

	int wide, tall;
	surface()->DrawGetTextureSize( _id, wide, tall );

	surface()->DrawTexturedRect(
		_pos[ 0 ], _pos[ 1 ],
		_pos[ 0 ] + wide,
		_pos[ 1 ] + tall
	);
}

void MemoryBitmap::SetPos( int x, int y )
{
	_pos[ 0 ] = x;
	_pos[ 1 ] = y;
}

void MemoryBitmap::GetContentSize( int &wide, int &tall )
{
	GetSize( wide, tall );
}

void MemoryBitmap::GetSize( int &wide, int &tall )
{
	wide = 0;
	tall = 0;

	if( _valid )
		surface()->DrawGetTextureSize( _id, wide, tall );
}

void MemoryBitmap::SetSize( int wide, int tall )
{
	//Nothing
}

void MemoryBitmap::SetColor( SDK_Color col )
{
	_color = col;
}

void MemoryBitmap::ForceUpload()
{
	if( NULL_HANDLE == _id )
		_id = surface()->CreateNewTextureID();

	surface()->DrawSetTextureRGBA( _id, _texture, _w, _h, false, true );

	_uploaded = true;

	_valid = surface()->IsTextureIDValid( _id );
}
}
