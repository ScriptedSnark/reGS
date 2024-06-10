#include <tier0/platform.h>

#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

#include "Bitmap.h"

namespace vgui2
{
Bitmap::Bitmap( const char* filename, bool hardwareFiltered )
	: _filtered( hardwareFiltered )
{
	_filename = reinterpret_cast<char*>( malloc( strlen( filename ) + 1 ) );
	strcpy( _filename, filename );

	ForceUpload();
}

Bitmap::~Bitmap()
{
	if( _filename )
		free( _filename );
}

void Bitmap::Paint()
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

	surface()->DrawTexturedRect(
		_pos[ 0 ], _pos[ 1 ],
		_pos[ 0 ] + wide,
		 _pos[ 1 ] + tall
	);
}

void Bitmap::SetPos( int x, int y )
{
	_pos[ 0 ] = x;
	_pos[ 1 ] = y;
}

void Bitmap::GetContentSize( int &wide, int &tall )
{
	GetSize( wide, tall );
}

void Bitmap::GetSize( int &wide, int &tall )
{
	wide = 0;
	tall = 0;

	if( _valid )
	{
		surface()->DrawGetTextureSize( _id, wide, tall );
	}
}

void Bitmap::SetSize( int wide, int tall )
{
	this->wide = wide;
	this->tall = tall;
}

void Bitmap::SetColor( SDK_Color col )
{
	_color = col;
}

void Bitmap::ForceUpload()
{
	if( !_valid || _uploaded )
	{
		return;
	}

	if( NULL_HANDLE == _id )
		_id = surface()->CreateNewTextureID();

	surface()->DrawSetTextureFile(
		_id, _filename, _filtered, false
	);

	_uploaded = true;
	_valid = surface()->IsTextureIDValid( _id );

	GetSize( wide, tall );
}
}
