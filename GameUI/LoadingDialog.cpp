#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/PHandle.h>
#include <vgui_controls/ProgressBar.h>

#include "BitmapImagePanel.h"
#include "EngineInterface.h"
#include "GameUI_Interface.h"
#include "igameuifuncs.h"
#include "LoadingDialog.h"
#include "ModInfo.h"

CLoadingDialog* LoadingDialog()
{
	return static_cast<CLoadingDialog*>(g_hLoadingDialog.Get() );
}

CLoadingDialog::CLoadingDialog( vgui2::Panel* parent )
	: BaseClass( parent, "LoadingDialog", true )
{
	SetSize( 416, 100 );
	SetTitle( "#GameUI_Loading", true );

	m_pProgress = new vgui2::ProgressBar( this, "Progress" );
	m_pProgress2 = new vgui2::ProgressBar( this, "Progress2" );
	m_pInfoLabel = new vgui2::Label( this, "InfoLabel", "" );
	m_pCancelButton = new vgui2::Button( this, "CancelButton", "#GameUI_Cancel" );
	m_pTimeRemainingLabel = new vgui2::Label( this, "TimeRemainingLabel", "" );

	m_pCancelButton->SetCommand( "Cancel" );

	m_pInfoLabel->SetBounds( 20, 32, 392, 24 );
	m_pProgress->SetBounds( 20, 64, 300, 24 );
	m_pCancelButton->SetBounds( 330, 64, 72, 24 );

	m_pInfoLabel->SetTextColorState( vgui2::Label::CS_DULL );

	SetMinimizeButtonVisible( false );
	SetMaximizeButtonVisible( false );
	SetCloseButtonVisible( false );
	SetSizeable( false );
	SetMoveable( false );

	m_pProgress2->SetVisible( false );

	if( engine->CheckParm( "-steam", nullptr ) || engine->CheckParm( "-showplatform", nullptr ) )
	{
		if( gameuifuncs->IsConnectedToVACSecureServer() )
		{
			LoadControlSettings( "Resource/LoadingDialogVAC.res" );
			m_bShowingVACInfo = true;
		}
		else
		{
			LoadControlSettings( "Resource/LoadingDialog.res" );
		}
	}
	else
	{
		if( ModInfo().BShowSimpleLoadingDialog() )
			LoadControlSettings( "Resource/LoadingDialogNoBannerSingle.res" );
		else
			LoadControlSettings( "Resource/LoadingDialogNoBanner.res" );
	}
}

CLoadingDialog::~CLoadingDialog()
{
}

void CLoadingDialog::ApplySchemeSettings( vgui2::IScheme* pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
}

void CLoadingDialog::OnClose()
{
	vgui2::surface()->RestrictPaintToSinglePanel( NULL_HANDLE );
	BaseClass::OnClose();
	MarkForDeletion();
}

void CLoadingDialog::PerformLayout()
{
	MoveToCenterOfScreen();
	BaseClass::PerformLayout();
}

void CLoadingDialog::OnKeyCodePressed( vgui2::KeyCode code )
{
	if( code == vgui2::KEY_ESCAPE )
		OnCommand( "Cancel" );
	else
		BaseClass::OnKeyCodePressed( code );
}

void CLoadingDialog::OnCommand( const char* command )
{
	if( !stricmp( command, "Cancel" ) )
	{
		if( m_isHttpDownloadDialog )
		{
			engine->pfnClientCmd( "httpstop\n" );
		}
		else
		{
			engine->pfnClientCmd( "disconnect\n" );
		}
		
		Close();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CLoadingDialog::DisplayProgressBar()
{
	SetTitle( "#GameUI_Loading", true );

	vgui2::surface()->RestrictPaintToSinglePanel( GetVPanel() );

	Activate();

	m_pProgress->SetVisible( true );

	if( !ModInfo().BShowSimpleLoadingDialog() )
	{
		m_pInfoLabel->SetVisible( true );
	}

	m_pInfoLabel->SetText( L"" );

	m_pCancelButton->SetText( "#GameUI_Cancel" );

	m_pCancelButton->SetCommand( "Cancel" );
}

void CLoadingDialog::SetupControlSettingsForErrorDisplay( const char* settingsFile )
{
	SetTitle( "#GameUI_Disconnected", true );
	LoadControlSettings( settingsFile );

	vgui2::surface()->RestrictPaintToSinglePanel( GetVPanel() );

	Activate();
	m_pProgress->SetVisible( false );
	m_pInfoLabel->SetVisible( true );

	m_pCancelButton->SetText( "#GameUI_Close" );
	m_pCancelButton->SetCommand( "Close" );

	m_pInfoLabel->SetText( "" );
	m_pCancelButton->SetEnabled( false );
}

void CLoadingDialog::DisplayGenericError( const char* failureReason, const char* extendedReason )
{
	if( m_bShowingVACError )
	{
		m_bShowingVACError = false;
		return;
	}

	SetupControlSettingsForErrorDisplay( "Resource/LoadingDialogError.res" );

	if( !extendedReason || !( *extendedReason ) )
	{
		m_pInfoLabel->SetText( failureReason );
		return;
	}

	wchar_t finalMsg[ 512 ];
	wchar_t compositeReason[ 256 ];
	wchar_t formatStr[ 256 ];

	const wchar_t* pszLocalized;
	
	if( *extendedReason == '#'
		&& ( ( pszLocalized = vgui2::localize()->Find( extendedReason ) ) != nullptr ) )
	{
		wcsncpy( compositeReason, pszLocalized, ARRAYSIZE( compositeReason ) );
	}
	else
	{
		vgui2::localize()->ConvertANSIToUnicode( extendedReason, compositeReason, sizeof( compositeReason ) );
	}

	if( *failureReason == '#'
		&& ( ( pszLocalized = vgui2::localize()->Find( failureReason ) ) != nullptr ) )
	{
		wcsncpy( formatStr, pszLocalized, ARRAYSIZE( compositeReason ) );
	}
	else
	{
		vgui2::localize()->ConvertANSIToUnicode( failureReason, formatStr, sizeof( formatStr ) );
	}

	vgui2::localize()->ConstructString(
		finalMsg, sizeof( finalMsg ),
		formatStr, 1,
		compositeReason
	);

	m_pInfoLabel->SetText( finalMsg );
}

void CLoadingDialog::DisplayVACBannedError()
{
	SetupControlSettingsForErrorDisplay( "Resource/LoadingDialogErrorVACBanned.res" );
	SetTitle( "#VAC_ConnectionRefusedTitle", true );

	m_bShowingVACError = true;
}

void CLoadingDialog::DisplayNoSteamConnectionError()
{
	SetupControlSettingsForErrorDisplay( "Resource/LoadingDialogErrorNoSteamConnection.res" );
	m_bShowingVACError = true;
}

void CLoadingDialog::DisplaySteamConnectionError( const char* username, ESteamLoginFailure steamLoginFailure )
{
	switch( steamLoginFailure )
	{
	case STEAMLOGINFAILURE_CONNECTIONLOST:
		SetupControlSettingsForErrorDisplay( "Resource/LoadingDialogConnectionLost.res" );
		break;
	case STEAMLOGINFAILURE_NOCONNECTION:
		SetupControlSettingsForErrorDisplay( "Resource/LoadingDialogErrorNoSteamConnection.res" );
		break;
	case STEAMLOGINFAILURE_LOGGED_IN_ELSEWHERE:
		SetupControlSettingsForErrorDisplay( "Resource/LoadingDialogErrorLoggedInElsewhere.res" );
		break;

	default: break;
	}

	m_bShowingVACError = true;

	m_pCancelButton->SetText( "#GameUI_Close" );
	m_pCancelButton->SetCommand( "close" );

	strncpy( m_szUserName, username, ARRAYSIZE( m_szUserName ) );
	m_szUserName[ ARRAYSIZE( m_szUserName ) - 1 ] = '\0';
}

void CLoadingDialog::SetStatusText( const char* statusText )
{
	m_pInfoLabel->SetText( statusText );
}

void CLoadingDialog::SetProgressPoint( int progressPoint )
{
	if( !m_bShowingVACInfo && gameuifuncs->IsConnectedToVACSecureServer() )
	{
		LoadControlSettings( "Resource/LoadingDialogVAC.res" );
		m_bShowingVACInfo = true;
	}

	m_flProgress = static_cast<double>( progressPoint ) / ( m_iRangeMax - m_iRangeMin );

	m_pProgress->SetProgress( m_flProgress );
}

void CLoadingDialog::SetProgressRange( int min, int max )
{
	m_iRangeMin = min;
	m_iRangeMax = max;
}

void CLoadingDialog::SetSecondaryProgress( float progress )
{
	if( !m_bShowingSecondaryProgress )
	{
		if( progress > 0.99 )
			return;

		if( m_bShowingVACInfo || gameuifuncs->IsConnectedToVACSecureServer() )
		{
			LoadControlSettings( "Resource/LoadingDialogDualProgressVAC.res" );
			m_bShowingVACInfo = true;
		}
		else
		{
			LoadControlSettings( "Resource/LoadingDialogDualProgress.res" );
		}

		m_bShowingSecondaryProgress = true;
		m_pProgress->SetProgress( m_flProgress );

		m_pProgress2->SetVisible( true );

		m_flSecondaryProgressStartTime = vgui2::system()->GetFrameTime();
	}

	if( progress > m_flSecondaryProgress )
	{
		m_pProgress2->SetProgress( progress );
		m_flSecondaryProgress = progress;
		m_flLastSecondaryProgressUpdateTime = vgui2::system()->GetFrameTime();
	}

	if( m_flSecondaryProgress > progress )
	{
		m_pProgress2->SetProgress( progress );
		m_flSecondaryProgress = progress;
		m_flLastSecondaryProgressUpdateTime = vgui2::system()->GetFrameTime();
		m_flSecondaryProgressStartTime = vgui2::system()->GetFrameTime();
	}
}

void CLoadingDialog::SetSecondaryProgressText( const char* statusText )
{
	SetControlString( "SecondaryProgressLabel", statusText );
}

void CLoadingDialog::SwitchToHttpDownloadDialog()
{
	m_isHttpDownloadDialog = true;
	LoadControlSettings( "Resource/LoadingDialog.res" );
}

void CLoadingDialog::OnThink()
{
	BaseClass::OnThink();

	if( m_bShowingSecondaryProgress )
	{
		if( m_flSecondaryProgress >= 1.0 )
		{
			m_pTimeRemainingLabel->SetText( "complete" );
		}
		else
		{
			wchar_t unicode[ 512 ];

			if( vgui2::ProgressBar::ConstructTimeRemainingString(
				unicode,
				sizeof( unicode ),
				m_flSecondaryProgressStartTime,
				vgui2::system()->GetFrameTime(),
				m_flSecondaryProgress,
				m_flLastSecondaryProgressUpdateTime,
				true ) )
			{
				m_pTimeRemainingLabel->SetText( unicode );
			}
			else
			{
				m_pTimeRemainingLabel->SetText( "" );
			}
		}
	}
}
