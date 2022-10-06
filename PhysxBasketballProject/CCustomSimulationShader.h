#pragma once
#include "tgMemoryDisable.h"
#include "PxFiltering.h"
#include "tgMemoryEnable.h"

class PxActor;

class CCustomSimulationShader : public physx::PxSimulationFilterCallback
{
public:

	static physx::PxFilterFlags CustomSimulationFilterShader(
		physx::PxFilterObjectAttributes attributes0,
		physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1,
		physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags,
		const void* constantBlock,
		physx::PxU32 constantBlockSize)
	{
		// let triggers through
		if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
			return physx::PxFilterFlag::eDEFAULT;
		}
		// generate contacts for all that were not filtered above
		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

		// trigger the contact callback for pairs (A,B) where
		// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;

		return physx::PxFilterFlag::eDEFAULT;
	}

	virtual		physx::PxFilterFlags	pairFound(physx::PxU32 pairID,
		physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, const physx::PxActor* a0, const physx::PxShape* s0,
		physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, const physx::PxActor* a1, const physx::PxShape* s1,
		physx::PxPairFlags& pairFlags) override;

	
	virtual		void			pairLost(physx::PxU32 pairID,
		physx::PxFilterObjectAttributes attributes0,
		physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1,
		physx::PxFilterData filterData1,
		bool objectRemoved) override;

	
	virtual		bool			statusChange(physx::PxU32& pairID, physx::PxPairFlags& pairFlags, physx::PxFilterFlags& filterFlags) override;
};