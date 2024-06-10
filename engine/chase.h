#ifndef ENGINE_CHASE_H
#define ENGINE_CHASE_H

extern cvar_t chase_active;

void Chase_Init();

void Chase_Reset();

void TraceLine( vec_t* start, vec_t* end, vec_t* impact );

void Chase_Update();

#endif //ENGINE_CHASE_H
