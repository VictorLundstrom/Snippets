#pragma once
#include "tgCSingleton.h"

#include "tgMemoryDisable.h"
#include <vector>
#include "tgMemoryEnable.h"

#define NrOfFlags 10

class CPlayer;
class CFlag;

class CFlagGameMode : public tgCSingleton<CFlagGameMode>
{
public:
	CFlagGameMode();
	~CFlagGameMode();

	void Update(const tgFloat DeltaTime);

private:
	void InitFlags(); //Spawns "NrOfFlags" amount of CFlags at random locations around a specific origin using Line Collision.

	void WinGame();

	void RewindRestoreState(CFlag* pFlag); //Restores the state for the specified CFlag after Rewind Effect finished

	CPlayer* m_pPlayer;

	tgBool m_WonGame;

	std::vector<CFlag*> m_FlagArrayConstant; //Total CFlags
	std::vector<CFlag*> m_FlagArray; //Active CFlags
};

