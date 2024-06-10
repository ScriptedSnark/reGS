#ifndef ENGINE_SV_UPLD_H
#define ENGINE_SV_UPLD_H

#include "custom.h"
#include "server.h"

void SV_MoveToOnHandList( resource_t* pResource );

void SV_AddToResourceList( resource_t* pResource, resource_t* pList );

void SV_RemoveFromResourceList( resource_t* pResource );

void SV_ClearResourceList( resource_t* pList );

void SV_ClearResourceLists( client_t* cl );

int SV_EstimateNeededResources();

void SV_CreateCustomizationList( client_t* pHost );

void SV_RegisterResources();

bool SV_UploadComplete( client_t* cl );

bool SV_RequestMissingResources();

void SV_RequestMissingResourcesFromClients();

void SV_Customization( client_t* pPlayer, resource_t* pResource, bool bSkipPlayer );

#endif //ENGINE_SV_UPLD_H
