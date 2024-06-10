#include "ipratelimit.h"
#include "ipratelimitWrapper.h"

static CIPRateLimit rateChecker;

int CheckIP( netadr_t adr )
{
	return rateChecker.CheckIP( adr );
}
