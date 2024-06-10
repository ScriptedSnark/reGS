#ifndef COMMON_RANDOM_H
#define COMMON_RANDOM_H

#include "tier0/platform.h"

void SeedRandomNumberGenerator();

float RandomFloat( float flLow, float flHigh );

int32 RandomLong( int32 lLow, int32 lHigh );

#endif //COMMON_RANDOM_H
