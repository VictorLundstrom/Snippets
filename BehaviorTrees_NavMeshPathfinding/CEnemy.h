#ifndef __CENEMY_H__
#define __CENEMY_H__

#include "CPathfinding.h"
#include "CPlayer.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeBuilder.h"
#include "EAnimationState.h"

#include	<tgCInterpolator.h>
#include <tgCSphere.h>

#include	<tgMemoryDisable.h>
#include <deque>
#include	<tgMemoryEnable.h>

class CEnemy
{
public:

	// Constructor / Destructor
	CEnemy( void );
	~CEnemy( void );

//////////////////////////////////////////////////////////////////////////

	//Pathfinding
	void SetupQueue();
	void CheckNextPoint(const tgFloat DeltaTime);
	void CheckIfStuck(const tgFloat DeltaTime);
	void ReachedNextPoint();
	void SetSpeedToNextPoint();
	void ReachedGoal();
	//
	void	CheckForPlayer();
	void  CheckDistToPlayer();
	void  MoveEnemy();
	void	Update			( const tgFloat DeltaTime );
	void	Render();
	void	HandleCollision	( const tgFloat DeltaTime );
	void	HandleAnimation	( const tgFloat DeltaTime );

	tgCV3D GetPos() const { return m_Position; }
	CPathfinding* GetPathfinder() const { return m_pPathfinding; }



//////////////////////////////////////////////////////////////////////////

private:

	void RenderPath();
	tgBool	CheckGrounded	( const tgCWorld& rCollisionWorld );
	void Unstuck();

//////////////////////////////////////////////////////////////////////////

	tgCModel*			m_pModel;
	tgCAnimation*		m_Animations[ EAnimationState::NUM_STATES ];
	tgCInterpolator*	m_pInterpolatorCurrent;
	tgCInterpolator*	m_pInterpolatorBlend;
	tgCInterpolator*	m_pInterpolatorNext;

	//Pathfinding
	CPathfinding*		m_pPathfinding;
	tgCV3D				m_pPreviousPoint;
	std::deque<tgCV3D> m_PathQueue;
	tgBool				m_QueueIsSet;
	tgBool				m_ReachedGoal;
	tgFloat				m_OffsetReachPoint;
	tgCV3D				m_LastPos;
	const tgFloat		m_CheckStuckTime;
	tgFloat				m_CurrCheckStuckTime;
	const tgFloat		m_PathUpdateTime;
	tgFloat				m_CurrPathUpdateTime;

	CPlayer*				m_pPlayer;
	tgBool				m_PlayerDetected;
	tgBool				m_ConditionHaveSuspectedLocation;
	tgCV3D				m_LastKnownPlayerPos;

	tgCV3D				m_MovementDirection;
	tgCV3D				m_Position;
	tgCV3D				m_Rotation;
	tgCV3D				m_Velocity;

	tgFloat				m_WalkSpeed;
	tgFloat				m_RunSpeed;
	tgFloat				m_Blend;
	tgFloat				m_Radius;
	tgFloat				m_SlopeThreshold;
	tgFloat				m_AttackTime;

	tgCSphere			m_DetectionSphere;
	const tgFloat		m_ShootRange;
	tgBool				m_InShootRange;

	EAnimationState	m_State;
	EAnimationState	m_LastState;

	tgBool				m_IsBlending;
	tgBool				m_Running;
	tgBool				m_Grounded;

	//Behavior Tree
	bt::BehaviorTree*	m_pBehaviorTree;
	const tgFloat		m_BTInterval;
	tgFloat				m_BTCurrInterval;

};	// CEnemy

#endif // __CENEMY_H__
