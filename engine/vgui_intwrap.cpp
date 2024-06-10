#include "quakedef.h"
#include "vgui_int.h"

//SDL defines this - Solokiller
#undef main
#include <VGUI_App.h>

#include "cdll_int.h"

#include "interface.h"

#include "EngineSurface.h"
#include "sound.h"
#include "sys_getmodes.h"
#include "VGUI_EngineSurfaceWrap.h"

SDL_Window* pmainwindow = nullptr;

static EngineSurfaceWrap* staticEngineSurface = nullptr;

static vgui::Panel* staticPanel = nullptr;

void VGuiWrap_SetRootPanelSize()
{
	auto pRoot = VGuiWrap_GetPanel();

	if( pRoot )
	{
		int x = 0, y = 0;
		Rect_t rect;

		rect.y = 0;

		if( VideoMode_IsWindowed() )
		{
			SDL_GetWindowPosition( pmainwindow, &x, &y );
			SDL_GetWindowSize( pmainwindow, &rect.width, &rect.height );
		}
		else
		{
			VideoMode_GetCurrentVideoMode( &rect.width, &rect.height, nullptr );
		}

		rect.height += rect.y;

		pRoot->setBounds( x, y, rect.width, rect.height );
	}
}

void VGuiWrap_Startup()
{
	if( staticEngineSurface )
		return;

	auto pApp = vgui::App::getInstance();

	pApp->reset();

	staticPanel = new vgui::Panel( 0, 0, 320, 240 );

	auto pScheme = pApp->getScheme();

	staticPanel->setPaintBorderEnabled( false );
	staticPanel->setPaintBackgroundEnabled( false );
	staticPanel->setPaintEnabled( false );
	staticPanel->setCursor( pScheme->getCursor( vgui::Scheme::scu_none ) );

	auto factoryFn = Sys_GetFactoryThis();

	auto pSurface = static_cast<IEngineSurface*>( factoryFn( ENGINESURFACE_INTERFACE_VERSION, nullptr ) );

	staticEngineSurface = new EngineSurfaceWrap( staticPanel, pSurface );

	VGuiWrap_SetRootPanelSize();
}

void VGuiWrap_Shutdown()
{
	delete staticPanel;
	staticPanel = nullptr;

	if( staticEngineSurface )
		delete staticEngineSurface;

	staticEngineSurface = nullptr;
}

bool VGuiWrap_CallEngineSurfaceAppHandler( void* event, void* userData )
{
	if( staticEngineSurface )
		staticEngineSurface->AppHandler( event, userData );

	return false;
}

vgui::Panel* VGuiWrap_GetPanel()
{
	g_engdstAddrs.VGui_GetPanel();

	return staticPanel;
}

void VGuiWrap_ReleaseMouse()
{
	if( vgui::App::getInstance() && staticEngineSurface )
	{
		if( VGuiWrap2_UseVGUI1() )
		{
			staticEngineSurface->setCursor( 
				vgui::App::getInstance()->
					getScheme()->
						getCursor( vgui::Scheme::scu_arrow ) );

			staticEngineSurface->lockCursor();
		}

		ClientDLL_DeactivateMouse();
		SetMouseEnable( false );
	}
}

void VGuiWrap_GetMouse()
{
	if( staticEngineSurface )
	{
		if( VGuiWrap2_UseVGUI1() )
			staticEngineSurface->unlockCursor();

		ClientDLL_ActivateMouse();
		SetMouseEnable( true );
	}
}

void VGuiWrap_SetVisible( bool state )
{
	if( staticPanel )
	{
		staticPanel->setVisible( state );
	}
}

void VGuiWrap_Paint( bool paintAll )
{
	auto pRoot = VGuiWrap_GetPanel();

	if( pRoot )
	{
		VGuiWrap_SetRootPanelSize();

		vgui::App::getInstance()->externalTick();

		if( paintAll )
		{
			pRoot->paintTraverse();
		}
		else
		{
			int extents[ 4 ];

			pRoot->getAbsExtents(
				extents[ 0 ],
				extents[ 1 ],
				extents[ 2 ],
				extents[ 3 ]
			);

			VGui_ViewportPaintBackground( extents );
		}
	}
}

class CDummyApp : public vgui::App
{
public:
	void main( int argc, char* argv[] ) override;

protected:
	void platTick() override;
};

void CDummyApp::main( int argc, char* argv[] )
{
	//Nothing
}

void CDummyApp::platTick()
{
	//Nothing
}

//App for VGUI programs, globally accessed through vgui::App::getInstance().
static CDummyApp theApp;
