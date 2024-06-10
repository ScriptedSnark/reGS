#include "quakedef.h"
#include "cdll_int.h"
#include "cdll_exp.h"
#include "host.h"
#include "interface.h"
#include "LoadBlob.h"
#include "module.h"
#include "net_chan.h"
#include "server.h"

//TODO: security modules are obsolete and no longer used. Remove all of this - Solokiller

using PFunc = void ( * )( void* );

cl_enginefunc_dst_t g_engdstAddrs = k_engdstNull

modfuncs_t g_modfuncs = {};
module_t g_module = {};

BlobFootprint_t g_blobfootprintModule = {};

modshelpers_t g_modshelpers =
{
	&SV_KickPlayer
};

void* g_pvModuleSpecial = nullptr;

static engdata_t g_engdata = 
{
	nullptr,
	&g_engdstAddrs,
	nullptr,
	nullptr,
	&g_modfuncs,
	Cmd_GetFunctions(),
	nullptr,
	hudCon_Printf,
	&cvar_vars,
	nullptr,
	&SZ_GetSpace,
	&g_modfuncs,
	&Sys_GetProcAddress,
	&GetModuleHandle,
	&svs,
	nullptr,
	//Cast away the const correctness on the fmt parameter
	( decltype( engdata_t::pfnSV_DropClient ) )( &SV_DropClient ),
	&Netchan_Transmit,
	&NET_SendPacket,
	&Cvar_FindVar,
	nullptr,
	
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

//Dummy function used for C interfaces
void NullDst()
{
}

void* GetModuleHandle( char* name )
{
	if( name )
	{
		auto pModule = Sys_LoadModule( name );

		if( pModule )
			Sys_UnloadModule( pModule );

		return pModule;
	}

	return nullptr;
}

void CommonLoadSecurityModule( void* nAddrModule )
{
	if( g_modfuncs.m_nVersion == k_nModuleVersionCur )
	{
		if( g_modfuncs.m_pfnSetEngineVersion )
			g_modfuncs.m_pfnSetEngineVersion( k_nEngineVersion16Validated );

		g_modfuncs.m_pfnLoadMod( reinterpret_cast<char*>( nAddrModule ) );
		g_modfuncs.m_pfnGetEngDstAddrs( &g_engdstAddrs );

		if( g_modfuncs.m_pfnSetModSHelpers )
			g_modfuncs.m_pfnSetModSHelpers( &g_modshelpers );

		engdata_t engdata;

		memcpy( &engdata, &g_engdata, sizeof( engdata ) );

		g_modfuncs.m_pfnSetEngData( &engdata );

		g_modfuncs.m_pfnModuleLoaded();

		g_module.fLoaded = true;
	}
	else
	{
		CloseSecurityModule();
	}
}

void LoadSecurityModuleFromDisk( char* pszDllName )
{
	g_modfuncs.m_nVersion = k_nModuleVersionCur;
	NLoadBlobFile( pszDllName, &g_blobfootprintModule, &g_modfuncs, true );

	if( g_blobfootprintModule.m_hDll )
	{
		g_pvModuleSpecial = malloc( MODULE_MAX_SIZE );

		auto pPFunc = reinterpret_cast<PFunc>( Sys_GetProcAddress( reinterpret_cast<void*>( g_blobfootprintModule.m_hDll ), "P" ) );
		pPFunc( g_pvModuleSpecial );

		CommonLoadSecurityModule( g_pvModuleSpecial );
	}
}

void CloseSecurityModule()
{
	if( g_module.fLoaded )
	{
		if( g_modfuncs.m_pfnCloseMod )
			g_modfuncs.m_pfnCloseMod();

		FreeBlob( &g_blobfootprintModule );

		if( g_pvModuleSpecial )
		{
			free( g_pvModuleSpecial );
			g_pvModuleSpecial = nullptr;
		}

		Q_memset( &g_modfuncs, 0, sizeof( g_modfuncs ) );

		cl_enginefunc_dst_t engdstNull = k_engdstNull
		memcpy( &g_engdstAddrs, &engdstNull, sizeof( g_engdstAddrs ) );

		Q_memset( &g_module, 0, sizeof( g_module ) );
	}
}
