#include "quakedef.h"
#include "cl_parsefn.h"

void CL_InitEventSystem()
{
	//TODO: implement - Solokiller
}

void CL_HookEvent( char* name, void( *pfnEvent )( event_args_t* ) )
{
	//TODO: implement - Solokiller
}

void CL_QueueEvent( int flags, int index, float delay, event_args_t* pargs )
{
	//TODO: implement - Solokiller
}

void CL_ResetEvent( event_info_t* ei )
{
	ei->index = 0;
	Q_memset( &ei->args, 0, sizeof( ei->args ) );
	ei->fire_time = 0;
	ei->flags = 0;
}
