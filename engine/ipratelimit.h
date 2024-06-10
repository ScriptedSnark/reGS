#ifndef ENGINE_IPRATELIMIT_H
#define ENGINE_IPRATELIMIT_H

#include <UtlRBTree.h>

#include "netadr.h"

class CIPRateLimit
{
private:
	enum
	{
		MAX_TREE_SIZE = 512,
		START_TREE_SIZE = 256,
		FLUSH_TIMEOUT = 120,
	};

	typedef struct iprate_s
	{
		typedef int ip_t;

		ip_t ip;
		int lastTime;
		int count;
	} iprate_t;

public:
	CIPRateLimit();
	~CIPRateLimit() = default;

	bool CheckIP( netadr_t adr );

private:
	static bool LessIP( const iprate_t& lhs, const iprate_t& rhs );

private:
	CUtlRBTree<iprate_t, int> m_IPTree;
	int m_iGlobalCount = 0;
	int m_lLastTime = -1;

private:
	CIPRateLimit( const CIPRateLimit& ) = delete;
	CIPRateLimit& operator=( const CIPRateLimit& ) = delete;
};


#endif //ENGINE_IPRATELIMIT_H
