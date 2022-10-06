#pragma once
#include "tgMemoryDisable.h"
#include "PxSimulationEventCallback.h"
#include "tgMemoryEnable.h"

class CPlayer;

class CCollisionCallback : public physx::PxSimulationEventCallback
{
public:
	CCollisionCallback();

	//Collision Callbacks
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override { PX_UNUSED(constraints); PX_UNUSED(count); }
	void onWake(physx::PxActor** actors, physx::PxU32 count)override { PX_UNUSED(actors); PX_UNUSED(count); }
	void onSleep(physx::PxActor** actors, physx::PxU32 count)override { PX_UNUSED(actors); PX_UNUSED(count); }
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
	void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)override {}
	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;

	void SetPlayerRef(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	void SetBasketballCourtRef(physx::PxRigidActor* pBasketballCourt) { m_pBasketballCourt = pBasketballCourt; }

private:
	CPlayer* m_pPlayer;
	physx::PxRigidActor* m_pBasketballCourt;
	
};

