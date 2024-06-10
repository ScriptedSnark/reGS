#include <strtools.h>
#include <UtlVector.h>

#include "sys.h"
#include "traceinit.h"

class CInitTracker
{
public:
	enum
	{
		NUM_LISTS = 4
	};

	struct InitFunc
	{
		const char* initname;
		const char* shutdownname;

		int referencecount;
		int sequence;

		bool warningprinted;
		double inittime;
		double shutdowntime;
	};

public:
	CInitTracker() = default;
	~CInitTracker();

	void Init( const char* init, const char* shutdown, int listnum );

	void Shutdown( const char* shutdown, int listnum );

public:
	int m_nNumFuncs[ NUM_LISTS ] = {};

	CUtlVector<InitFunc*> m_Funcs[ NUM_LISTS ];
};

static CInitTracker g_InitTracker;

CInitTracker::~CInitTracker()
{
	for( int list = 0; list < NUM_LISTS; ++list )
	{
		if( m_nNumFuncs[ list ] > 0 )
		{
			for( int i = 0; i < m_nNumFuncs[ list ]; ++i )
			{
				auto pFunc = m_Funcs[ list ][ i ];

				if( pFunc->referencecount )
				{
					Sys_Printf( "Missing shutdown function for %s : %s\n", pFunc->initname, pFunc->shutdownname );
				}

				delete pFunc;
			}
		}

		m_Funcs[ list ].RemoveAll();
		m_nNumFuncs[ list ] = 0;
	}
}

void CInitTracker::Init( const char* init, const char* shutdown, int listnum )
{
	auto pFunc = new InitFunc;

	pFunc->initname = init;
	pFunc->shutdownname = shutdown;
	pFunc->inittime = 0.0;
	pFunc->referencecount = 1;
	pFunc->shutdowntime = 0.0;
	pFunc->sequence = m_nNumFuncs[ listnum ];
	pFunc->warningprinted = false;

	m_Funcs[ listnum ].InsertBefore( m_nNumFuncs[ listnum ], pFunc );

	++m_nNumFuncs[ listnum ];
}

void CInitTracker::Shutdown( const char* shutdown, int listnum )
{
	const auto iFuncCount = m_nNumFuncs[ listnum ];

	if( !iFuncCount )
	{
		Sys_Printf( "Mismatched shutdown function %s\n", shutdown );
		return;
	}

	//This is unlikely to be the same code that is in the engine, but it is functionally equivalent.

	if( iFuncCount > 0 )
	{
		InitFunc* pFunc = nullptr;

		for( int i = 0; i < iFuncCount; ++i )
		{
			pFunc = m_Funcs[ listnum ][ i ];

			if( pFunc->referencecount )
			{
				break;
			}
		}

		//Shutdown functions should always be called in reverse order of initialization.
		//If we get this, then a shutdown function was called out of order.
		if( pFunc && pFunc->referencecount && Q_stricmp( pFunc->shutdownname, shutdown ) )
		{
			if( !pFunc->warningprinted )
			{
				pFunc->warningprinted = true;

				//This is what it looks like when reverse engineered, but it's probably the for loop below that causes this.
				//The end of the loop jumps to inside this statement, so this is probably an early out optimization by the compiler. - Solokiller
				/*
				if( iFuncCount <= 0 )
				{
				Sys_Printf( "Shutdown function %s not in list!!!\n", shutdown );
				return;
				}
				*/
			}
		}
	}

	for( int i = 0; i < iFuncCount; ++i )
	{
		auto pFunc = m_Funcs[ listnum ][ i ];

		if( !Q_stricmp( pFunc->shutdownname, shutdown ) )
		{
			--pFunc->referencecount;
			return;
		}
	}

	Sys_Printf( "Shutdown function %s not in list!!!\n", shutdown );
}

void TraceInit( const char* i, const char* s, int listnum )
{
	g_InitTracker.Init( i, s, listnum );
}

void TraceShutdown( const char* s, int listnum )
{
	g_InitTracker.Shutdown( s, listnum );
}
