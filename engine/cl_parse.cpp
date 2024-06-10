#include "quakedef.h"
#include "client.h"
#include "cl_parse.h"

int CL_UPDATE_BACKUP = 1 << 6;
int CL_UPDATE_MASK = CL_UPDATE_BACKUP - 1;

void CL_ShutDownUsrMessages()
{
	//TODO: implement - Solokiller
}

pfnUserMsgHook HookServerMsg( const char* pszName, pfnUserMsgHook pfn )
{
	//TODO: implement - Solokiller
	return nullptr;
}

void CL_RemoveFromResourceList( resource_t* pResource )
{
	if( !pResource->pPrev || !pResource->pNext )
		Sys_Error( "Mislinked resource in CL_RemoveFromResourceList" );

	if( pResource == pResource->pNext || pResource->pPrev == pResource )
		Sys_Error( "Attempt to free last entry in list." );

	pResource->pPrev->pNext = pResource->pNext;
	pResource->pNext->pPrev = pResource->pPrev;
	pResource->pPrev = nullptr;
	pResource->pNext = nullptr;
}

void CL_ClearResourceLists()
{
	cl.downloadUrl[ 0 ] = '\0';

	for( resource_t* pResource = cl.resourcesneeded.pNext, * pNext;
		 pResource != &cl.resourcesneeded;
		 pResource = pNext )
	{
		pNext = pResource->pNext;

		CL_RemoveFromResourceList( pResource );
		Mem_Free( pResource );
	}

	cl.resourcesneeded.pNext = &cl.resourcesneeded;
	cl.resourcesneeded.pPrev = &cl.resourcesneeded;

	for( resource_t* pResource = cl.resourcesonhand.pNext, *pNext;
		 pResource != &cl.resourcesonhand;
		 pResource = pNext )
	{
		pNext = pResource->pNext;

		CL_RemoveFromResourceList( pResource );
		Mem_Free( pResource );
	}

	cl.resourcesonhand.pNext = &cl.resourcesonhand;
	cl.resourcesonhand.pPrev = &cl.resourcesonhand;
}
