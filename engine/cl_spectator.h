#ifndef ENGINE_CL_SPECTATOR_H
#define ENGINE_CL_SPECTATOR_H

#include "render.h"

typedef struct overviewInfo_s
{
	vec3_t origin;
	float z_min;
	float z_max;
	float zoom;
	bool rotated;
} overviewInfo_t;

extern overviewInfo_t gDevOverview;
extern local_state_t spectatorState;

extern cvar_t dev_overview;

int CL_IsSpectateOnly();

bool CL_IsDevOverviewMode();

bool CL_AddEntityToPhysList( int entIndex );

void CL_MoveSpectatorCamera();

void CL_SetDevOverView( refdef_t* refdef );

#endif //ENGINE_CL_SPECTATOR_H
