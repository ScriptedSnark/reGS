#include "quakedef.h"
#include "cl_demo.h"

char gDemoMessageBuffer[ 512 ] = {};

client_textmessage_t tm_demomessage =
{
	0,
	255, 255, 255, 255,
	255, 255, 255, 255,
	-1, -1,
	0, 0,
	0, 0,
	"__DEMOMESSAGE__",
	gDemoMessageBuffer
};

demo_api_t demoapi = 
{
	&CL_DemoAPIRecording,
	&CL_DemoAPIPlayback,
	&CL_DemoAPITimedemo,
	&CL_WriteClientDLLMessage
};

int CL_DemoAPIRecording()
{
	//TODO: implement - Solokiller
	return false;
}

int CL_DemoAPIPlayback()
{
	//TODO: implement - Solokiller
	return false;
}

int CL_DemoAPITimedemo()
{
	//TODO: implement - Solokiller
	return false;
}

void CL_WriteClientDLLMessage( int size, byte* buf )
{
	//TODO: implement - Solokiller
}

void CL_WriteDLLUpdate( client_data_t* cdat )
{
	//TODO: implement - Solokiller
}

void CL_DemoAnim( int anim, int body )
{
	//TODO: implement - Solokiller
}

void CL_DemoEvent( int flags, int idx, float delay, event_args_t* pargs )
{
	//TODO: implement - Solokiller
}
