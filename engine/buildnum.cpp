#include "strtools.h"

namespace
{
/**
*	Day on which Half-Life was released.
*/
static const int RELEASE_DAY = 34995;

static int cached_build_number = 0;

static const char* const date = __DATE__;

static const int NUM_MONTHS = 12;

static const char* const mon[ NUM_MONTHS ] =
{
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

static const char mond[ NUM_MONTHS ] =
{
	31,
	28,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
	31
};
}

int build_number()
{
	if( cached_build_number )
		return cached_build_number;

	int days = 0;

	int month;

	for( month = 0; month < NUM_MONTHS; ++month )
	{
		if( !Q_strncasecmp( date, mon[ month ], 3 ) )
		{
			break;
		}

		days += mond[ month ];
	}

	//Edge case: if __DATE__ contains an unknown month string, possibly due to locale issues, then we'll return 0 instead.
	//Better than returning strange values.
	if( month >= NUM_MONTHS )
	{
		return cached_build_number;
	}

	const int iDaysInMonth = days + strtol( date + 4, nullptr, 10 ) - 1;
	const int iYear = strtol( date + 7, nullptr, 10 );
	int iTotalDays = ( signed int ) floor( ( long double ) ( iYear - 1901 ) * 365.25 ) + iDaysInMonth;

	//Adjust for leap years.
	if( !( iYear & 3 ) && ( month > 1 ) )
		iTotalDays -= 1;

	cached_build_number = iTotalDays - RELEASE_DAY;

	return cached_build_number;
}
