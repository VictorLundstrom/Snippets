#pragma once
#include "tgCV3D.h"
#include "tgCSphere.h"

#include <tgMemoryDisable.h>
#include <vector>
#include <tgMemoryEnable.h>

class  CPlayer;
struct SRewindModel;

class CEnemy
{
public:
	// Constructor / Destructor
	CEnemy(CPlayer* pPlayer);
	~CEnemy(void);

	//////////////////////////////////////////////////////////////////////////

	void	Update(const tgFloat DeltaTime);
	void	HandleCollision(const tgFloat DeltaTime);

	tgCV3D GetPos() const { return m_Position; }
	tgUInt32 GetState() const { return (tgUInt32)m_State; }
	SRewindModel* GetRewindModel() { return m_pRewindModel; }

	enum EState
	{
		STATE_PATROL,
		STATE_CHASE,
		STATE_ATTACK,

		NUM_STATES

	};	// EState

	struct SNode
	{
		tgCV3D Pos;
	};

private:
	void InitPredefinedPatrolNodes();

	tgBool CheckGrounded(const tgCWorld& rCollisionWorld);

	void UpdatePathfindingBools();
	void ResetPathfindingBools();

	void ExitState(EState State);
	void EnterState(EState State);

	void DoPatrol();
	void DoChase();

	void SetAttackHitbox();

	//Rewind
	void RewindRestoreState(); //Because we are only updating the Matrix in the RewindManager we also have to update the Enemy's Pos and Rot variables after we completed the Rewind Effect.

	//////////////////////////////////////////////////////////////////////////

	tgCModel*	  m_pModel;
	SRewindModel* m_pRewindModel;

	tgCV3D m_MovementDirection;
	tgCV3D m_Position;
	tgCV3D m_Rotation;
	tgCV3D m_Velocity;

	tgFloat m_Speed;
	tgFloat m_Radius;
	tgFloat m_SlopeThreshold;
	tgFloat m_AttackTime;

	EState m_State;
	EState m_LastState;

	tgBool m_Grounded;

	//Pathfinding
	tgCV3D		  m_Goal;
	const tgFloat m_NodeRange;
	const tgFloat m_DetectRadius;
	const tgFloat m_AttackRange;

	std::vector<SNode> m_PredefinedPatrolNodes;
	std::vector<SNode> m_PathNodes;

	//Pathfinding Bools
	tgBool m_ReachedGoal;
	tgBool m_SpottedPlayer;
	tgBool m_InAttackRange;
	tgBool m_DetectedNoise;

	CPlayer*	m_pPlayer;

	tgCSphere	  m_AttackHitbox;
	const tgFloat m_MaxAttackRadius;

};