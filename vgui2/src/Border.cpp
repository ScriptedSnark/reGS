#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

#include "Border.h"
#include "KeyValues.h"
#include "VPanel.h"

namespace vgui2
{
Border::Border()
{
}

Border::~Border()
{
	if( _name )
		delete[] _name;

	for( auto& side : _sides )
	{
		if( side.lines )
			delete[] side.lines;
	}
}

void Border::Paint( VPANEL panel )
{
	int wide, tall;
	VHandleToPanel( panel )->GetSize( wide, tall );
	Paint( 0, 0, wide, tall, -1, 0, 0 );
}

void Border::Paint( int x0, int y0, int x1, int y1 )
{
	Paint( x0, y0, x1, y1, -1, 0, 0 );
}

void Border::Paint( int x0, int y0, int x1, int y1, int breakSide, int breakStart, int breakStop )
{
	{
		auto& side = _sides[ 0 ];

		int left = x0;

		for( int i = 0; i < side.count; ++i, ++left )
		{
			auto& line = side.lines[ i ];

			vgui2::surface()->DrawSetColor( line.col );

			if( breakSide )
			{
				vgui2::surface()->DrawFilledRect(
					left,
					line.startOffset + y0,
					left + 1,
					y1 - line.endOffset
				);
			}
			else
			{
				if( breakStart > 0 )
				{
					vgui2::surface()->DrawFilledRect(
						left,
						line.startOffset + y0,
						left + 1,
						breakStart + y0
					);
				}

				if( ( y1 - line.endOffset ) > breakStop )
				{
					vgui2::surface()->DrawFilledRect(
						left,
						y0 + breakStop + 1,
						left + 1,
						y1 - line.endOffset
					);
				}
			}
		}
	}

	{
		auto& side = _sides[ 1 ];

		int top = y0;

		for( int i = 0; i < side.count; ++i, ++top )
		{
			auto& line = side.lines[ i ];

			vgui2::surface()->DrawSetColor( line.col );

			if( breakSide == 1 )
			{
				if( breakStart > 0 )
				{
					vgui2::surface()->DrawFilledRect(
						line.startOffset + x0,
						top,
						breakStart + 1,
						top + 1
					);
				}

				if( ( x1 - line.endOffset ) > breakStop )
				{
					vgui2::surface()->DrawFilledRect(
						x0 + breakStop + 1,
						top,
						x1 - line.endOffset,
						top + 1
					);
				}
			}
			else
			{
				vgui2::surface()->DrawFilledRect(
					line.startOffset + x0,
					top,
					x1 - line.endOffset,
					top + 1
				);
			}
		}
	}

	{
		auto& side = _sides[ 2 ];

		int right = x1 - 1;

		for( int i = 0; i < side.count; ++i, --right )
		{
			auto& line = side.lines[ i ];

			vgui2::surface()->DrawSetColor( line.col );

			vgui2::surface()->DrawFilledRect(
				right,
				line.startOffset + y0,
				right + 1,
				y1 - line.endOffset
			);
		}
	}

	{
		auto& side = _sides[ 3 ];

		int bottom = y1 - 1;

		for( int i = 0; i < side.count; ++i, --bottom )
		{
			auto& line = side.lines[ i ];

			vgui2::surface()->DrawSetColor( line.col );

			vgui2::surface()->DrawFilledRect(
				line.startOffset + x0,
				bottom,
				x1 - line.endOffset,
				bottom + 1
			);
		}
	}
}

void Border::SetInset( int left, int top, int right, int bottom )
{
	_inset[ 0 ] = left;
	_inset[ 1 ] = top;
	_inset[ 2 ] = right;
	_inset[ 3 ] = bottom;
}

void Border::GetInset( int &left, int &top, int &right, int &bottom )
{
	left = _inset[ 0 ];
	top = _inset[ 1 ];
	right = _inset[ 2 ];
	bottom = _inset[ 3 ];
}

void Border::ApplySchemeSettings( IScheme *pScheme, KeyValues *inResourceData )
{
	auto insetString = inResourceData->GetString( "inset", "0 0 0 0" );

	int left, top, right, bottom;
	GetInset( left, top, right, bottom );

	sscanf( insetString, "%d %d %d %d", &left, &top, &right, &bottom );

	SetInset( left, top, right, bottom );

	ParseSideSettings( 0, inResourceData->FindKey( "Left" ), pScheme );
	ParseSideSettings( 1, inResourceData->FindKey( "Top" ), pScheme );
	ParseSideSettings( 2, inResourceData->FindKey( "Right" ), pScheme );
	ParseSideSettings( 3, inResourceData->FindKey( "Bottom" ), pScheme );
}

const char *Border::GetName()
{
	return _name;
}

void Border::SetName( const char *name )
{
	if( _name )
		delete[] _name;

	_name = new char[ strlen( name ) + 1 ];
	strcpy( _name, name );
}

void Border::ParseSideSettings( int side_index, KeyValues* inResourceData, IScheme* pScheme )
{
	if( !inResourceData )
		return;

	auto& side = _sides[ side_index ];

	int iCount = 0;

	for( auto pKV = inResourceData->GetFirstSubKey(); pKV; pKV = pKV->GetNextKey() )
	{
		++iCount;
	}

	side.count = iCount;
	side.lines = new line_t[ iCount ];

	for( int i = 0; i < iCount; ++i )
	{
		side.lines[ i ].col.SetColor( 0, 0, 0, 0 );
	}

	int index = 0;

	for( auto pKV = inResourceData->GetFirstSubKey(); pKV; pKV = pKV->GetNextKey(), ++index )
	{
		auto color = pScheme->GetColor( pKV->GetString( "color", nullptr ), SDK_Color( 0, 0, 0, 0 ) );

		side.lines[ index ].col = color;

		auto pszOffset = pKV->GetString( "offset", nullptr );

		int Start, End;

		if( pszOffset )
		{
			sscanf( pszOffset, "%d %d", &Start, &End );
		}
		else
		{
			Start = 0;
			End = 0;
		}

		side.lines[ index ].startOffset = Start;
		side.lines[ index ].endOffset = End;
	}
}
}
