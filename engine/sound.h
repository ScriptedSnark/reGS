#ifndef ENGINE_SND_H
#define ENGINE_SND_H

typedef struct sfx_s
{
	char 	name[ MAX_QPATH ];
	cache_user_t	cache;
	int servercount;
} sfx_t;

extern cvar_t suitvolume;

extern bool g_fUseDInput;

void S_Init();

void S_Shutdown();

sfx_t* S_PrecacheSound( const char* name );

void S_StartDynamicSound( int entnum, int entchannel, sfx_t* sfx, vec3_t origin, float fvol, float attenuation, int flags, int pitch );

void S_StopSound( int entnum, int entchannel );

void S_StopAllSounds( bool clear );

void Snd_AcquireBuffer();
void Snd_ReleaseBuffer();

void SetMouseEnable( int fState );

void VOX_Init();

#endif //ENGINE_SND_H
