#ifndef ENGINE_IGAME_H
#define ENGINE_IGAME_H

/**
*	Manages the game's engine level state.
*/
class IGame
{
public:
	virtual ~IGame() {}

	virtual bool Init( void *pvInstance ) = 0;
	virtual bool Shutdown() = 0;

	virtual bool CreateGameWindow() = 0;
	virtual void SleepUntilInput( int time ) = 0;

	virtual void* GetMainWindow() = 0;
	virtual void** GetMainWindowAddress() = 0;

	virtual void SetWindowXY( int x, int y ) = 0;
	virtual void SetWindowSize( int w, int h ) = 0;
	virtual void GetWindowRect( int* x, int* y, int* w, int* h ) = 0;

	virtual bool IsActiveApp() = 0;
	virtual bool IsMultiplayer() = 0;

	virtual void PlayStartupVideos() = 0;
	virtual void PlayAVIAndWait( const char *fileName ) = 0;

	virtual void SetCursorVisible( bool bState ) = 0;
};

extern IGame* game;

#endif //ENGINE_IGAME_H
