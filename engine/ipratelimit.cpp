#include <ctime>

#include "quakedef.h"
#include "ipratelimit.h"

bool CIPRateLimit::LessIP( const iprate_t& lhs, const iprate_t& rhs )
{
	return lhs.ip < rhs.ip;
}

CIPRateLimit::CIPRateLimit()
	: m_IPTree( 0, START_TREE_SIZE, &CIPRateLimit::LessIP )
{
}

bool CIPRateLimit::CheckIP( netadr_t adr )
{
	const time_t curTime = time( nullptr );

	if( m_IPTree.Count() > MAX_TREE_SIZE )
	{
		//Remove timed out IPs until we're below the threshold.
		for( auto i = m_IPTree.FirstInorder(); m_IPTree.Count() >= static_cast<unsigned int>( floor( MAX_TREE_SIZE / 1.5 ) ) && i != m_IPTree.InvalidIndex(); )
		{
			auto& ip = m_IPTree[ i ];

			if( ( curTime - ip.lastTime ) > FLUSH_TIMEOUT && ip.ip != *reinterpret_cast<int*>( adr.ip ) )
			{
				auto next = m_IPTree.NextInorder( i );
				m_IPTree.RemoveAt( i );
				i = next;
			}
			else
			{
				i = m_IPTree.NextInorder( i );
			}
		}
	}

	iprate_t findEntry;
	findEntry.count = 0;
	findEntry.lastTime = 0;
	findEntry.ip = *reinterpret_cast<int*>( adr.ip );

	auto i = m_IPTree.Find( findEntry );

	if( i == m_IPTree.InvalidIndex() )
	{
		iprate_t newEntry;
		newEntry.count = 1;
		newEntry.lastTime = curTime;
		newEntry.ip = *reinterpret_cast<int*>( adr.ip );

		m_IPTree.Insert( newEntry );
	}
	else
	{
		auto& ip = m_IPTree[ i ];

		++ip.count;

		if( ( curTime - ip.lastTime ) > max_queries_window.value )
		{
			ip.lastTime = curTime;
			ip.count = 1;
		}
		else
		{
			if( ip.count / max_queries_window.value > max_queries_sec.value )
				return false;
		}
	}

	++m_iGlobalCount;

	if( ( curTime - m_lLastTime ) <= max_queries_window.value )
	{
		return m_iGlobalCount / max_queries_window.value <= max_queries_sec_global.value;
	}
	else
	{
		m_iGlobalCount = 1;
		m_lLastTime = curTime;
		return true;
	}
}
