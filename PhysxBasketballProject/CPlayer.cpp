#include <tgSystem.h>
#include "CPlayer.h"
#include "Managers/CModelManager.h"
#include "Managers/CPhysicsManager.h"
#include "EFilterGroup.h"

CPlayer::CPlayer(physx::PxPhysics* pPxPhysics, physx::PxScene* pPxScene, physx::PxTransform SpawnTransform):
	m_pMaterial(nullptr),
	m_pShape(nullptr),
	m_pCapsule(physx::PxCapsuleGeometry()),
	m_CapsuleHalfHeight (1),
	m_CapsuleRadius(0.5),
	m_pPlayerController(nullptr),
	m_pPlayerActor(nullptr),
	m_pModel(nullptr),
	m_Score(0)
{
	//Physx
	m_pMaterial = pPxPhysics->createMaterial(0.9f, 0.9f, 0.1f);
	m_pCapsule.halfHeight = m_CapsuleHalfHeight;
	m_pCapsule.radius = m_CapsuleRadius;
	m_pShape = pPxPhysics->createShape(m_pCapsule, *m_pMaterial, true);

	m_pPlayerActor = pPxPhysics->createRigidDynamic(SpawnTransform);
	m_pPlayerActor->setRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y);
	m_pPlayerActor->setMaxLinearVelocity(50);
	m_pPlayerActor->setLinearDamping(1);

	m_pPlayerActor->attachShape(*m_pShape);
	
	CPhysicsManager::GetInstance().SetupFiltering(m_pPlayerActor, FilterGroup::ePLAYER, FilterGroup::eGround | FilterGroup::eBALL);

	physx::PxRigidBodyExt::updateMassAndInertia(*m_pPlayerActor, 10.0f);

	
	//link and store the specific model instance for this actor
	m_pModel = CModelManager::GetInstance().LoadModel("data/models/basketball.tfm", "Player", false);
	m_pPlayerActor->userData = (void*)m_pModel;

	pPxScene->addActor(*m_pPlayerActor);

	//Player
	m_pPlayerController = new CPlayerController(tgInput::EType::TYPE_EVERYTHING, m_pPlayerActor);
}

CPlayer::~CPlayer()
{
	// Destroy the player camera controller
	if (m_pPlayerController)
		delete m_pPlayerController;
}

void CPlayer::UpdateCamPos()
{
	m_pPlayerController->SetPosition(m_pModel->GetTransform().GetMatrixWorld().Pos);
}