#include "quakedef.h"
#include "hashpak.h"
#include "host.h"
#include "sv_upld.h"

void SV_MoveToOnHandList( resource_t* pResource )
{
	if( pResource )
	{
		SV_RemoveFromResourceList( pResource );
		SV_AddToResourceList( pResource, &host_client->resourcesonhand );
	}
	else
	{
		Con_DPrintf( "Null resource passed to SV_MoveToOnHandList\n" );
	}
}

void SV_AddToResourceList( resource_t* pResource, resource_t* pList )
{
	if( pResource->pPrev || pResource->pNext )
	{
		Con_Printf( "Resource already linked\n" );
	}
	else
	{
		pResource->pPrev = pList->pPrev;
		pList->pPrev->pNext = pResource;
		pList->pPrev = pResource;
		pResource->pNext = pList;
	}
}

void SV_RemoveFromResourceList( resource_t* pResource )
{
	pResource->pPrev->pNext = pResource->pNext;
	pResource->pNext->pPrev = pResource->pPrev;
	pResource->pPrev = nullptr;
	pResource->pNext = nullptr;
}

void SV_ClearResourceList( resource_t* pList )
{
	for( resource_t* pResource = pList->pNext, *pNext;
		 pResource != pList;
		 pResource = pNext )
	{
		pNext = pResource->pNext;
		SV_RemoveFromResourceList( pResource );

		Mem_Free( pResource );
	}

	pList->pPrev = pList;
	pList->pNext = pList;
}

void SV_ClearResourceLists( client_t* cl )
{
	if( !cl )
		Sys_Error( "SV_ClearResourceLists with NULL client!" );

	SV_ClearResourceList( &cl->resourcesneeded );
	SV_ClearResourceList( &cl->resourcesonhand );
}

int SV_EstimateNeededResources()
{
	int iTotalSize = 0;

	for( auto i = host_client->resourcesneeded.pNext; i != &host_client->resourcesneeded; i = i->pNext )
	{
		if( i->type == t_decal )
		{
			if( !HPAK_ResourceForHash( "custom.hpk", i->rgucMD5_hash, nullptr ) )
			{
				if( i->nDownloadSize )
				{
					i->ucFlags |= RES_WASMISSING;
					iTotalSize += i->nDownloadSize;
				}
			}
		}
	}

	return iTotalSize;
}

void SV_CreateCustomizationList( client_t* pHost )
{
	int nLumps;
	customization_t* pCust;
	bool bIgnoreDup;

	for( auto pResource = pHost->resourcesonhand.pNext; pResource != &pHost->resourcesonhand; )
	{
		nLumps = 0;

		if( COM_CreateCustomization( &pHost->customdata, pResource, -1, RES_FATALIFMISSING | RES_WASMISSING, &pCust, &nLumps ) )
		{
			pCust->nUserData2 = nLumps;
			gEntityInterface.pfnPlayerCustomization( pHost->edict, pCust );
		}
		else
		{
			if( !sv_allow_upload.value )
			{
				Con_Printf( "Ignoring custom decal from %s\n", pHost->name );
			}
			else
			{
				Con_Printf( "Ignoring invalid custom decal from %s\n", pHost->name );
			}
		}

		pResource = pResource->pNext;

		bIgnoreDup = false;

		//Check if the next resource is a duplicate
		while( pResource )
		{
			for( auto pCustom = pHost->customdata.pNext;
				 pCustom;
				 pCustom = pCustom->pNext )
			{
				if( !Q_memcmp( pCustom->resource.rgucMD5_hash, pResource->rgucMD5_hash, sizeof( pResource->rgucMD5_hash ) ) )
				{
					Con_DPrintf( "SV_CreateCustomization list, ignoring dup. resource for player %s\n", pHost->name );
					bIgnoreDup = true;
					break;
				}
			}

			if( !bIgnoreDup )
				break;

			pResource = pResource->pNext;
		}
	}
}

void SV_RegisterResources()
{
	host_client->uploading = false;

	for( auto pResource = host_client->resourcesonhand.pNext;
		 pResource != &host_client->resourcesonhand;
		 pResource = pResource->pNext )
	{
		SV_CreateCustomizationList( host_client );
		SV_Customization( host_client, pResource, true );
	}
}

bool SV_UploadComplete( client_t* cl )
{
	if( cl->resourcesneeded.pNext == &cl->resourcesneeded )
	{
		SV_RegisterResources();

		SV_PropagateCustomizations();

		if( sv_allow_upload.value )
			Con_DPrintf( "Custom resource propagation complete.\n" );

		cl->uploaddoneregistering = true;

		return true;
	}

	return false;
}

bool SV_RequestMissingResources()
{
	if( host_client->uploading
		&& !host_client->uploaddoneregistering )
	{
		SV_UploadComplete( host_client );
	}

	return false;
}

void SV_RequestMissingResourcesFromClients()
{
	host_client = svs.clients;
	
	for( int i = 0; i < svs.maxclients; ++i, ++host_client )
	{
		SV_RequestMissingResources();
	}
}

void SV_Customization( client_t* pPlayer, resource_t* pResource, bool bSkipPlayer )
{
	int i;

	host_client = svs.clients;

	for( i = 0; i < svs.maxclients; ++i, ++host_client )
	{
		if( host_client == pPlayer )
			break;
	}

	if( i == svs.maxclients )
		Sys_Error( "Couldn't find player index for customization." );

	for( int cl = 0; cl < svs.maxclients; ++cl )
	{
		if( ( host_client->active || host_client->spawned ) &&
			!host_client->fakeclient &&
			( pPlayer != host_client || !bSkipPlayer ) )
		{
			//TODO: implement - Solokiller
			/*
			MSG_WriteByte( &host_client->netchan.message, 46 );
			MSG_WriteByte( &host_client->netchan.message, i );
			MSG_WriteByte( &host_client->netchan.message, pResource->type );
			MSG_WriteString( &host_client->netchan.message, pResource->szFileName );
			MSG_WriteShort( &host_client->netchan.message, pResource->nIndex );
			MSG_WriteLong( &host_client->netchan.message, pResource->nDownloadSize );
			MSG_WriteByte( &host_client->netchan.message, pResource->ucFlags );

			if( pResource->ucFlags & RES_CUSTOM )
				SZ_Write( &host_client->netchan.message, pResource->rgucMD5_hash, 16 );
				*/
		}
	}
}
