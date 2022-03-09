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
	cl_entity_t* dest;
	TEMPENTITY* pTVar1;
	TEMPENTITY* pTVar2;

	pTVar2 = gpTempEntFree;
	if (gpTempEntFree == NULL) {
		Con_DPrintf("Overflow %d temporary ents!\n", 500);
	}
	else if (model == NULL) {
		pTVar2 = NULL;
		Con_DPrintf("efx.CL_TempEntAlloc: No model\n");
	}
	else {
		dest = &gpTempEntFree->entity;
		gpTempEntFree = gpTempEntFree->next;
		Q_memset(dest, 0, 3000);
		pTVar2->flags = FTENT_NONE;
		(pTVar2->entity).curstate.colormap = 0;
		pTVar2->die = cl.time + 0.75;
		(pTVar2->entity).model = model;
		(pTVar2->entity).curstate.rendermode = 0;
		(pTVar2->entity).curstate.renderfx = 0;
		pTVar2->fadeSpeed = 0.5;
		pTVar2->hitSound = 0;
		pTVar2->clientIndex = -1;
		pTVar2->bounceFactor = 1.0;
		pTVar2->hitcallback = NULL;
		pTVar2->callback = NULL;
		pTVar2->priority = 0;
		(pTVar2->entity).origin[0] = *org;
		(pTVar2->entity).origin[1] = org[1];
		(pTVar2->entity).origin[2] = org[2];
		pTVar1 = gpTempEntActive;
		gpTempEntActive = pTVar2;
		pTVar2->next = pTVar1;
	}
	return pTVar2;
}

TEMPENTITY* CL_TempEntAllocNoModel( vec_t* org )
{
	cl_entity_t* dest;
	TEMPENTITY* pTVar1;
	TEMPENTITY* pTVar2;

	pTVar1 = gpTempEntFree;
	if (gpTempEntFree == NULL) {
		Con_DPrintf("Overflow %d temporary ents!\n", 500);
	}
	else {
		dest = &gpTempEntFree->entity;
		gpTempEntFree = gpTempEntFree->next;
		Q_memset(dest, 0, 3000);
		pTVar1->flags = 0;
		(pTVar1->entity).curstate.colormap = 0;
		pTVar1->die = cl.time + 0.75;
		(pTVar1->entity).model = NULL;
		(pTVar1->entity).curstate.rendermode = 0;
		pTVar1->flags = FTENT_NOMODEL;
		(pTVar1->entity).curstate.renderfx = 0;
		pTVar1->fadeSpeed = 0.5;
		pTVar1->hitSound = 0;
		pTVar1->clientIndex = -1;
		pTVar1->bounceFactor = 1.0;
		pTVar1->hitcallback = NULL;
		pTVar1->callback = NULL;
		pTVar1->priority = 0;
		(pTVar1->entity).origin[0] = *org;
		(pTVar1->entity).origin[1] = org[1];
		(pTVar1->entity).origin[2] = org[2];
		pTVar2 = gpTempEntActive;
		gpTempEntActive = pTVar1;
		pTVar1->next = pTVar2;
	}
	return pTVar1;
}

TEMPENTITY* CL_TempEntAllocHigh( vec_t* org, model_t* model )
{
	tempent_s** pptVar1;
	TEMPENTITY* pTVar2;
	tempent_s* ptVar3;
	TEMPENTITY* pTVar4;

	pTVar2 = gpTempEntActive;
	pTVar4 = gpTempEntFree;
	if (model == NULL) {
		pTVar4 = NULL;
		Con_DPrintf("temporary ent model invalid\n");
	}
	else if (gpTempEntFree == NULL) {
		ptVar3 = gpTempEntActive;
		if (gpTempEntActive != NULL) {
			do {
				if (ptVar3->priority == 0) goto INIT_ENTITY;
				ptVar3 = ptVar3->next;
			} while (ptVar3 != NULL);
		}
		Con_DPrintf("Couldn't alloc a high priority TENT!\n");
	}
	else {
		gpTempEntActive = gpTempEntFree;
		pptVar1 = &gpTempEntFree->next;
		gpTempEntFree = gpTempEntFree->next;
		*pptVar1 = pTVar2;
		ptVar3 = pTVar4;
	INIT_ENTITY:
		Q_memset(&ptVar3->entity, 0, 3000);
		(ptVar3->entity).curstate.colormap = 0;
		ptVar3->flags = FTENT_NONE;
		ptVar3->die = cl.time + 0.75;
		(ptVar3->entity).curstate.rendermode = 0;
		(ptVar3->entity).model = model;
		(ptVar3->entity).curstate.renderfx = 0;
		ptVar3->fadeSpeed = 0.5;
		ptVar3->hitSound = 0;
		ptVar3->clientIndex = -1;
		ptVar3->bounceFactor = 1.0;
		ptVar3->hitcallback = NULL;
		ptVar3->callback = NULL;
		ptVar3->priority = 1;
		(ptVar3->entity).origin[0] = *org;
		(ptVar3->entity).origin[1] = org[1];
		(ptVar3->entity).origin[2] = org[2];
		pTVar4 = ptVar3;
	}
	return pTVar4;
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

void R_AttachTentToPlayer2(int client, model_s* pModel, float zoffset, float life)
{
	model_s* pmVar1;
	TEMPENTITY* pTVar2;
	int iVar3;
	vec3_t position;

	if ((-1 < client) && (client <= cl.maxclients)) {
		// TODO: impl - xWhitey
		if (true /*cl_entit.curstate.messagenum == cl.parsecount*/) { // There's prob should be CL_GetEntityByIndex, but its definition doesn't exist here
			//position[0] = cl_entities[client].origin[0];             } the same as             ^^^^^^^^^^^^
			//position[2] = zoffset + cl_entities[client].origin[2];   }
			//position[1] = cl_entities[client].origin[1];             }
			pTVar2 = (*efx.CL_TempEntAllocHigh)(position, pModel);
			if (pTVar2 != NULL) {
				(pTVar2->entity).curstate.rendermode = 0;
				pTVar2->tentOffset[2] = zoffset;
				(pTVar2->entity).curstate.renderfx = 0xe;
				(pTVar2->entity).curstate.renderamt = 0xff;
				(pTVar2->entity).baseline.renderamt = 0xff;
				(pTVar2->entity).curstate.framerate = 1.0;
				pTVar2->clientIndex = (short)client;
				pTVar2->tentOffset[0] = 0.0;
				pTVar2->tentOffset[1] = 0.0;
				pTVar2->die = life + (float)cl.time;
				pmVar1 = (pTVar2->entity).model;
				pTVar2->flags = pTVar2->flags | 0xa000;
				if (pmVar1->type == mod_sprite) {
					//iVar3 = ModelFrameCount(pModel); TODO: impl - xWhitey
					pTVar2->flags = pTVar2->flags | 0x10100; // unknown flag?? - xWhitey
					(pTVar2->entity).curstate.framerate = 10.0;
					pTVar2->frameMax = (float)iVar3;
				}
				else {
					pTVar2->frameMax = 0.0;
				}
				(pTVar2->entity).curstate.frame = 0.0;
				return;
			}
			Con_Printf("No temp ent.\n");
		}
		return;
	}
	Con_Printf("Bad client in R_AttachTentToPlayer()!\n");
	return;
}

void R_AttachTentToPlayer( int client, int modelIndex, float zoffset, float life )
{
	model_t* pModel;

	pModel = CL_GetModelByIndex(modelIndex); // TODO: impl - xWhitey
	if (pModel != (model_t*)0x0) {
		R_AttachTentToPlayer2(client, pModel, zoffset, life); // TODO: impl - xWhitey
		return;
	}
	Con_Printf("No model %d!\n");
	return;
}

void R_KillAttachedTents( int client )
{
	tempent_s* ptVar1;

	if ((client > -1) && (client <= cl.maxclients)) {
		ptVar1 = gpTempEntActive;
		if (gpTempEntActive != NULL) {
			do {
				while (((ptVar1->flags & FTENT_PLYRATTACHMENT) == 0 || (ptVar1->clientIndex != client))) {
					ptVar1 = ptVar1->next;
					if (ptVar1 == NULL) {
						return;
					}
				}
				ptVar1->die = cl.time;
				ptVar1 = ptVar1->next;
			} while (ptVar1 != NULL);
		}
		return;
	}
	Con_Printf("Bad client in R_KillAttachedTents()!\n");
	return;
}
