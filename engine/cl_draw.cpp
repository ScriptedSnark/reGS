#include "quakedef.h"
#include "client.h"
#include "cl_draw.h"
#include "gl_model.h"

#define SPR_MAX_SPRITES 256

HSPRITE ghCrosshair = 0;
wrect_t gCrosshairRc = {};
int gCrosshairR = 0;
int gCrosshairG = 0;
int gCrosshairB = 0;

static msprite_t* gpSprite = nullptr;

static int gSpriteCount = 0;
static SPRITELIST* gSpriteList = nullptr;

void SetCrosshair( HSPRITE hspr, wrect_t rc, int r, int g, int b )
{
	g_engdstAddrs.pfnSetCrosshair( &hspr, &rc, &r, &g, &b );

	ghCrosshair = hspr;
	gCrosshairRc.left = rc.left;
	gCrosshairRc.right = rc.right;
	gCrosshairRc.top = rc.top;
	gCrosshairRc.bottom = rc.bottom;
	gCrosshairR = r;
	gCrosshairG = g;
	gCrosshairB = b;
}

void DrawCrosshair( int x, int y )
{
	if( ghCrosshair )
	{
		SPR_Set( ghCrosshair, gCrosshairR, gCrosshairG, gCrosshairB );

		SPR_DrawHoles(
			0,
			x - ( gCrosshairRc.right - gCrosshairRc.left ) / 2,
			y - ( gCrosshairRc.bottom - gCrosshairRc.top ) / 2,
			&gCrosshairRc
		);
	}
}

void SPR_Init()
{
	if( !gSpriteList )
	{
		ghCrosshair = 0;
		gSpriteCount = SPR_MAX_SPRITES;
		gSpriteList = reinterpret_cast<SPRITELIST*>( Mem_ZeroMalloc( SPR_MAX_SPRITES * sizeof( SPRITELIST ) ) );
		gpSprite = nullptr;
	}
}

void SPR_Shutdown()
{
	//TODO: implement - Solokiller
	gpSprite = nullptr;
	gSpriteList = nullptr;
	gSpriteCount = 0;
	ghCrosshair = 0;
}

void SPR_Shutdown_NoModelFree()
{
	//TODO: implement - Solokiller
	gpSprite = nullptr;
	gSpriteList = nullptr;
	gSpriteCount = 0;
	ghCrosshair = 0;
}

HSPRITE SPR_Load( const char* pTextureName )
{
	//TODO: implement - Solokiller
	return 0;
}

int SPR_Frames( HSPRITE hSprite )
{
	//TODO: implement - Solokiller
	return 0;
}

int SPR_Height( HSPRITE hSprite, int frame )
{
	//TODO: implement - Solokiller
	return 0;
}

int SPR_Width( HSPRITE hSprite, int frame )
{
	//TODO: implement - Solokiller
	return 0;
}

void SPR_Set( HSPRITE hSprite, int r, int g, int b )
{
	//TODO: implement - Solokiller
}

void SPR_EnableScissor( int x, int y, int width, int height )
{
	//TODO: implement - Solokiller
}

void SPR_DisableScissor()
{
	//TODO: implement - Solokiller
}

void SPR_Draw( int frame, int x, int y, const wrect_t* prc )
{
	//TODO: implement - Solokiller
}

void SPR_DrawHoles( int frame, int x, int y, const wrect_t* prc )
{
	//TODO: implement - Solokiller
}

void SPR_DrawAdditive( int frame, int x, int y, const wrect_t* prc )
{
	//TODO: implement - Solokiller
}

void SPR_DrawGeneric( int frame, int x, int y, const wrect_t* prc, int src, int dest, int width, int height )
{
	//TODO: implement - Solokiller
}

client_sprite_t* SPR_GetList( char* psz, int* piCount )
{
	g_engdstAddrs.pfnSPR_GetList( &psz, &piCount );

	auto pszData = reinterpret_cast<char*>( COM_LoadTempFile( psz, nullptr ) );

	if( pszData )
	{
		pszData = COM_Parse( pszData );
		const auto iNumSprites = atoi( com_token );

		if( iNumSprites )
		{
			auto pList = reinterpret_cast<client_sprite_t*>( calloc( sizeof( client_sprite_t ) * iNumSprites, 1 ) );

			if( pList )
			{
				auto pSprite = pList;

				for( int i = 0; i < iNumSprites; ++i, ++pSprite )
				{
					pszData = COM_Parse( pszData );
					Q_strncpy( pSprite->szName, com_token, ARRAYSIZE( pSprite->szName ) );

					pszData = COM_Parse( pszData );
					pSprite->iRes = atoi( com_token );

					pszData = COM_Parse( pszData );
					Q_strncpy( pSprite->szSprite, com_token, ARRAYSIZE( pSprite->szSprite ) );

					pszData = COM_Parse( pszData );
					pSprite->rc.left = atoi( com_token );

					pszData = COM_Parse( pszData );
					pSprite->rc.top = atoi( com_token );

					pszData = COM_Parse( pszData );
					pSprite->rc.right = pSprite->rc.left + atoi( com_token );

					pszData = COM_Parse( pszData );
					pSprite->rc.bottom = pSprite->rc.top + atoi( com_token );
				}

				if( piCount )
					*piCount = iNumSprites;
			}

			return pList;
		}
	}

	return nullptr;
}

SPRITELIST* SPR_Get( HSPRITE hSprite )
{
	const int iIndex = hSprite - 1;

	if( iIndex >= 0 && iIndex < gSpriteCount )
		return &gSpriteList[ iIndex ];

	return nullptr;
}

model_t* SPR_GetModelPointer( HSPRITE hSprite )
{
	auto pList = SPR_Get( hSprite );

	if( pList )
		return pList->pSprite;

	return nullptr;
}

void SetFilterMode( int mode )
{
	//TODO: implement - Solokiller
}

void SetFilterColor( float r, float g, float b )
{
	//TODO: implement - Solokiller
}

void SetFilterBrightness( float brightness )
{
	//TODO: implement - Solokiller
}
