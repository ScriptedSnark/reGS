#include "quakedef.h"
#include "client.h"
#include "cl_tent.h"

static TEMPENTITY gTempEnts[ MAX_TEMP_ENTITIES ];

static TEMPENTITY* gpTempEntActive = nullptr;
static TEMPENTITY* gpTempEntFree = nullptr;

void CL_TempEntInit()
{
	Q_memset( gTempEnts, 0, sizeof( gTempEnts ) );

	//Fix up pointers
	for( int i = 0;  i < ARRAYSIZE( gTempEnts ) - 1; ++i )
	{
		gTempEnts[ i ].next = &gTempEnts[ i + 1 ];
	}

	gTempEnts[ ARRAYSIZE( gTempEnts ) - 1 ].next = nullptr;

	gpTempEntFree = gTempEnts;
	gpTempEntActive = nullptr;
}

TEMPENTITY* CL_TempEntAlloc( vec_t* org, model_t* model )
{
	//TODO: implement - Solokiller
	return nullptr;
}

TEMPENTITY* CL_TempEntAllocNoModel( vec_t* org )
{
	//TODO: implement - Solokiller
	return nullptr;
}

TEMPENTITY* CL_TempEntAllocHigh( vec_t* org, model_t* model )
{
	//TODO: implement - Solokiller
	return nullptr;
}

TEMPENTITY* CL_AllocCustomTempEntity( float* origin, model_t* model, int high, void( *callback )( TEMPENTITY*, float, float ) )
{
	//TODO: implement - Solokiller
	return nullptr;
}

void R_BloodSprite( vec_t* org, int colorindex, int modelIndex, int modelIndex2, float size )
{
	//TODO: implement - Solokiller
}

void R_BreakModel( float* pos, float* size, float* dir, float random, float life, int count, int modelIndex, char flags )
{
	//TODO: implement - Solokiller
}

void R_Bubbles( vec_t* mins, vec_t* maxs, float height, int modelIndex, int count, float speed )
{
	//TODO: implement - Solokiller
}

void R_BubbleTrail( vec_t* start, vec_t* end, float height, int modelIndex, int count, float speed )
{
	//TODO: implement - Solokiller
}

void R_Explosion( float* pos, int model, float scale, float framerate, int flags )
{
	//TODO: implement - Solokiller
}

void R_FizzEffect( cl_entity_t* pent, int modelIndex, int density )
{
	//TODO: implement - Solokiller
}

void R_FireField( vec_t* org, int radius, int modelIndex, int count, int flags, float life )
{
	//TODO: implement - Solokiller
}

void R_FunnelSprite( float* org, int modelIndex, int reverse )
{
	//TODO: implement - Solokiller
}

void R_MultiGunshot( vec_t* org, vec_t* dir, vec_t* noise, int count, int decalCount, int* decalIndices )
{
	//TODO: implement - Solokiller
}

void R_MuzzleFlash( float* pos1, int type )
{
	//TODO: implement - Solokiller
}

void R_ParticleBox( float* mins, float* maxs, byte r, byte g, byte b, float life )
{
	//TODO: implement - Solokiller
}

void R_ParticleLine( float* start, float* end, byte r, byte g, byte b, float life )
{
	//TODO: implement - Solokiller
}

void R_PlayerSprites( int client, int modelIndex, int count, int size )
{
	//TODO: implement - Solokiller
}

void R_Projectile( vec_t* origin, vec_t* velocity, int modelIndex, int life, int owner, void( *hitcallback )( TEMPENTITY*, pmtrace_t* ) )
{
	//TODO: implement - Solokiller
}

void R_RicochetSound( vec_t* pos )
{
	//TODO: implement - Solokiller
}

void R_RicochetSprite( float* pos, model_t* pmodel, float duration, float scale )
{
	//TODO: implement - Solokiller
}

void R_RocketFlare( float* pos )
{
	//TODO: implement - Solokiller
}

void R_SparkEffect( float* pos, int count, int velocityMin, int velocityMax )
{
	//TODO: implement - Solokiller
}

void R_SparkShower( float* pos )
{
	//TODO: implement - Solokiller
}

void R_Spray( vec_t* pos, vec_t* dir, int modelIndex, int count, int speed, int spread, int rendermode )
{
	//TODO: implement - Solokiller
}

void R_Sprite_Explode( TEMPENTITY* pTemp, float scale, int flags )
{
	//TODO: implement - Solokiller
}

void R_Sprite_Smoke( TEMPENTITY* pTemp, float scale )
{
	//TODO: implement - Solokiller
}

void R_Sprite_Spray( vec_t* pos, vec_t* dir, int modelIndex, int count, int speed, int iRand )
{
	//TODO: implement - Solokiller
}

void R_Sprite_Trail( int type, vec_t* start, vec_t* end,
					 int modelIndex, int count, float life, float size,
					 float amplitude, int renderamt, float speed )
{
	//TODO: implement - Solokiller
}

void R_Sprite_WallPuff( TEMPENTITY* pTemp, float scale )
{
	//TODO: implement - Solokiller
}

void R_TracerEffect( vec_t* start, vec_t* end )
{
	//TODO: implement - Solokiller
}

void R_TempSphereModel( float* pos, float speed, float life, int count, int modelIndex )
{
	//TODO: implement - Solokiller
}

TEMPENTITY* R_TempModel( float* pos, float* dir, float* angles, float life, int modelIndex, int soundtype )
{
	//TODO: implement - Solokiller
	return nullptr;
}

TEMPENTITY* R_DefaultSprite( float* pos, int spriteIndex, float framerate )
{
	//TODO: implement - Solokiller
	return nullptr;
}

TEMPENTITY* R_TempSprite( float* pos, float* dir, float scale, int modelIndex, int rendermode, int renderfx, float a, float life, int flags )
{
	//TODO: implement - Solokiller
	return nullptr;
}

void R_AttachTentToPlayer( int client, int modelIndex, float zoffset, float life )
{
	//TODO: implement - Solokiller
}

void R_KillAttachedTents( int client )
{
	//TODO: implement - Solokiller
}
