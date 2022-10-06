#include <tgSystem.h>
#include "CCollisionCallback.h"

#include "Managers/CPhysicsManager.h"
#include "CPlayer.h"
#include "EFilterGroup.h"

#include "tgLog.h"

#include "tgMemoryDisable.h"
#include <vector>
#include "tgMemoryEnable.h"

CCollisionCallback::CCollisionCallback():
m_pPlayer(nullptr),
m_pBasketballCourt(nullptr)
{
}


void CCollisionCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	tgLogMessage("CONTACT \n");

	const physx::PxRigidActor* pPlayerActor = m_pPlayer->GetPlayerActor();

	for (physx::PxU32 i = 0; i < nbPairs; i++)
	{
		const physx::PxContactPair& cp = pairs[i];

		if (cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			//If Player is involved in collision
			if ((pairHeader.actors[0] == pPlayerActor) || (pairHeader.actors[1] == pPlayerActor))
			{
				physx::PxActor* otherActor = (pPlayerActor == pairHeader.actors[0]) ?
					pairHeader.actors[1] : pairHeader.actors[0];

				physx::PxRigidActor* otherRigidActor = static_cast<physx::PxRigidActor*>(otherActor);
				const physx::PxU32 nBShapes = otherRigidActor->getNbShapes();

				std::vector<physx::PxShape*> shapes(nBShapes);
				otherRigidActor->getShapes(&shapes[0], shapes.size());

				for (physx::PxShape* shape : shapes)
				{
					if (shape->getSimulationFilterData().word0 & FilterGroup::Enum::eGround)
					{
						if (m_pPlayer->GetController().GetCanJump() == false)
							m_pPlayer->GetController().SetCanJump(true);
						break;
					}
				}
			}
		}
	}
}

void CCollisionCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	for (physx::PxU32 i = 0; i < count; i++)
	{
		if (pairs[i].otherShape->getSimulationFilterData().word0 & FilterGroup::Enum::eBALL && pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			tgLogMessage("BALL TRIGGERED \n");
			physx::PxActor* pActor = pairs[i].otherShape->getActor();
			physx::PxRigidDynamic* pRigidDynamicActor = static_cast<physx::PxRigidDynamic*>(pActor);
			const float linearVelocityY = pRigidDynamicActor->getLinearVelocity().y;

			if (linearVelocityY < 0)
			{
				m_pPlayer->UpdateScore();
				tgLogMessage("SCORED \n");
			}
			break;
		}
	}
}