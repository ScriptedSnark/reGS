#ifndef ENGINE_VOICE_H
#define ENGINE_VOICE_H

#include "ivoicetweak.h"

extern IVoiceTweak g_VoiceTweakAPI;

void Voice_RegisterCvars();

bool Voice_Init( const char* pCodecName, int quality );
void Voice_Deinit();

int VoiceTweak_StartVoiceTweakMode();

void VoiceTweak_EndVoiceTweakMode();

void VoiceTweak_SetControlFloat( VoiceTweakControl iControl, float value );

float VoiceTweak_GetControlFloat( VoiceTweakControl iControl );

int VoiceTweak_GetSpeakingVolume();

#endif //ENGINE_VOICE_H
