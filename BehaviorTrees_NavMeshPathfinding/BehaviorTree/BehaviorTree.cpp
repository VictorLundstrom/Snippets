#include <tgSystem.h>
#include "BehaviorTree.h"

#include "Specialization/CPathfinding.h"

using namespace bt;

/////////////////Conditions//////////////////////////

Status ConditionPlayerVisible::Update()
{
	if (*m_pPlayerVisible)
		return Status::BH_SUCCESS;
	else
		return Status::BH_FAILURE;
}

Status ConditionPlayerInRange::Update()
{
	if (*m_pInShootRange)
		return Status::BH_SUCCESS;
	else
		return Status::BH_FAILURE;
}

Status ConditionHaveSuspectedLocation::Update()
{
	if (*m_pHasSuspectedLocation)
		return Status::BH_SUCCESS;
	else
		return Status::BH_FAILURE;
}


/////////////////Actions//////////////////////////

void ActionFireAtPlayer::OnInitialize()
{
	*m_pState = EAnimationState::STATE_ATTACK;
}

//FireAtPlayer
Status ActionFireAtPlayer::Update()
{
	printf("ActionFireAtPlayer Updated \n");
	return Status::BH_SUCCESS;
}

//MoveToPlayer
Status ActionMoveToPlayer::Update()
{
	m_pPathfinder->DeterminePath(*m_pPlayerPos);

	if (*m_pReachedGoal)
		return Status::BH_SUCCESS;
	if (m_pPathfinder->GetResult() == CPathfinding::EResult::PATH_FOUND)
	{
		*m_pHasSuspectedLocation = true;
		*m_pLastKnownPlayerPos = m_pPathfinder->GetGoal()->pos;
		return Status::BH_RUNNING;
	}
	else
	{
		*m_pHasSuspectedLocation = false;
		*m_pLastKnownPlayerPos = tgCV3D::Zero;
		return Status::BH_FAILURE;
	}
}

void ActionMoveToPlayer::OnTerminate(Status)
{
	*m_pQueueIsSet = false;
}

//MoveToLastKnownPos
void ActionMoveToLastKnownPos::OnInitialize()
{
	m_pPathfinder->DeterminePath(*m_pLastKnownPlayerPos);
	m_pLastKnownPlayerGoal = m_pPathfinder->GetGoal();
}

Status ActionMoveToLastKnownPos::Update()
{
	if (*m_pReachedGoal ) //If we reached the players last known location
		return BH_SUCCESS;
	else if (m_pPathfinder->GetGoal() == m_pLastKnownPlayerGoal)//If we're currently pathing to players last known location
		return BH_RUNNING;
	else return BH_FAILURE; //If we are no longer pathing towards the players last known location
}

void ActionMoveToLastKnownPos::OnTerminate(Status)
{
	*m_pQueueIsSet = false;
	m_pLastKnownPlayerGoal = NULL;
	*m_pHasSuspectedLocation = false;
}

//MoveToRandomPos
void ActionMoveToRandomPos::OnInitialize()
{
	m_pPathfinder->DeterminePath(m_pPathfinder->RandomizeGoal());
	m_pPatrolGoal = m_pPathfinder->GetGoal();
}

Status ActionMoveToRandomPos::Update()
{
	if (*m_pReachedGoal && (m_pPathfinder->GetGoal() == m_pPatrolGoal) ) //If we reached last known goal
		return BH_SUCCESS;
	else if (m_pPathfinder->GetGoal() == m_pPatrolGoal) //If we're currently pathing to our initial goal
		return BH_RUNNING;
	else return BH_FAILURE; //If we are no longer pathing towards the goal

}

void ActionMoveToRandomPos::OnTerminate(Status)
{
	*m_pQueueIsSet = false;
	m_pPatrolGoal = NULL;
}

//LookAround
Status ActionLookAround::Update()
{
	printf("ActionLookAround Updated");
	return Status::BH_SUCCESS;
}

void ActionLookAround::OnTerminate(Status status)
{
	if (status == Status::BH_SUCCESS)
		*m_pReachedGoal = false;
}