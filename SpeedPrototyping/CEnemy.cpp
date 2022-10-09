#include <tgSystem.h>
#include "CEnemy.h"

#include "Managers/CModelManager.h"
#include "Managers/CWorldManager.h"
#include "tgCDebugManager.h"
#include "tgCLine3D.h"
#include "tgCCollision.h"
#include "CPlayer.h"
#include "Managers/CRewindManager.h"

CEnemy::CEnemy(CPlayer* pPlayer)
: m_pModel(nullptr)
, m_pRewindModel(nullptr)
, m_MovementDirection(tgCV3D::Zero)
, m_Position(3.0f, 4.0f, 1.0f)
, m_Rotation(0.0f, 180.0f, 0.0f)
, m_Velocity(0.0f, 0.0f, 0.0f)
, m_Speed(2.0f)
, m_Radius(0.5f)
, m_SlopeThreshold(0.8f)
, m_AttackTime(0.0f)
, m_State(STATE_PATROL)
, m_LastState(STATE_PATROL)
, m_Grounded(false)
, m_Goal(tgCV3D::Zero)
, m_NodeRange(0.1f)
, m_DetectRadius(5.0f)
, m_AttackRange(2.0f)
, m_ReachedGoal(false)
, m_SpottedPlayer(false)
, m_InAttackRange(false)
, m_DetectedNoise(false)
, m_pPlayer(pPlayer)
, m_AttackHitbox(tgCV3D::Zero, 0.0f)
, m_MaxAttackRadius(3.0f)
{
	// Load model
	m_pModel = CModelManager::GetInstance().LoadModel("models/orc", "Enemy", true);
	tgCMesh* pMesh = m_pModel->GetMesh(0);
	// Loop over all submeshes
	for (tgUInt32 SubMeshIndex = 0; SubMeshIndex < pMesh->GetNumSubMeshes(); ++SubMeshIndex)
	{
		tgCMesh::SSubMesh* pSubMesh = pMesh->GetSubMesh(SubMeshIndex);
		tgCMaterial& rMaterial = pSubMesh->Material;

		rMaterial.SetColor(tgCColor::Red);
	}

	m_pModel->GetTransform().GetMatrixLocal().Scale(tgCV3D(0.01f, 0.01f, 0.01f), tgCMatrix::ECombine::COMBINE_REPLACE);
	m_pModel->GetTransform().GetMatrixLocal().RotateXYZ(m_Rotation, tgCMatrix::COMBINE_POST_MULTIPLY);
	m_pModel->GetTransform().GetMatrixLocal().Translate(m_Position, tgCMatrix::COMBINE_POST_MULTIPLY);

	m_pModel->GetTransform().Update();

	//Enable Rewind Functionality
	CRewindManager::GetInstance().AddRewindModel(m_pModel);
	m_pRewindModel = CRewindManager::GetInstance().FindRewindModel(m_pModel);

	//Setup Patrol Path
	InitPredefinedPatrolNodes();

}

CEnemy::~CEnemy(void)
{
	// Destroy model
	if (m_pModel)
		CModelManager::GetInstance().DestroyModel(m_pModel);
}

void CEnemy::Update(const tgFloat DeltaTime)
{
	//////////////////////////////////////////////////////////////////////////////////////////

	UpdatePathfindingBools();

	//////////////////////////////////////////////////////////////////////////////////////////

	if (CRewindManager::GetInstance().IsRewinding(m_pRewindModel)) //If Rewind effect is currently ongoing
		return;
	else if (CRewindManager::GetInstance().FinishedRewinding(m_pRewindModel)) //If Rewind effect has finished
		RewindRestoreState();

	switch (m_State)
	{
		case STATE_PATROL:
		{
			if (m_SpottedPlayer || m_DetectedNoise)
				m_State = STATE_CHASE;
			else
			{
				DoPatrol();
				break;
			}

			ExitState(STATE_PATROL);
			EnterState(m_State);
		}
		break;

		case STATE_CHASE:
		{
			if (!m_SpottedPlayer && !m_DetectedNoise)
				m_State = STATE_PATROL;
			else
			{
				DoChase();
				break;
			}

			ExitState(STATE_CHASE);
			EnterState(m_State);
		}
		break;

		case STATE_ATTACK:
		{
			const tgFloat	AttackLength = 1.25f;

			m_AttackTime += DeltaTime;

			tgCSphere CopyAttackHitbox = m_AttackHitbox;
			CopyAttackHitbox.SetRadius(tgMathInterpolateLinear(0.0f, m_MaxAttackRadius, m_AttackTime));
			
			tgCDebugManager::GetInstance().AddLineSphere(CopyAttackHitbox, tgCColor::Red);


			if (m_AttackTime >= AttackLength) //If attack should finish
			{
				if (CopyAttackHitbox.PointInside(m_pPlayer->GetPos())) //If player is inside attack hitbox
				{
					m_pPlayer->Die();
					m_State = STATE_PATROL;
					ExitState(STATE_ATTACK);
					EnterState(m_State);

					break;
				}

				/////////////////////////////////////////////
				
				m_AttackTime = 0.0f;

				SetAttackHitbox();

				if (!m_InAttackRange)
					m_State = STATE_CHASE;
				else
					break;

				ExitState(STATE_ATTACK);
				EnterState(m_State);
			}
		}
		break;
	}

	if (m_InAttackRange && m_State != STATE_ATTACK) //Enter attack state if within range and not currently attacking
	{
		ExitState(m_State);
		m_State = STATE_ATTACK;
		EnterState(STATE_ATTACK);
	}

	//////////////////////////////////////////////////////////////////////////////////////////

	if (m_MovementDirection.DotProduct()) //If moving
	{
		const tgCV3D MoveDir = m_MovementDirection.Normalized();

		tgFloat TargetRotation = (TG_RAD_TO_DEG(tgMathATan2(MoveDir.z, -MoveDir.x)) - 90.0f);

		while ((TargetRotation - m_Rotation.y) > 180.0f)
			TargetRotation -= 360.0f;
		while ((TargetRotation - m_Rotation.y) < -180.0f)
			TargetRotation += 360.0f;

		m_Position += m_MovementDirection * m_Speed * DeltaTime;
		m_Rotation.y = tgMathInterpolateLinear(m_Rotation.y, TargetRotation, DeltaTime * 15);
	}

	//////////////////////////////////////////////////////////////////////////////////////////

	HandleCollision(DeltaTime);

	//////////////////////////////////////////////////////////////////////////////////////////

	//Update model matrices with new Pos and Rot
	m_pModel->GetTransform().GetMatrixLocal().Scale(tgCV3D(0.01f, 0.01f, 0.01f), tgCMatrix::ECombine::COMBINE_REPLACE);
	m_pModel->GetTransform().GetMatrixLocal().RotateXYZ(m_Rotation, tgCMatrix::COMBINE_POST_MULTIPLY);
	m_pModel->GetTransform().GetMatrixLocal().Translate(m_Position, tgCMatrix::COMBINE_POST_MULTIPLY);
	m_pModel->GetTransform().Update();

	//////////////////////////////////////////////////////////////////////////////////////////

	ResetPathfindingBools();

	//////////////////////////////////////////////////////////////////////////////////////////
}

void CEnemy::HandleCollision(const tgFloat DeltaTime)
{
	const tgCWorld& rWorld = *CWorldManager::GetInstance().GetWorld("Collision");

	m_Position += m_Velocity * DeltaTime;
	m_Grounded = CheckGrounded(rWorld);

	if (!m_Grounded)
	{
		const tgFloat	Gravity = 9.81f;

		m_Velocity.y -= Gravity * DeltaTime;
	}

	//////////////////////////////////////////////////////////////////////////

	const tgCSphere	Sphere(m_Position + tgCV3D(0, m_Radius, 0), m_Radius);
	tgCCollision	Collision(true);
	Collision.SetType(tgCMesh::EType::TYPE_WORLD);

	if (Collision.SphereAllMeshesInWorld(Sphere, rWorld))
	{
		const tgCV3D	Normal = Collision.GetLocalNormal();
		const tgFloat	Slope = Normal.DotProduct(tgCV3D::PositiveY);

		if (Slope <= m_SlopeThreshold)
		{
			const tgFloat	InvFraction = (1.0f - Collision.GetFraction());
			const tgCV3D	Push = (Normal * InvFraction) * Sphere.GetRadius();
			m_Position.x += Push.x;
			m_Position.z += Push.z;
		}
	}
}

tgBool CEnemy::CheckGrounded(const tgCWorld& rCollisionWorld)
{
	if (m_Velocity.y > 0.0f)
		return false;

	//////////////////////////////////////////////////////////////////////////

	tgCLine3D		Line(m_Position + tgCV3D(0.0f, m_Radius, 0.0f), m_Position + tgCV3D(0.0f, -0.05f, 0.0f));
	tgCCollision	Collision(true);
	Collision.SetType(tgCMesh::EType::TYPE_WORLD);

	if (Collision.LineAllMeshesInWorld(Line, rCollisionWorld))
	{
		const tgFloat	Slope = Collision.GetLocalNormal().DotProduct(tgCV3D::PositiveY);

		if (Slope > m_SlopeThreshold)
		{
			m_Position = Collision.GetLocalIntersection();
			m_Velocity.y = 0.0f;

			return true;
		}
	}

	return false;
}

void CEnemy::UpdatePathfindingBools()
{
	const tgCLine3D GoalDistLine = tgCLine3D(m_Position, m_Goal);
	tgCDebugManager::GetInstance().AddLine3D(GoalDistLine, tgCColor::Green);
	if (GoalDistLine.Length() < m_NodeRange)
		m_ReachedGoal = true;

	const tgCSphere DetectSphere(m_Position, m_DetectRadius);
	tgCDebugManager::GetInstance().AddLineSphere(DetectSphere, tgCColor::Fuchsia);
	if (DetectSphere.PointInside(m_pPlayer->GetPos()))
		m_SpottedPlayer = true;

	const tgCLine3D AttackRangeLine = tgCLine3D(m_Position, m_pPlayer->GetPos());
	tgCDebugManager::GetInstance().AddLine3D(AttackRangeLine, tgCColor::Aqua);
	if (AttackRangeLine.Length() <= m_AttackRange)
		m_InAttackRange = true;

	if (m_pPlayer->IsRunning())
		m_DetectedNoise = true;
}

void CEnemy::ResetPathfindingBools()
{
	m_ReachedGoal = false;
	m_SpottedPlayer = false;
	m_InAttackRange = false;
	m_DetectedNoise = false;
}

void CEnemy::ExitState(EState State)
{
	switch (State)
	{
		case STATE_PATROL:
		{
			m_Goal = tgCV3D::Zero;
			m_PathNodes.clear();
			m_MovementDirection = tgCV3D::Zero;
		}
		break;

		case STATE_CHASE:
		{
			m_MovementDirection = tgCV3D::Zero;
		}
		break;

		case STATE_ATTACK:
		{
			m_AttackTime = 0.0f;
		}
		break;
	}

}

void CEnemy::EnterState(EState State)
{
	switch (State)
	{
	case STATE_PATROL:
	{	
		//Find Closest Start or End point in the PatrolPath, and if the end node is closest, reverse the array.
		if((m_PredefinedPatrolNodes.front().Pos - m_Position).Length() > (m_PredefinedPatrolNodes.back().Pos - m_Position).Length())
			std::reverse(m_PredefinedPatrolNodes.begin(), m_PredefinedPatrolNodes.end());

		//Make Path
		tgUInt32 ClosestPatrolNodeIndex = 0;
		tgFloat ClosestLength = TG_FLOAT_MAX;
		for(tgUInt32 i = 0; i < m_PredefinedPatrolNodes.size(); i++)
		{
			if ( (m_PredefinedPatrolNodes[i].Pos - m_Position).Length() < ClosestLength)
			{
				ClosestPatrolNodeIndex = i;
				ClosestLength = (m_PredefinedPatrolNodes[i].Pos - m_Position).Length();
			}
		}
		for (tgUInt32 j = ClosestPatrolNodeIndex; j < m_PredefinedPatrolNodes.size(); j++) //Ignore the nodes that are in front of the ClosestPatrolNode
		{
			m_PathNodes.push_back(m_PredefinedPatrolNodes[j]);
		}

		m_Goal = m_PathNodes.back().Pos;

		//Set rotation towards the first Node
		m_MovementDirection = (m_PathNodes.front().Pos - m_Position).Normalized();
	}
	break;

	case STATE_CHASE:
	{
		//Set rotation towards the player
		m_MovementDirection = (m_pPlayer->GetPos() - m_Position).Normalized();
	}
	break;

	case STATE_ATTACK:
	{
		SetAttackHitbox();
	}
	break;
	}
}

void CEnemy::DoPatrol()
{
	//If we reached target node
	if ((m_PathNodes.front().Pos - m_Position).Length() < m_NodeRange)
	{
		m_PathNodes.erase(m_PathNodes.begin());

		if (m_PathNodes.size() > 0)
		{
			//Change Rotation to target node
			m_MovementDirection = (m_PathNodes.front().Pos - m_Position).Normalized();
		}
		else
		{
			EnterState(STATE_PATROL);
		}
	}
}

void CEnemy::DoChase()
{
	//Set rotation towards the player
	m_MovementDirection = (m_pPlayer->GetPos() - m_Position).Normalized();
}

void CEnemy::InitPredefinedPatrolNodes()
{
	const SNode One { tgCV3D(2.0f, 0.03f, 6.5f) };
	const SNode Two { tgCV3D (2.15f, 0.03f, 12.09f) };
	const SNode Three { tgCV3D (8.0f, 0.03f, 12.0f) };
	const SNode Four { tgCV3D (16.0f, 0.03f, 11.0f) };
	const SNode Five { tgCV3D (24.0f, 3.19f, 10.5f) };
	const SNode Six { tgCV3D(29.0f, 3.18f, 10.35f) };

	m_PredefinedPatrolNodes.push_back(One);
	m_PredefinedPatrolNodes.push_back(Two);
	m_PredefinedPatrolNodes.push_back(Three);
	m_PredefinedPatrolNodes.push_back(Four);
	m_PredefinedPatrolNodes.push_back(Five);
	m_PredefinedPatrolNodes.push_back(Six);

	m_PathNodes = m_PredefinedPatrolNodes;

	m_MovementDirection = (m_PathNodes.front().Pos - m_Position).Normalized();
}

void CEnemy::SetAttackHitbox()
{
	m_AttackHitbox.Set(m_Position, 0.0f);
}

void CEnemy::RewindRestoreState()
{
	m_Position = m_pModel->GetTransform().GetMatrixWorld().Pos;
	m_Rotation.y = m_pModel->GetTransform().GetMatrixWorld().At.y;

	ExitState(m_State);
	ResetPathfindingBools();

	EnterState(STATE_PATROL);
	UpdatePathfindingBools();

}