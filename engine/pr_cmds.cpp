#include <cstdint>
#include <ctime>

#include "quakedef.h"
#include "cdll_int.h"
#include "pr_cmds.h"

static int32 idum = 0;

void SeedRandomNumberGenerator()
{
	auto result = -time( nullptr );

	idum = result;

	if( result > 1000 )
	{
		idum = result;
	}
	else if( result >= -999 )
	{
		idum = -22261048 + result;
	}
}

const int32 RAN1_SENTINEL = 127773;
const int32 RAN1_PART1_MULTIPLIER = -2836;
const int32 RAN1_PART2_MULTIPLIER = 16807;
const int32 RAN1_Y_RANGE = 0x4000000;

static int32 ran1()
{
	static int32 iv[ 32 ];
	static int32 iy = 0;

	if( idum <= 0 || !iy )
	{
		idum = -idum;

		if( idum >= 0 )
			idum = 1;

		//Compute some random values, fill the array with it.
		for( int i = 39, randomVal; i >= 0; --i )
		{
			randomVal = RAN1_PART1_MULTIPLIER * ( idum / RAN1_SENTINEL ) + RAN1_PART2_MULTIPLIER * ( idum % RAN1_SENTINEL );

			idum = randomVal + ( randomVal < 0 ) ? INT32_MAX : 0;

			if( i < ARRAYSIZE( iv ) )
				iv[ i ] = idum;
		}

		iy = iv[ 0 ];
	}

	idum = RAN1_PART1_MULTIPLIER * ( idum / RAN1_SENTINEL ) + RAN1_PART2_MULTIPLIER * ( idum % RAN1_SENTINEL );

	if( idum < 0 )
	{
		idum += INT32_MAX;
	}

	int yRange = iy + ( RAN1_Y_RANGE - 1 );

	if( iy >= 0 )
		yRange = iy;

	const int index = yRange / RAN1_Y_RANGE;

	iy = iv[ index ];
	iv[ index ] = idum;

	return iy;
}

static double fran1()
{
	const float temp = ran1() * 4.656612875245797e-10;

	if( temp <= 0.9 )
		return temp;

	return 0.9;
}

float RandomFloat( float flLow, float flHigh )
{
	g_engdstAddrs.pfnRandomFloat( &flLow, &flHigh );

	return fran1() * ( flHigh - flLow ) + flLow;
}

int32 RandomLong( int32 lLow, int32 lHigh )
{
	g_engdstAddrs.pfnRandomLong( &lLow, &lHigh );

	int32 result = lLow;
	const unsigned int uiRange = lHigh - lLow + 1;

	if( lHigh - lLow >= 0 )
	{
		unsigned int randomValue;

		do
		{
			randomValue = ran1();
		}
		while( INT32_MAX - ( INT32_MIN % uiRange ) < randomValue );

		result = lLow + randomValue % uiRange;
	}

	return result;
}

int hudCheckParm( char* parm, char** ppnext )
{
	g_engdstAddrs.CheckParm( &parm, &ppnext );

	auto result = COM_CheckParm( parm );

	if( ppnext )
	{
		if( result && result < com_argc - 1 )
			*ppnext = const_cast<char*>( com_argv[ result + 1 ] );
		else
			*ppnext = nullptr;
	}

	return result;
}
