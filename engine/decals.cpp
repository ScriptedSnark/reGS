#include "quakedef.h"
#include "decals.h"

bool m_bDrawInitialized = false;

void Draw_DecalShutdown()
{
	//TODO: implement - Solokiller
}

void Draw_Shutdown()
{

	if( m_bDrawInitialized )
	{
		m_bDrawInitialized = false;
		//TODO: implement - Solokiller
	}
}

void Decal_Init()
{
	//TODO: implement - Solokiller
}

int Draw_DecalIndex( int id )
{
	//TODO: implement - Solokiller
	return 0;
}

int Draw_DecalIndexFromName( char* name )
{
	//TODO: implement - Solokiller
	return 0;
}
