#ifndef COMMON_GAMEUI_ICAREERUI_H
#define COMMON_GAMEUI_ICAREERUI_H

#include "interface.h"

#include "CareerDefs.h"

class ICareerUI : public IBaseInterface
{
public:

	virtual bool IsPlayingMatch() = 0;

	virtual ITaskVec* GetCurrentTaskVec() = 0;

	virtual bool PlayAsCT() = 0;
	virtual int GetReputationGained() = 0;
	virtual int GetNumMapsUnlocked() = 0;
	virtual bool DoesWinUnlockAll() = 0;
	virtual int GetRoundTimeLength() = 0;
	virtual int GetWinfastLength() = 0;

	virtual CareerDifficultyType GetDifficulty() const = 0;
	virtual int GetCurrentMapTriplet( MapInfo* maps ) = 0;

	virtual void OnRoundEndMenuOpen( bool didWin ) = 0;
	virtual void OnMatchEndMenuOpen( bool didWin ) = 0;

	virtual void OnRoundEndMenuClose( bool stillPlaying ) = 0;
	virtual void OnMatchEndMenuClose( bool stillPlaying ) = 0;
};

#define CAREERUI_INTERFACE_VERSION "CareerUI001"

#endif //COMMON_GAMEUI_ICAREERUI_H
