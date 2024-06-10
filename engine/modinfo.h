#ifndef ENGINE_MODINFO_H
#define ENGINE_MODINFO_H

enum MOD_GAMEPLAY_TYPE_E
{
	BOTH = 0,
	SINGLEPLAYER_ONLY,
	MULTIPLAYER_ONLY
};

struct modinfo_t
{
	bool bIsMod;

	char szInfo[ 256 ];
	char szDL[ 256 ];
	char szHLVersion[ 32 ];
	int version;
	int size;

	bool svonly;
	bool cldll;
	bool secure;

	MOD_GAMEPLAY_TYPE_E type;

	int num_edicts;
};

extern modinfo_t gmodinfo;

void SV_ResetModInfo();

void DLL_SetModKey( modinfo_t *pinfo, char *pkey, char *pvalue );

#endif //ENGINE_MODINFO_H
