#ifndef ENGINE_SYS_GETMODES_H
#define ENGINE_SYS_GETMODES_H

#include "igameuifuncs.h"

/**
*	Manages the main window's video modes.
*/
class IVideoMode
{
public:
	virtual const char* GetName() = 0;

	virtual bool Init( void* pvInstance ) = 0;
	virtual void Shutdown() = 0;

	virtual bool AddMode( int width, int height, int bpp ) = 0;
	virtual vmode_t* GetCurrentMode() = 0;
	virtual vmode_t* GetMode( int num ) = 0;
	virtual int GetModeCount() = 0;

	virtual bool IsWindowedMode() const = 0;

	virtual bool GetInitialized() const = 0;
	virtual void SetInitialized( bool init ) = 0;

	virtual void UpdateWindowPosition() = 0;

	virtual void FlipScreen() = 0;

	virtual void RestoreVideo() = 0;
	virtual void ReleaseVideo() = 0;
};

extern bool bNeedsFullScreenModeSwitch;

extern IVideoMode* videomode;

bool BUsesSDLInput();

void VideoMode_Create();

bool VideoMode_IsWindowed();

void VideoMode_GetVideoModes( vmode_t** liststart, int* count );

void VideoMode_GetCurrentVideoMode( int* wide, int* tall, int* bpp );

void VideoMode_GetCurrentRenderer( char* name, int namelen, int* windowed, int* hdmodels, int* addons_folder, int* vid_level );

void VideoMode_RestoreVideo();

void VideoMode_SetVideoMode( int width, int height, int bpp );

void VideoMode_SwitchMode( int hardware, int windowed );

#endif //ENGINE_SYS_GETMODES_H
