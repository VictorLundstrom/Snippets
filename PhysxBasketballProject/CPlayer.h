#pragma once
#include "Camera/Controllers/CPlayerController.h"

class tgCModel;

class CPlayer
{
public:
	CPlayer(physx::PxPhysics* pPxPhysics, physx::PxScene* pPxScene, physx::PxTransform SpawnTransform);
	~CPlayer();

	void UpdateCamPos();

	void UpdateScore() { m_Score++; }
	const tgUInt32 GetScore() { return m_Score; }

	physx::PxRigidDynamic* GetPlayerActor() { return m_pPlayerActor; }
	CPlayerController& GetController() { return *m_pPlayerController; }
private:
	physx::PxMaterial* m_pMaterial;
	physx::PxShape* m_pShape;
	physx::PxCapsuleGeometry m_pCapsule;

	//Capsule properties
	const physx::PxReal m_CapsuleHalfHeight;
	const physx::PxReal m_CapsuleRadius;

	CPlayerController* m_pPlayerController;

	physx::PxRigidDynamic* m_pPlayerActor;

	tgCModel* m_pModel;

	tgUInt32 m_Score;
};