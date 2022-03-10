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
	return (unsigned int)(cls.demorecording != false);
}

int CL_DemoAPIPlayback()
{
	return (unsigned int)(cls.demoplayback != false);
}

int CL_DemoAPITimedemo()
{
	return (unsigned int)(cls.timedemo != false);
}

void CL_WriteClientDLLMessage( int size, byte* buf )
{
	unsigned long localTime;
	float f;
	byte cmd;

	if ((CL_DemoAPIRecording() && (cls.demofile != NULL)) && (size > -1)) {
		cmd = '\t';
		FS_Write(&cmd, 1, cls.demofile);
		f = (realtime - cls.demostarttime);
		FS_Write(&f, 4, cls.demofile);
		localTime = (host_framecount - cls.demostartframe);
		FS_Write(&localTime, 4, cls.demofile);
		FS_Write(&size, 4, cls.demofile);
		FS_Write(buf, size, cls.demofile);
	}
	return;
}

void CL_WriteDLLUpdate( client_data_t* cdata )
{
	cl_demo_data_t demcmd;

	if (cls.demofile != NULL) {
		demcmd.cmd = '\x04';
		demcmd.time = (realtime - cls.demostarttime);
		demcmd.frame = (host_framecount - cls.demostartframe);
		demcmd.origin[0] = cdata->origin[0];
		demcmd.origin[1] = cdata->origin[1];
		demcmd.origin[2] = cdata->origin[2];
		demcmd.viewangles[0] = cdata->viewangles[0];
		demcmd.viewangles[1] = cdata->viewangles[1];
		demcmd.viewangles[2] = cdata->viewangles[2];
		demcmd.iWeaponBits = cdata->iWeaponBits;
		demcmd.fov = cdata->fov;
		FS_Write(&demcmd, 41, cls.demofile);
	}
	return;
}

void CL_DemoAnim( int anim, int body )
{
	demo_anim_t demcmd;

	if (cls.demofile != NULL) {
		demcmd.cmd = '\a';
		demcmd.time = (realtime - cls.demostarttime);
		demcmd.frame = (host_framecount - cls.demostartframe);
		demcmd.anim = anim;
		demcmd.body = body;
		FS_Write(&demcmd, 17, cls.demofile);
	}
	return;
}

void CL_DemoEvent( int flags, int idx, float delay, event_args_t* pargs )
{
	demo_event_t demcmd;

	if (cls.demofile != NULL) {
		demcmd.cmd = '\x06';
		demcmd.time = (realtime - cls.demostarttime);
		demcmd.frame = (host_framecount - cls.demostartframe);
		demcmd.flags = flags;
		demcmd.idx = idx;
		demcmd.delay = delay;
		FS_Write(&demcmd, 21, cls.demofile);
		FS_Write(pargs, 72, cls.demofile);
	}
	return;
}
