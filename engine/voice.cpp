#include "quakedef.h"
#include "voice.h"

IVoiceTweak g_VoiceTweakAPI = 
{
	&VoiceTweak_StartVoiceTweakMode,
	&VoiceTweak_EndVoiceTweakMode,
	&VoiceTweak_SetControlFloat,
	&VoiceTweak_GetControlFloat,
	&VoiceTweak_GetSpeakingVolume
};

void Voice_RegisterCvars()
{
	//TODO: implement - Solokiller
}

bool Voice_Init( const char* pCodecName, int quality )
{
	//TODO: implement - Solokiller
	return true;
}

void Voice_Deinit()
{
	//TODO: implement - Solokiller
}

int VoiceTweak_StartVoiceTweakMode()
{
	//TODO: implement - Solokiller
	return false;
}

void VoiceTweak_EndVoiceTweakMode()
{
	//TODO: implement - Solokiller
}

void VoiceTweak_SetControlFloat( VoiceTweakControl iControl, float value )
{
	//TODO: implement - Solokiller
}

float VoiceTweak_GetControlFloat( VoiceTweakControl iControl )
{
	//TODO: implement - Solokiller
	return 0;
}

int VoiceTweak_GetSpeakingVolume()
{
	//TODO: implement - Solokiller
	return 0;
}
