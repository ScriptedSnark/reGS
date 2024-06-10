#ifndef VGUI_SRC_VPANEL_H
#define VGUI_SRC_VPANEL_H

#include <vgui/Dar.h>
#include <vgui/IPanel.h>

class KeyValues;

namespace vgui2
{
typedef unsigned short HPanelList;

const HPanelList INVALID_PANELLIST = 0xFFFF;

/**
*	Stores per-panel data that is used for engine level operations.
*/
class VPanel
{
public:
	VPanel();
	~VPanel();

	virtual void Init( IClientPanel* attachedClientPanel );

	virtual SurfacePlat* Plat();

	virtual void SetPlat( SurfacePlat* Plat );

	virtual HPanelList GetListEntry();

	virtual void SetListEntry( HPanelList listEntry );

	virtual bool IsPopup();

	virtual void SetPopup( bool state );

	virtual bool Render_IsPopupPanelVisible();

	virtual void Render_SetPopupVisible( bool state );

	virtual void SetPos( int x, int y );

	virtual void GetPos( int& x, int& y );

	virtual void SetSize( int wide, int tall );

	virtual void GetSize( int& wide, int& tall );

	virtual void SetMinimumSize( int wide, int tall );

	virtual void GetMinimumSize( int& wide, int& tall );

	virtual void SetZPos( int z );

	virtual int GetZPos();

	virtual void GetAbsPos( int& x, int& y );

	virtual void GetClipRect( int& x0, int& y0, int& x1, int& y1 );

	virtual void SetInsets( int left, int top, int right, int bottom );

	virtual void GetInsets( int& left, int& top, int& right, int& bottom );

	virtual void Solve();

	virtual void SetVisible( bool state );

	virtual void SetEnabled( bool state );

	virtual bool IsVisible();

	virtual bool IsEnabled();

	virtual void SetParent( VPanel* newParent );

	virtual int GetChildCount();

	virtual VPanel* GetChild( int index );

	virtual VPanel* GetParent();

	virtual void MoveToFront();

	virtual void MoveToBack();

	virtual bool HasParent( VPanel* potentialParent );

	virtual const char* GetName();

	virtual const char* GetClassName();

	virtual HScheme GetScheme();

	virtual void SendMessage( KeyValues* params, VPANEL ifrompanel );

	virtual IClientPanel* Client();

	virtual void SetKeyBoardInputEnabled( bool state );

	virtual void SetMouseInputEnabled( bool state );

	virtual bool IsKeyBoardInputEnabled();

	virtual bool IsMouseInputEnabled();

private:
	Dar<VPanel*> _childDar;
	VPanel* _parent = nullptr;
	SurfacePlat* _plat = nullptr;
	HPanelList _listEntry = INVALID_PANELLIST;

	IClientPanel* _clientPanel = nullptr;

	short _pos[ 2 ] = {};
	short _size[ 2 ] = {};
	short _minimumSize[ 2 ] = {};
	short _inset[ 4 ] = {};
	short _clipRect[ 4 ] = {};
	short _absPos[ 2 ] = {};
	short _zpos = 0;

	bool _visible = true;
	bool _enabled = true;
	bool _popup = false;
	bool _popupVisible = false;
	bool _mouseInput = true;
	bool _kbInput = true;

private:
	VPanel( const VPanel& ) = delete;
	VPanel& operator=( const VPanel& ) = delete;
};

inline VPanel* VHandleToPanel( VPANEL panel )
{
	return reinterpret_cast<VPanel*>( panel );
}

inline VPANEL VPanelToHandle( VPanel* panel )
{
	return reinterpret_cast<VPANEL>( panel );
}
}

#endif //VGUI_SRC_VPANEL_H
