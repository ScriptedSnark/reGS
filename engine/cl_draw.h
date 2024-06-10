#ifndef ENGINE_CL_DRAW_H
#define ENGINE_CL_DRAW_H

struct SPRITELIST
{
	model_t* pSprite;
	char* pName;
	int frameCount;
};

void SetCrosshair( HSPRITE hspr, wrect_t rc, int r, int g, int b );

void DrawCrosshair( int x, int y );

void SPR_Init();

void SPR_Shutdown();

void SPR_Shutdown_NoModelFree();

HSPRITE SPR_Load( const char* pTextureName );

int SPR_Frames( HSPRITE hSprite );

int SPR_Height( HSPRITE hSprite, int frame );

int SPR_Width( HSPRITE hSprite, int frame );

void SPR_Set( HSPRITE hSprite, int r, int g, int b );

void SPR_EnableScissor( int x, int y, int width, int height );

void SPR_DisableScissor();

void SPR_Draw( int frame, int x, int y, const wrect_t* prc );

void SPR_DrawHoles( int frame, int x, int y, const wrect_t* prc );

void SPR_DrawAdditive( int frame, int x, int y, const wrect_t* prc );

void SPR_DrawGeneric( int frame, int x, int y, const wrect_t* prc, int src, int dest, int width, int height );

client_sprite_t* SPR_GetList( char* psz, int* piCount );

SPRITELIST* SPR_Get( HSPRITE hSprite );

model_t* SPR_GetModelPointer( HSPRITE hSprite );

void SetFilterMode( int mode );

void SetFilterColor( float r, float g, float b );

void SetFilterBrightness( float brightness );

#endif //ENGINE_CL_DRAW_H
