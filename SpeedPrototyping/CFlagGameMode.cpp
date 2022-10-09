#include <tgSystem.h>
#include "CFlagGameMode.h"

#include "GameStateMachine/CGameStates.h"
#include "CLevel.h"
#include "CPlayer.h"

#include "tgCCollision.h"
#include "tgCLine3D.h"

#include "tgLog.h"
#include "Managers/CModelManager.h"
#include "Managers/CRewindManager.h"
#include "Managers/CWorldManager.h"
#include "tgCDebugManager.h"
#include "CFlag.h"

CFlagGameMode::CFlagGameMode():
m_pPlayer(nullptr),
m_WonGame(false)
{
	CLevel* pLevel = CGameStates::GetInstance().GetStateGame()->GetLevel();
	m_pPlayer = pLevel->GetPlayer();

	InitFlags();
}

CFlagGameMode::~CFlagGameMode()
{
	for (CFlag* pFlag : m_FlagArrayConstant)
		delete pFlag;
}

void CFlagGameMode::Update(const tgFloat DeltaTime)
{
	for (CFlag* pFlag : m_FlagArrayConstant)
	{
		if (CRewindManager::GetInstance().IsRewinding(pFlag->GetRewindModel())) //If Rewind effect is currently ongoing
			return;

		//Finished Rewinding
		if (CRewindManager::GetInstance().FinishedRewinding(pFlag->GetRewindModel())) //If Rewind effect has finished
			RewindRestoreState(pFlag);
	}

	for (tgUInt32 i = 0; i < m_FlagArray.size(); i++)
	{
		if (m_FlagArray[i]->HitPlayer(m_pPlayer->GetPos()))
		{
			m_FlagArray[i]->GetModel()->GetMesh(0)->SetFlags(tgCMesh::FLAG_NONE);
			m_FlagArray[i]->ApplyBuff(m_pPlayer->GetSpeed());
			m_FlagArray.erase(m_FlagArray.begin() + i);
			i--;
			continue;
		}

		m_FlagArray[i]->UpdateColor(DeltaTime);
	}

	if (m_FlagArray.size() == 0 && !m_WonGame)
	{
		WinGame();
		m_WonGame = true;
	}
}

void CFlagGameMode::InitFlags()
{
	tgCV3D FlagPositions[NrOfFlags];

	const tgCV3D OriginSpawn(3.0f, 0.0f, -1.0f);
	const tgCV3D YOffset(0.0f, 0.5f, 0.0f);

	const tgCMesh* pFloorWorld = &CWorldManager::GetInstance().GetWorld("Collision")->GetSector(0)->pMeshArray[4];

	tgCCollision CollisionSpawn(true);
	CollisionSpawn.SetType(tgCMesh::EType::TYPE_WORLD);

	for (tgUInt32 i = 0; i < NrOfFlags; i++)
	{
		const tgCV3D RandVec = tgCV3D(tgMathRandom(-20, 20), 0.0f, tgMathRandom(-20, 20));

		const tgCV3D SpawnPos = OriginSpawn + RandVec;
		const tgCV3D LineStart = SpawnPos - YOffset;
		const tgCV3D LineEnd = SpawnPos + YOffset;

		tgCLine3D Line(LineEnd, LineStart);

		if (CollisionSpawn.LineAllMeshesInWorld(Line, *CWorldManager::GetInstance().GetWorld("Collision")) && CollisionSpawn.GetMesh() == pFloorWorld) //If Line intersected with the specified world as well as the specific mesh within that world.
			FlagPositions[i] = CollisionSpawn.GetLocalIntersection();
		else
			i--;

		CollisionSpawn.Clear();
		CollisionSpawn.SetType(tgCMesh::TYPE_WORLD);
	}

	//////////////////////////////////////////////////////

	for (tgUInt32 i = 0; i < NrOfFlags; i++)
	{
		tgCString UniqueName = tgCString("Flag%i", i);

		m_FlagArray.push_back(new CFlag(UniqueName, FlagPositions[i]));
	}

	m_FlagArrayConstant = m_FlagArray; //Store a copy
}

void CFlagGameMode::RewindRestoreState(CFlag* pFlag)
{
	pFlag->ResetColor();

	for (CFlag* pFlagActive : m_FlagArray)
	{
		if (pFlagActive == pFlag) //If this flag was never picked up by the player during the history of the Rewind Effect
			return;
	}

	if (pFlag->GetModel()->GetMesh(0)->GetFlags() & tgCMesh::CREATEFLAG_EVERYTHING) //If flag was picked up by the player during the history of the Rewind Effect
		m_FlagArray.push_back(pFlag);
}

void CFlagGameMode::WinGame()
{
	tgLogMessage("Player Wins! \n");
}