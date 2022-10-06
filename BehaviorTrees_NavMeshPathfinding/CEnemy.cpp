#include	<tgSystem.h>
#include	"CEnemy.h"

#include "GameStateMachine/CGameStates.h"
#include	"Camera/CCamera.h"
#include	"Managers/CModelManager.h"
#include	"Managers/CWorldManager.h"
#include	"CApplication.h"
#include	"CClock.h"

#include	<tgCDebugManager.h>
#include	<tgCAnimation.h>
#include	<tgCInterpolator.h>
#include	<tgCAnimationManager.h>
#include	<tgCCore.h>
#include	<tgCLine3D.h>
#include	<tgCCollision.h>
#include	<tgLog.h>


CEnemy::CEnemy(void)
	: m_pModel(NULL)
	//, m_Animations
	, m_pInterpolatorCurrent(NULL)
	, m_pInterpolatorBlend(NULL)
	, m_pInterpolatorNext(NULL)
	, m_pPathfinding(NULL)
	, m_pPreviousPoint(tgCV3D::Zero)
	, m_QueueIsSet(false)
	, m_ReachedGoal(false)
	, m_OffsetReachPoint(1.3f)
	, m_LastPos(tgCV3D::Zero)
	, m_CheckStuckTime(2.0f)
	, m_CurrCheckStuckTime(0.0f)
	, m_PathUpdateTime(1.0f)
	, m_CurrPathUpdateTime(0.0f)
	, m_pPlayer(NULL)
	, m_PlayerDetected(false)
	, m_ConditionHaveSuspectedLocation(false)
	, m_LastKnownPlayerPos(tgCV3D::Zero)
	, m_MovementDirection(tgCV3D::Zero)
	, m_Position(3.0f, 0.0f, -1.0f)
	, m_Rotation(tgCV3D::Zero)
	, m_Velocity(tgCV3D::Zero)
	, m_WalkSpeed(1.0f)
	, m_RunSpeed(5.0f)
	, m_Blend(0.0f)
	, m_Radius(0.5f)
	, m_SlopeThreshold(0.8f)
	, m_AttackTime(0.0f)
	, m_DetectionSphere(m_Position, 10.0f)
	, m_InShootRange(false)
	, m_ShootRange(7.0f)
	, m_State(STATE_IDLE)
	, m_LastState(STATE_IDLE)
	, m_IsBlending(false)
	, m_Running(false)
	, m_Grounded(false)
	, m_pBehaviorTree(NULL)
	, m_BTInterval(0.5f)
	, m_BTCurrInterval(0.0f)

{
	// Load model
	m_pModel	= CModelManager::GetInstance().LoadModel( "models/orc", "Enemy", true );

	m_pModel->GetMesh(0)->GetSubMesh(0)->Material.SetColor(tgCColor::Maroon);

	// Load animations
	m_Animations[STATE_IDLE] = tgCAnimationManager::GetInstance().Create("animations/orc_idle");
	m_Animations[ STATE_WALK ]		= tgCAnimationManager::GetInstance().Create( "animations/orc_walk" );
	m_Animations[ STATE_RUN ]		= tgCAnimationManager::GetInstance().Create( "animations/orc_run" );
	m_Animations[ STATE_ATTACK ]	= tgCAnimationManager::GetInstance().Create( "animations/orc_attack" );

	// Create interpolators
	m_pInterpolatorCurrent			= new tgCInterpolator( *m_Animations[ STATE_IDLE ] );
	m_pInterpolatorBlend			= new tgCInterpolator( *m_Animations[STATE_IDLE] );
	m_pInterpolatorNext				= new tgCInterpolator( *m_Animations[STATE_IDLE] );

	m_pPlayer = CGameStates::GetInstance().GetStateGame()->GetPlayer();

	// Create Pathfinder which also Loads in the Navmesh world
	m_pPathfinding = new CPathfinding(&m_Position);

	//Create Behavior Tree
	m_pBehaviorTree = BehaviorTreeBuilder()
		.Composite<bt::ActiveSelector>(tgCString("ActiveSelector 1 (root)")) //#1

				.Composite<bt::ActiveSequence>(tgCString("ActiveSequence 1")) //#2 - Attack the player if seen!
					.Node<bt::ConditionPlayerVisible>(tgCString("ConditionPlayerVisible"), &m_PlayerDetected).End()
					.Composite<bt::ActiveSelector>(tgCString("ActiveSelector 2")) //#3
						.Composite<bt::ActiveSequence>(tgCString("ActiveSequence 4")) //#4
							.Node<bt::ConditionPlayerInRange>(tgCString("ConditionPlayerInRange"), &m_InShootRange).End()
							.Node<bt::ActionFireAtPlayer>(tgCString("ActionFireAtPlayer"), &m_State).End()
						.End() //#4
						.Node<bt::ActionMoveToPlayer>(tgCString("ActionMoveToPlayer"), m_pPathfinding, &m_ReachedGoal, m_pPlayer->GetPosPtr(), &m_QueueIsSet, &m_PathQueue, &m_ConditionHaveSuspectedLocation, &m_LastKnownPlayerPos).End()
					.End()//#3
				.End()//#2

				.Composite<bt::ActiveSequence>(tgCString("ActiveSequence 2")) //#5 - Search near last known position
					.Node<bt::ConditionHaveSuspectedLocation>(tgCString("ConditionHaveSuspectedLocation"), &m_ConditionHaveSuspectedLocation).End()
					.Node<bt::ActionMoveToLastKnownPos>(tgCString("ActionMoveToLastKnownPos"), m_pPathfinding, &m_ReachedGoal, &m_QueueIsSet , &m_PathQueue, &m_ConditionHaveSuspectedLocation, &m_LastKnownPlayerPos).End()
					.Node<bt::ActionLookAround>(tgCString("ActionLookAround 1"), &m_ReachedGoal).End()
				.End() //#5

				.Composite<bt::ActiveSequence>(tgCString("ActiveSequence 3")) //#6 - Patrol to random location
					.Node<bt::ActionMoveToRandomPos>(tgCString("ActionMoveToRandomPos"), m_pPathfinding, &m_ReachedGoal, &m_QueueIsSet, &m_PathQueue).End()
					.Node<bt::ActionLookAround>(tgCString("ActionLookAround 2"), &m_ReachedGoal).End()
				.End() //#6

		.End() //#1
	.End();
		
}	// */ // CEnemy


CEnemy::~CEnemy( void )
{
	//Destroy Pathfinder
	if (m_pPathfinding)
		delete m_pPathfinding;

	// Destroy interpolators
	delete m_pInterpolatorCurrent;
	delete m_pInterpolatorBlend;
	delete m_pInterpolatorNext;

	// Destroy animations
	for( tgCAnimation* pAnimation : m_Animations )
	{
		if( pAnimation )
			tgCAnimationManager::GetInstance().Destroy( &pAnimation );
	}

	// Destroy model
	if( m_pModel )
		CModelManager::GetInstance().DestroyModel( m_pModel );

}	// */ // ~CEnemy


void CEnemy::SetupQueue()
{
	m_PathQueue.clear(); //Clear the pathQueue from last time

	const std::vector<SPoint*>& rPath = m_pPathfinding->GetPath();

	for (SPoint * pPoint: rPath)
		m_PathQueue.push_back(pPoint->pos);

	m_pPreviousPoint = m_pPathfinding->GetStart()->pos;

	SetSpeedToNextPoint();

	m_QueueIsSet = true;
}

void CEnemy::CheckNextPoint(const tgFloat DeltaTime)
{
	if (!m_PathQueue.empty())
	{
		tgCV3D& rTargetPoint = m_PathQueue.front();
		tgCSphere Sphere(m_Position, m_Radius);

		if (Sphere.PointInside(rTargetPoint))
			ReachedNextPoint();

		CheckIfStuck(DeltaTime);
	}
}

void CEnemy::CheckIfStuck(const tgFloat DeltaTime)
{
	if (m_CurrCheckStuckTime == 0.0f) //Set LastPos
		m_LastPos = m_Position;

	m_CurrCheckStuckTime += DeltaTime; //Increment time

	if (m_CurrCheckStuckTime >= m_CheckStuckTime) //Reset time and check if position is same for the last 0.5 seconds, if so, perform Unstuck
	{
		m_CurrCheckStuckTime = 0.0f;
		if ((m_Position - m_LastPos).Length() < 0.1f)
		{
			Unstuck();
		}
	}
}

void CEnemy::ReachedNextPoint()
{
	tgCV3D& rNextPoint = m_PathQueue.front();

	m_pPreviousPoint = rNextPoint;
	m_PathQueue.pop_front();

	if (m_PathQueue.empty())
		ReachedGoal();
	else
		SetSpeedToNextPoint();

}

void CEnemy::SetSpeedToNextPoint()
{
	tgCV3D& rNextPoint = m_PathQueue.front();

	const tgCV2D NextPoint2D = tgCV2D(rNextPoint.x, rNextPoint.z);
	const tgCV2D PrevPoint2D = tgCV2D(m_Position.x, m_Position.z);

	const tgCV2D Dir = (NextPoint2D - PrevPoint2D).Normalized();

	const float Angle = TG_RAD_TO_DEG(tgMathATan2(Dir.x, Dir.y));

	m_Rotation.y = Angle;
}

void CEnemy::ReachedGoal()
{
	m_ReachedGoal = true;
	m_QueueIsSet = false;
}

void CEnemy::CheckForPlayer()
{
	m_DetectionSphere.SetPos(m_Position);

	if (m_DetectionSphere.PointInside(m_pPlayer->GetPos()))
	{
		m_Running = true;
		m_PlayerDetected = true;

		tgCDebugManager::GetInstance().AddLineSphere(m_DetectionSphere, tgCColor::Red);
	}
	else
	{
		m_Running = false;
		m_PlayerDetected = false;

		tgCDebugManager::GetInstance().AddLineSphere(m_DetectionSphere, tgCColor::Lime);
	}
}

void CEnemy::CheckDistToPlayer()
{
	if ( (m_pPlayer->GetPos() - m_Position).Length() <= m_ShootRange )
		m_InShootRange = true;
	else
		m_InShootRange = false;
}

void CEnemy::MoveEnemy()
{
	if(m_QueueIsSet)
		m_MovementDirection = m_pModel->GetTransform().GetMatrixWorld().At;
	else
		m_MovementDirection = tgCV3D::Zero;
}

void CEnemy::Update( const tgFloat DeltaTime )
{
	if (m_pPathfinding->GetResult() == CPathfinding::EResult::PATH_FOUND)
	{
		SetupQueue();
		m_pPathfinding->ResetVariables();
	}

	CheckNextPoint(DeltaTime);
	CheckForPlayer();
	CheckDistToPlayer();

	switch( m_State )
	{
		case STATE_IDLE:
		{
			if (m_QueueIsSet)
				m_State = m_Running ? STATE_RUN : STATE_WALK;
		}
		break;

		case STATE_WALK:
		{
			MoveEnemy();

			if (!m_QueueIsSet)
				m_State = STATE_IDLE;
			else if (m_Running)
				m_State = STATE_RUN;
		}
		break;

		case STATE_RUN:
		{
			MoveEnemy();

			if (!m_QueueIsSet)
				m_State = STATE_IDLE;
			if( !m_Running )
				m_State	= STATE_WALK;
		}
		break;

		case STATE_ATTACK:
		{
			MoveEnemy();

			const tgFloat AttackLength	= 1.25f;

			m_AttackTime += DeltaTime;

			if( m_AttackTime >= AttackLength )
			{
				m_AttackTime = 0.0f;

				if (m_QueueIsSet)
					m_State = m_Running ? STATE_RUN : STATE_WALK;
				else
					m_State = STATE_IDLE;
			}
		}
		break;
	}

//////////////////////////////////////////////////////////////////////////

	if (m_MovementDirection.DotProduct()) //If Moving (Walking/Running)
	{
		tgCMatrix RotationMatrix = tgCMatrix::Identity;
		RotationMatrix.At	= m_MovementDirection.Normalized();

		RotationMatrix.OrthoNormalize();

		const tgFloat	SpeedModifier	= ( m_State == STATE_RUN ) ? m_RunSpeed : m_WalkSpeed;

		m_Position += RotationMatrix.At * SpeedModifier * DeltaTime;
	}

//////////////////////////////////////////////////////////////////////////

	HandleCollision( DeltaTime );
	HandleAnimation( DeltaTime );

//////////////////////////////////////////////////////////////////////////

	m_pModel->GetTransform().GetMatrixLocal().Scale( 0.01f, tgCMatrix::COMBINE_REPLACE );
	m_pModel->GetTransform().GetMatrixLocal().RotateXYZ( m_Rotation, tgCMatrix::COMBINE_POST_MULTIPLY );
	m_pModel->GetTransform().GetMatrixLocal().Translate( m_Position, tgCMatrix::COMBINE_POST_MULTIPLY );
	m_pModel->GetTransform().Update();

	//Update Behavior Tree
	m_BTCurrInterval += DeltaTime;
	if (m_BTCurrInterval > m_BTInterval)
	{
		m_pBehaviorTree->UpdateTree();
		m_BTCurrInterval = 0.0f;
	}

}	// */ // Update

void CEnemy::Render()
{
	m_pPathfinding->Render();
	RenderPath();
}

void CEnemy::RenderPath()
{
	const tgCSphere StartSphere = tgCSphere(m_pPathfinding->GetStart()->pos, 0.3f);
	tgCDebugManager::GetInstance().AddLineSphere(StartSphere, tgCColor::Green);

	for (tgCV3D Point : m_PathQueue)
	{
		const tgCSphere PathSphere = tgCSphere(Point, 0.3f);

		if (Point == m_pPathfinding->GetGoal()->pos)
		{
			tgCDebugManager::GetInstance().AddLineSphere(PathSphere, tgCColor::Black);
			continue;
		}

		tgCDebugManager::GetInstance().AddLineSphere(PathSphere, tgCColor::Blue);
	}
}


void CEnemy::HandleCollision( const tgFloat DeltaTime )
{
	const tgCWorld& rWorld = *CWorldManager::GetInstance().GetWorld( "Collision" );

	m_Position += m_Velocity * DeltaTime;
	m_Grounded = CheckGrounded( rWorld );

	if( !m_Grounded )
	{
		const tgFloat Gravity = 9.81f;

		m_Velocity.y -= Gravity * DeltaTime;
	}

//////////////////////////////////////////////////////////////////////////

	const tgCSphere Sphere( m_Position + tgCV3D( 0, m_Radius, 0 ), m_Radius );
	tgCCollision Collision( true );
	Collision.SetType( tgCMesh::EType::TYPE_WORLD );

	if( Collision.SphereAllMeshesInWorld( Sphere, rWorld ) )
	{
		const tgCV3D Normal = Collision.GetLocalNormal();
		const tgFloat Slope = Normal.DotProduct( tgCV3D::PositiveY );

		if( Slope <= m_SlopeThreshold )
		{
			const tgFloat InvFraction = ( 1.0f - Collision.GetFraction() );
			const tgCV3D Push	= ( Normal * InvFraction ) * Sphere.GetRadius();
			m_Position.x += Push.x;
			m_Position.z += Push.z;
		}
	}

}	// */ // HandleCollision

void CEnemy::HandleAnimation( const tgFloat DeltaTime )
{
	if( m_LastState != m_State )
	{
		if( m_IsBlending )
		{
			m_Blend = m_pInterpolatorCurrent->GetTime();
			m_pInterpolatorCurrent->SetAnimation( *m_pInterpolatorNext->GetAnimation() );
			m_pInterpolatorCurrent->SetTime( m_pInterpolatorNext->GetTime() );
		}

		m_pInterpolatorNext->SetAnimation( *m_Animations[ m_State ] );
		m_IsBlending = true;
		m_LastState	= m_State;
	}

	if( m_IsBlending )
	{
		m_pInterpolatorBlend->Blend( *m_pInterpolatorCurrent, *m_pInterpolatorNext, m_Blend, true );
		m_pModel->SetAnimationMatrices( *m_pInterpolatorBlend );
		m_Blend += DeltaTime * 3;

		if( m_Blend > 1 )
		{
			m_IsBlending = false;
			m_Blend = 0;
			m_pInterpolatorCurrent->SetAnimation( *m_pInterpolatorNext->GetAnimation() );
			m_pInterpolatorCurrent->SetTime( m_pInterpolatorNext->GetTime() );
		}
	}
	else
	{
		m_pModel->SetAnimationMatrices( *m_pInterpolatorCurrent );
	}

	m_pInterpolatorCurrent->AddTime( DeltaTime, true, true );
	m_pInterpolatorNext->AddTime( DeltaTime, true, true );

	m_pModel->Update();

}	// */ // HandleAnimation


tgBool CEnemy::CheckGrounded( const tgCWorld& rCollisionWorld )
{
	if( m_Velocity.y > 0.0f )
		return false;

//////////////////////////////////////////////////////////////////////////

	tgCLine3D Line( m_Position + tgCV3D( 0.0f, m_Radius, 0.0f ), m_Position + tgCV3D( 0.0f, -0.05f, 0.0f ) );
	tgCCollision Collision( true );
	Collision.SetType( tgCMesh::EType::TYPE_WORLD );

	if( Collision.LineAllMeshesInWorld( Line, rCollisionWorld ) )
	{
		const tgFloat Slope = Collision.GetLocalNormal().DotProduct( tgCV3D::PositiveY );

		if( Slope > m_SlopeThreshold )
		{
			m_Position = Collision.GetLocalIntersection();
			m_Velocity.y = 0.0f;

			return true;
		}
	}

	return false;

}	// */ // CheckGrounded

void CEnemy::Unstuck()
{
	m_PathQueue.push_front(m_pPreviousPoint);

	const tgCV2D CurrPos2D = tgCV2D(m_Position.x, m_Position.z);
	const tgCV2D NextPoint2D = tgCV2D(m_pPreviousPoint.x, m_pPreviousPoint.z);

	const tgCV2D Dir = (NextPoint2D - CurrPos2D).Normalized();

	const float Angle = TG_RAD_TO_DEG(tgMathATan2(Dir.x, Dir.y));

	m_Rotation.y = Angle;
}
