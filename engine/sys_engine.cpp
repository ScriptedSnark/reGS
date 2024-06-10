#include "quakedef.h"
#include "cdll_int.h"
#include "dll_state.h"

#include "host.h"
#include "IEngine.h"
#include "IGame.h"

class CEngine final : public IEngine
{
public:
	CEngine() = default;
	~CEngine() = default;

	bool Load( bool dedicated, char* basedir, char* cmdline ) override;
	void Unload() override;

	void SetState( int iState ) override;
	int GetState() override;

	void SetSubState( int iSubState ) override;
	int GetSubState() override;

	int Frame() override;
	double GetFrameTime() override;
	double GetCurTime() override;

	void TrapKey_Event( int key, bool down ) override;
	void TrapMouse_Event( int buttons, bool down ) override;
	void StartTrapMode() override;
	bool IsTrapping() override;
	bool CheckDoneTrapping( int& buttons, int& key ) override;

	int GetQuitting() override;
	void SetQuitting( int quittype ) override;

private:
	int m_nQuitting = 0;
	int m_nDLLState = DLL_INACTIVE;
	int m_nSubState = 0;

	double m_fCurTime = 0;
	double m_fFrameTime = 0;
	double m_fOldTime = 0;

	bool m_bTrapMode = false;
	bool m_bDoneTrapping = false;
	int m_nTrapKey = 0;
	int m_nTrapButtons = 0;
};

namespace
{
static CEngine g_Engine;
}

IEngine* eng = &g_Engine;

bool CEngine::Load( bool dedicated, char* basedir, char* cmdline )
{
	SetState( DLL_ACTIVE );

	bool bInitialized = Sys_InitGame( cmdline, basedir, game->GetMainWindow(), dedicated );

	if( bInitialized )
	{
		ForceReloadProfile();
	}

	return bInitialized;
}

void CEngine::Unload()
{
	Sys_ShutdownGame();
	m_nDLLState = DLL_INACTIVE;
}

void CEngine::SetState( int iState )
{
	m_nDLLState = iState;
	GameSetState( iState );
}

int CEngine::GetState()
{
	return m_nDLLState;
}

void CEngine::SetSubState( int iSubState )
{
	if( iSubState != 1 )
	{
		GameSetSubState( iSubState );
	}
}

int CEngine::GetSubState()
{
	return m_nSubState;
}

int CEngine::Frame()
{
	if( !game->IsMultiplayer() )
	{
		int delay = 50;

		if( m_nDLLState != DLL_PAUSED )
			delay = 20;

		game->SleepUntilInput( delay );
	}

	m_fCurTime = Sys_FloatTime();
	m_fFrameTime = m_fCurTime - m_fOldTime;
	m_fOldTime = m_fCurTime;

	if( m_fFrameTime < 0.0 )
	{
		m_fFrameTime = 0.01;
	}

	if( m_nDLLState != DLL_INACTIVE )
	{
		int dummy;
		const auto result = Host_Frame( m_fCurTime, m_nDLLState, &dummy );

		if( result != m_nDLLState )
		{
			SetState( result );

			if( m_nDLLState == DLL_CLOSE )
			{
				SetQuitting( QUIT_TODESKTOP );
			}
			else if( m_nDLLState == DLL_STATE_RESTART )
			{
				SetQuitting( QUIT_RESTART );
			}
		}
	}

	return m_nDLLState;
}

double CEngine::GetFrameTime()
{
	return m_fFrameTime;
}

double CEngine::GetCurTime()
{
	return m_fCurTime;
}

void CEngine::TrapKey_Event( int key, bool down )
{
	bool bIsDown = down;

	bool bHandled = false;

	if( m_bTrapMode )
	{
		bIsDown = false;

		if( down )
		{
			m_bTrapMode = false;
			m_bDoneTrapping = true;
			m_nTrapKey = key;
			m_nTrapButtons = 0;

			bHandled = true;
		}
	}

	if( !bHandled )
	{
		Key_Event( key, bIsDown );
	}
}

void CEngine::TrapMouse_Event( int buttons, bool down )
{
	if( m_bTrapMode && buttons && !down )
	{
		m_bTrapMode = false;
		m_bDoneTrapping = true;
		m_nTrapKey = 0;
		m_nTrapButtons = buttons;
	}
	else
	{
		ClientDLL_MouseEvent( buttons );
	}
}

void CEngine::StartTrapMode()
{
	if( !m_bTrapMode )
	{
		m_bDoneTrapping = false;
		m_bTrapMode = true;
	}
}

bool CEngine::IsTrapping()
{
	return m_bTrapMode;
}

bool CEngine::CheckDoneTrapping( int& buttons, int& key )
{
	if( m_bTrapMode )
	{
		return false;
	}

	if( m_bDoneTrapping )
	{
		key = m_nTrapKey;
		buttons = m_nTrapButtons;
		m_bDoneTrapping = false;
		return true;
	}

	return false;
}

int CEngine::GetQuitting()
{
	return m_nQuitting;
}

void CEngine::SetQuitting( int quittype )
{
	m_nQuitting = quittype;
}
