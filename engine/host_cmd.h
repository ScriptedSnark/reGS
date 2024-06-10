#ifndef ENGINE_HOST_CMD_H
#define ENGINE_HOST_CMD_H

#include "GameUI/CareerDefs.h"

extern CareerStateType g_careerState;

extern bool g_iQuitCommandIssued;
extern bool g_bMajorMapChange;

void Host_InitCommands();

#endif //ENGINE_HOST_CMD_H
