#include <SDL2/SDL.h>
#include <GL/glew.h>

#include <UtlVector.h>

#include "quakedef.h"

#include "gl_vidnt.h"
#include "IGame.h"
#include "IRegistry.h"
#include "render.h"
#include "sound.h"
#include "sys_getmodes.h"
#include "vid.h"

bool bNeedsFullScreenModeSwitch = false;

IVideoMode* videomode = nullptr;

class CVideoMode_Common : public IVideoMode
{
public:
	struct bimage_t
	{
		byte *buffer;
		int x;
		int y;
		int width;
		int height;
		bool scaled;
	 };

	static const int MAX_MODES = 32;

public:
	CVideoMode_Common();

	bool Init( void* pvInstance ) override;

protected:
	void LoadStartupGraphic();
	void CenterEngineWindow( SDL_Window* hWndCenter, int width, int height );
	void AdjustWindowForMode();
	void DrawStartupGraphic( SDL_Window* window );

public:
	void Shutdown() override;

	bool AddMode( int width, int height, int bpp ) override;
	vmode_t* GetCurrentMode() override;
	vmode_t* GetMode( int num ) override;
	int GetModeCount() override;

	bool IsWindowedMode() const override;

	bool GetInitialized() const override;
	void SetInitialized( bool init ) override;

	void UpdateWindowPosition() override;

	void FlipScreen() override;

	void RestoreVideo() override;
	void ReleaseVideo() override;

	virtual int MaxBitsPerPixel();
	virtual void ReleaseFullScreen();
	virtual void ChangeDisplaySettingsToFullscreen();

protected:
	vmode_t m_rgModeList[ MAX_MODES ] = {};
	vmode_t m_safeMode;
	int m_nNumModes = 0;
	int m_nGameMode = 0;
	bool m_bInitialized = false;
	CUtlVector<CVideoMode_Common::bimage_t> m_ImageID;
	int m_iBaseResX = 0;
	int m_iBaseResY = 0;
	bool m_bWindowed = false;
};

CVideoMode_Common::CVideoMode_Common()
{
	m_safeMode.width = 640;
	m_safeMode.height = 480;
	m_safeMode.bpp = 32;
}

void SetupSDLVideoModes()
{
	SDL_SetHint( "SDL_VIDEO_X11_XRANDR", "1" );
	SDL_SetHint( "SDL_VIDEO_X11_XVIDMODE", "1" );

	SDL_InitSubSystem( SDL_INIT_VIDEO );

	const int iNumModes = SDL_GetNumDisplayModes( 0 );

	SDL_DisplayMode mode;

	for( int i = 0; i < iNumModes; ++i )
	{
		if( SDL_GetDisplayMode( 0, i, &mode ) )
		{
			break;
		}

		videomode->AddMode( mode.w, mode.h, 32 );
	}

	SDL_QuitSubSystem( SDL_INIT_VIDEO );
}

static int VideoModeCompare( const void *arg1, const void *arg2 )
{
	auto& lhs = *reinterpret_cast<const vmode_t*>( arg1 );
	auto& rhs = *reinterpret_cast<const vmode_t*>( arg2 );

	int result = -1;

	if( lhs.width >= rhs.width )
	{
		result = 1;

		if( lhs.width == rhs.width )
		{
			result = -1;

			if( lhs.height >= rhs.height )
				result = lhs.height > rhs.height;
		}
	}

	return result;
}

bool CVideoMode_Common::Init( void* pvInstance )
{
	if( !game->CreateGameWindow() )
		return false;

	int width = registry->ReadInt( "ScreenWidth", 1024 );
	int height = registry->ReadInt( "ScreenHeight", 768 );

	registry->ReadInt( "ScreenBPP", 32 );

	COM_CheckParm( "-16bpp" );
	COM_CheckParm( "-24bpp" );
	COM_CheckParm( "-32bpp" );

	MaxBitsPerPixel();

	m_nNumModes = 0;

	SetupSDLVideoModes();

	if( m_nNumModes > 1 )
		qsort( m_rgModeList, m_nNumModes, sizeof( vmode_t ), VideoModeCompare );

	int widthParam = COM_CheckParm( "-width" );
	if( widthParam )
		width = strtol( com_argv[ widthParam + 1 ], nullptr, 10 );

	widthParam = COM_CheckParm( "-w" );
	if( widthParam )
		width = strtol( com_argv[ widthParam + 1 ], nullptr, 10 );

	int heightParam = COM_CheckParm( "-height" );
	if( heightParam )
		height = strtol( com_argv[ heightParam + 1 ], nullptr, 10 );

	heightParam = COM_CheckParm( "-h" );
	if( heightParam )
		height = strtol( com_argv[ heightParam + 1 ], nullptr, 10 );

	if( IsWindowedMode() )
	{
		SDL_Rect rect;

		if( !SDL_GetDisplayBounds( 0, &rect ) )
		{
			if( width > rect.w )
				width = rect.w;
			if( height > rect.h )
				height = rect.h;
		}
	}

	if( height < 0 && width > 0 )
		height = static_cast<int>( width * 3.0 * 0.25 );

	int iMode;

	for( iMode = 0; iMode < m_nNumModes; ++iMode )
	{
		if( width == m_rgModeList[ iMode ].width && height == m_rgModeList[ iMode ].height )
		{
			break;
		}
	}

	//Use first mode if none found.
	if( iMode == m_nNumModes )
		iMode = 0;

	m_nGameMode = iMode;

	if( IsWindowedMode()
		|| !COM_CheckParm( "-nofbo" ) && Host_GetVideoLevel() <= 0 )
	{
		if( COM_CheckParm( "-forceres" ) || m_nNumModes <= 1 )
		{
			auto& mode = m_rgModeList[ iMode ];

			mode.width = width;
			mode.height = height;

			if( !m_nNumModes )
				m_nNumModes = 1;
		}
	}

	auto& mode = m_rgModeList[ iMode ];

	VideoMode_SetVideoMode( mode.width, mode.height, mode.bpp );

	LoadStartupGraphic();
	AdjustWindowForMode();
	game->PlayStartupVideos();
	DrawStartupGraphic( reinterpret_cast<SDL_Window*>( game->GetMainWindow() ) );

	return true;
}

void CVideoMode_Common::LoadStartupGraphic()
{
	FileHandle_t file = FS_Open( "resource/BackgroundLoadingLayout.txt", "rt" );

	if( file == FILESYSTEM_INVALID_HANDLE )
		return;

	const unsigned int uiSize = FS_Size( file );
	char* buffer = reinterpret_cast<char*>( alloca( uiSize + 16 ) );

	memset( buffer, 0, uiSize );

	FS_Read( buffer, uiSize, file );
	FS_Close( file );

	if( *buffer )
	{
		char token[ 512 ];

		do
		{
			buffer = FS_ParseFile( buffer, token, nullptr );
			if( !buffer || !token[ 0 ] )
				break;
			if( !stricmp( token, "resolution" ) )
			{
				buffer = FS_ParseFile( buffer, token, nullptr );
				m_iBaseResX = strtol( token, nullptr, 10 );
				buffer = FS_ParseFile( buffer, token, nullptr );
				m_iBaseResY = strtol( token, nullptr, 10 );
			}
			else
			{
				auto& image = m_ImageID.Element( m_ImageID.AddToTail() );
				image.buffer = reinterpret_cast<byte*>( malloc( 512 * 512 ) );

				if( !LoadTGA( token, image.buffer, 512 * 512, &image.width, &image.height ) )
				{
					m_ImageID.Remove( m_ImageID.Size() - 1 );
					return;
				}

				buffer = FS_ParseFile( buffer, token, nullptr );
				image.scaled = stricmp( token, "scaled" ) == 0;

				buffer = FS_ParseFile( buffer, token, nullptr );
				image.x = strtol( token, nullptr, 10 );

				buffer = FS_ParseFile( buffer, token, nullptr );
				image.y = strtol( token, nullptr, 10 );
			}
		}
		while( buffer && *buffer );
	}
}

void CVideoMode_Common::CenterEngineWindow( SDL_Window* hWndCenter, int width, int height )
{
	SDL_Rect rect;

	SDL_GetDisplayBounds( 0, &rect );

	int x = ( rect.w - width ) / 2;
	if( x < 0 )
		x = 0;

	int y = ( rect.h - height ) / 2;
	if( y < 0 )
		y = 0;

	game->SetWindowXY( x, y );
	SDL_SetWindowPosition( reinterpret_cast<SDL_Window*>( game->GetMainWindow() ), x, y );
}

void CVideoMode_Common::AdjustWindowForMode()
{
	auto pMode = GetCurrentMode();

	Snd_ReleaseBuffer();

	vid.width = pMode->width;
	vid.height = pMode->height;

	int width, height;

	if( IsWindowedMode() )
	{
		auto pGameMode = &m_rgModeList[ m_nGameMode ];

		width = pGameMode->width;
		height = pGameMode->height;
	}
	else
	{
		width = pMode->width;
		height = pMode->height;
		ChangeDisplaySettingsToFullscreen();
		SDL_SetWindowPosition( reinterpret_cast<SDL_Window*>( game->GetMainWindow() ), 0, 0 );
	}

	game->SetWindowSize( width, height );

	CenterEngineWindow( reinterpret_cast<SDL_Window*>( game->GetMainWindow() ), width, height );

	Snd_AcquireBuffer();
	VOX_Init();

	UpdateWindowPosition();
}

void CVideoMode_Common::DrawStartupGraphic( SDL_Window* window )
{
	if( m_ImageID.Size() == 0 )
		return;

	int width, height;

	SDL_GetWindowSize( window, &width, &height );

	glViewport( 0, 0, width, height );
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );
	glMatrixMode( GL_PROJECTION );
	glOrtho( 0.0, static_cast<double>( width ), static_cast<double>( height ), 0.0, -1.0, 1.0 );
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClear( GL_COLOR_BUFFER_BIT );

	auto pMode = GetCurrentMode();

	int modeWidth = width, modeHeight = height;

	if( pMode )
	{
		modeWidth = pMode->width;
		modeHeight = pMode->height;
	}

	double widthAdjust = 0;
	double heightAdjust = 0;

	if( !COM_CheckParm( "-stretchaspect" ) )
	{
		const double modeAspect = static_cast<double>( modeWidth ) / static_cast<double>( modeHeight );
		const double aspect = static_cast<double>( width ) / static_cast<double>( height );

		if( aspect > modeAspect )
		{
			widthAdjust = static_cast<double>( width ) - modeAspect * static_cast<double>( height );
		}
		else if( modeAspect > aspect )
		{
			const float flYDiff = 1.0 / modeAspect * static_cast<double>( width );
			heightAdjust = static_cast<double>( height - flYDiff );
		}
	}

	const float xScale = ( width - widthAdjust ) / static_cast<double>( m_iBaseResX );
	const float yScale = ( height - heightAdjust ) / static_cast<double>( m_iBaseResY );

	CUtlVector<unsigned int> vecGLTex;

	const double xOffset = widthAdjust * 0.5;
	const double yOffset = heightAdjust * 0.5;

	for( int image = 0; image < m_ImageID.Size(); ++image )
	{
		bimage_t* pImage = &m_ImageID[ image ];

		const int dx = static_cast<int>( round( pImage->x * xScale + xOffset ) );
		const int dy = static_cast<int>( round( pImage->y * yScale + yOffset ) );

		int dw, dt;

		if( pImage->scaled )
		{
			dw = static_cast<int>( round( ( pImage->width + pImage->x ) * xScale + xOffset ) );
			dt = static_cast<int>( round( ( pImage->height + pImage->y ) * yScale + yOffset ) );
		}
		else
		{
			dw = pImage->width + dx;
			dt = pImage->height + dy;
		}

		auto& tex = vecGLTex.Element( vecGLTex.AddToTail() );

		glGenTextures( 1, &tex );

		glBindTexture( GL_TEXTURE_2D, tex );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		int pow2Width = 0;
		int pow2Height = 0;

		for( int i = 0; i < 32; ++i )
		{
			pow2Width = 1 << i;

			if( pImage->width <= pow2Width )
				break;
		}

		for( int i = 0; i < 32; ++i )
		{
			pow2Height = 1 << i;

			if( pImage->height <= pow2Height )
				break;
		}

		float topu, topv;

		if( pImage->width == pow2Width && pImage->height == pow2Height )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, pImage->width, pImage->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage->buffer );
			topu = 1.0;
			topv = 1.0;
		}
		else
		{
			auto pToUse = new byte[ 4 * pow2Width * pow2Height ];

			//TODO: leaks GPU memory, but if we're out of RAM here it doesn't really matter. - Solokiller
			if( !pToUse )
				return;

			memset( pToUse, 0, 4 * pow2Width * pow2Height );

			//Copy the image into a power of 2 buffer.
			for( int y = 0; y < pImage->height; ++y )
			{
				for( int x = 0; x < pImage->width; ++x )
				{
					//Copy 4 bytes at a time, one pixel.
					*reinterpret_cast<uint32*>( &pToUse[ 4 * ( x + ( y * pow2Width ) ) ] ) = *reinterpret_cast<uint32*>( &pImage->buffer[ 4 * ( x + ( y * pow2Width ) ) ] );
				}
			}

			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, pow2Width, pow2Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pToUse );

			topu = pImage->width / static_cast<double>( pow2Width );
			topv = static_cast<double>( pImage->height ) / static_cast<double>( pow2Height );
			delete[] pToUse;
		}

		glBegin( GL_QUADS );
		glTexCoord2f( 0.0, topv );
		glVertex3i( dx, dt, 0 );
		glTexCoord2f( topu, topv );
		glVertex3i( dw, dt, 0 );
		glTexCoord2f( topu, 0.0 );
		glVertex3i( dw, dy, 0 );
		glTexCoord2f( 0.0, 0.0 );
		glVertex3i( dx, dy, 0 );
		glEnd();
	}

	SDL_GL_SwapWindow( window );

	for( int i = 0; i < vecGLTex.Size(); ++i )
	{
		glDeleteTextures( 1, &vecGLTex[ i ] );
	}

	for( int i = 0; i < m_ImageID.Size(); ++i )
	{
		if( m_ImageID[ i ].buffer )
		{
			free( m_ImageID[ i ].buffer );
		}
	}

	//TODO: can purge memory as well since it's never going to be used again. - Solokiller
	m_ImageID.RemoveAll();
}

void CVideoMode_Common::Shutdown()
{
	ReleaseFullScreen();

	if( GetInitialized() )
	{
		SetInitialized( false );
		videomode = nullptr;
		m_nGameMode = 0;
		ReleaseVideo();
	}
}

bool CVideoMode_Common::AddMode( int width, int height, int bpp )
{
	if( m_nNumModes >= MAX_MODES )
		return false;

	//Check if it's already in the list;
	for( int i = 0; i < m_nNumModes; ++i )
	{
		if( m_rgModeList[ i ].width == width && 
			m_rgModeList[ i ].height == height && 
			m_rgModeList[ i ] .bpp == bpp )
		{
			return true;
		}
	}

	auto& mode = m_rgModeList[ m_nNumModes ];

	mode.width = width;
	mode.height = height;
	mode.bpp = bpp;

	++m_nNumModes;

	return true;
}

vmode_t* CVideoMode_Common::GetCurrentMode()
{
	return GetMode( m_nGameMode );
}

vmode_t* CVideoMode_Common::GetMode( int num )
{
	if( num >= 0 && num < m_nNumModes )
		return &m_rgModeList[ num ];
	else
		return &m_safeMode;
}

int CVideoMode_Common::GetModeCount()
{
	return m_nNumModes;
}

bool CVideoMode_Common::IsWindowedMode() const
{
	return m_bWindowed;
}

bool CVideoMode_Common::GetInitialized() const
{
	return m_bInitialized;
}

void CVideoMode_Common::SetInitialized( bool init )
{
	m_bInitialized = init;
}

void CVideoMode_Common::UpdateWindowPosition()
{
	Rect_t window_rect;
	int x, y, w, h;

	game->GetWindowRect( &x, &y, &w, &h );

	window_rect.x = x;
	window_rect.width = x + w;
	window_rect.y = y;
	window_rect.height = y + h;
	VID_UpdateWindowVars( &window_rect, x + w / 2, y + h / 2 );
}

void CVideoMode_Common::FlipScreen()
{
	//Nothing
}

void CVideoMode_Common::RestoreVideo()
{
	//Nothing
}

void CVideoMode_Common::ReleaseVideo()
{
	//Nothing
}

int CVideoMode_Common::MaxBitsPerPixel()
{
	return 32;
}

void CVideoMode_Common::ReleaseFullScreen()
{
	//Nothing
}

void CVideoMode_Common::ChangeDisplaySettingsToFullscreen()
{
	//Nothing
}

class CVideoMode_OpenGL final : public CVideoMode_Common
{
public:
	CVideoMode_OpenGL( bool windowed );

	bool Init( void* pvInstance ) override;

	const char* GetName() override { return "gl"; }

	void FlipScreen() override;

	void RestoreVideo() override;
	void ReleaseVideo() override;

	void ReleaseFullScreen() override;
	void ChangeDisplaySettingsToFullscreen() override;
};

CVideoMode_OpenGL::CVideoMode_OpenGL( bool windowed )
{
	m_bWindowed = windowed;
}

bool CVideoMode_OpenGL::Init( void* pvInstance )
{
	const bool result = CVideoMode_Common::Init( pvInstance );

	SetInitialized( result );

	return result;
}

void CVideoMode_OpenGL::FlipScreen()
{
	Sys_Error( "This shouldn't be called\n" );
}

void CVideoMode_OpenGL::RestoreVideo()
{
	if( !IsWindowedMode() )
	{
		AdjustWindowForMode();
		SDL_ShowWindow( reinterpret_cast<SDL_Window*>( game->GetMainWindow() ) );
	}
}

void CVideoMode_OpenGL::ReleaseVideo()
{
	if( !IsWindowedMode() )
		ReleaseFullScreen();
}

void CVideoMode_OpenGL::ReleaseFullScreen()
{
	if( !IsWindowedMode() )
	{
		SDL_SetWindowFullscreen( reinterpret_cast<SDL_Window*>( game->GetMainWindow() ), 0 );
	}
}

void CVideoMode_OpenGL::ChangeDisplaySettingsToFullscreen()
{
	static Uint32 iLastScreenMode = SDL_WINDOW_FULLSCREEN_DESKTOP;

	if( !IsWindowedMode() && GetCurrentMode() )
	{
		Uint32 flags = SDL_WINDOW_FULLSCREEN;

		if( !bNeedsFullScreenModeSwitch )
			flags = SDL_WINDOW_FULLSCREEN_DESKTOP;

		//Force to windowed first before switching to fullscreen mode to force full update.
		if( iLastScreenMode != flags )
		{
			SDL_SetWindowFullscreen( reinterpret_cast<SDL_Window*>( game->GetMainWindow() ), 0 );
		}

		SDL_SetWindowFullscreen( reinterpret_cast<SDL_Window*>( game->GetMainWindow() ), flags );

		iLastScreenMode = flags;
	}
}

void VideoMode_Create()
{
	bool bWindowed;

	if( registry->ReadInt( "ScreenWindowed", 0 ) ||
		COM_CheckParm( "-sw" ) ||
		COM_CheckParm( "-startwindowed" ) ||
		COM_CheckParm( "-windowed" ) ||
		COM_CheckParm( "-window" ) )
	{
		bWindowed = true;
	}
	else if( COM_CheckParm( "-full" ) || COM_CheckParm( "-fullscreen" ) )
	{
		bWindowed = false;
	}
	else
	{
		bWindowed = false;
	}

	VideoMode_SwitchMode( true, bWindowed );

	registry->ReadInt( "EngineD3D", 0 );

	if( !COM_CheckParm( "-d3d" ) )
		COM_CheckParm( "-gl" );

	videomode = new CVideoMode_OpenGL( bWindowed );
}

bool VideoMode_IsWindowed()
{
	return videomode->IsWindowedMode();
}

void VideoMode_GetVideoModes( vmode_t** liststart, int* count )
{
	*count = videomode->GetModeCount();
	*liststart = videomode->GetMode( 0 );
}

void VideoMode_GetCurrentVideoMode( int *wide, int *tall, int *bpp )
{
	vmode_t* pMode = videomode->GetCurrentMode();

	if( pMode )
	{
		if( wide )
			*wide = pMode->width;
		if( tall )
			*tall = pMode->height;
		if( bpp )
			*bpp = pMode->bpp;
	}
}

void VideoMode_GetCurrentRenderer( char* name, int namelen, int* windowed, int* hdmodels, int* addons_folder, int* vid_level )
{
	if( namelen > 0 && name )
	{
		strncpy( name, videomode->GetName(), namelen );
	}
	if( windowed )
		*windowed = videomode->IsWindowedMode();

	if( hdmodels )
		*hdmodels = registry->ReadInt( "hdmodels", 1 ) > 0;
	
	if( addons_folder )
		*addons_folder = registry->ReadInt( "addons_folder", 0 ) > 0;

	if( vid_level )
		*vid_level = registry->ReadInt( "vid_level", 0 );
}

void VideoMode_RestoreVideo()
{
	videomode->RestoreVideo();
}

void VideoMode_SetVideoMode( int width, int height, int bpp )
{
	registry->WriteInt( "ScreenWidth", width );
	registry->WriteInt( "ScreenHeight", height );
	registry->WriteInt( "ScreenBPP", bpp );
}

void VideoMode_SwitchMode( int hardware, int windowed )
{
	registry->WriteInt( "ScreenWindowed", windowed );
}
