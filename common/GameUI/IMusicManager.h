#ifndef COMMON_GAMEUI_IMUSICMANAGER_H
#define COMMON_GAMEUI_IMUSICMANAGER_H

#include "interface.h"

enum TrackType
{
	TRACKTYPE_INVALID = 1,

	TRACKTYPE_SPLASH,
	TRACKTYPE_ROUNDWIN,
	TRACKTYPE_ROUNDLOSE,
	TRACKTYPE_MATCHWIN,
	TRACKTYPE_MATCHLOSE,
	TRACKTYPE_CREDITS,

	TRACKTYPE_NUM
};

class IMusicManager : public IBaseInterface
{
public:
	virtual void StartTrack( TrackType track, bool looping ) = 0;
	virtual void StopTrack() = 0;
	virtual void FadeOutTrack() = 0;
	virtual bool IsPlayingTrack() = 0;
	virtual bool IsPlayingTrack( TrackType track ) = 0;
};

#define MUSICMANAGER_INTERFACE_VERSION "MusicManager001"

#endif //COMMON_GAMEUI_IMUSICMANAGER_H
