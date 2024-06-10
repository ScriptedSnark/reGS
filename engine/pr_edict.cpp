#include "quakedef.h"
#include "pr_edict.h"
#include "progs.h"

void ReleaseEntityDLLFields( edict_t* pEdict )
{
	if( pEdict->pvPrivateData )
	{
		if( gNewDLLFunctions.pfnOnFreeEntPrivateData )
		{
			gNewDLLFunctions.pfnOnFreeEntPrivateData( pEdict );
		}

		Mem_Free( pEdict->pvPrivateData );
	}

	pEdict->pvPrivateData = nullptr;
}
