#pragma once
#include "tgCSingleton.h"
#include "tgCMatrix.h"

#include "tgMemoryDisable.h"
#include "PxPhysicsAPI.h"
#include "extensions/PxDefaultErrorCallback.h"
#include "extensions/PxDefaultAllocator.h"
#include <vector>
#include "tgMemoryEnable.h"

class tgCFont;
class CPlayer;
class CCollisionCallback;
class CCustomSimulationShader;

class CPhysicsManager : public tgCSingleton< CPhysicsManager >
{
public:
	CPhysicsManager();
	~CPhysicsManager();

	void InitPhysics();
	void StepPhysics(const tgFloat Deltatime);
	void RenderPhysics();
	void RenderScore(tgUInt32 RenderTargetWidth, tgUInt32 RenderTargetHeight);

	void ShootBall();

	void CleanupPhysics();

	void InitPlayer();

	void SetupFiltering(physx::PxRigidActor* pActor, physx::PxU32 filterGroup, physx::PxU32 filterMask);

	//Get & Set
	CPlayer* GetPlayer() { return m_pPlayer; }

	//Conversion
	tgCMatrix PxTransToTgMat(const physx::PxTransform& rTransform) const;
	physx::PxMat44 TgMatToPxMat(tgCMatrix TgMatrix) const;
	physx::PxTransform TgPosToPxTrans(const tgCV3D& rPos) const;

private:

	void UpdateDynamicModels();

	void CreatePlane();
	void CreateBasketballCourt();
	void CreateBasketballHoop();

	physx::PxTriangleMesh* CreateTriangleMesh(tgCModel* pModel);

	std::vector<physx::PxShape*> CreateTriangleMeshShapes(	tgCModel* pModel,
														physx::PxMaterial* pMaterial,
														bool isExclusive = false,
														physx::PxShapeFlags shapeFlags = physx::PxShapeFlag::eVISUALIZATION | physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE);


	//Callbacks
	physx::PxDefaultAllocator		m_Allocator;
	physx::PxDefaultErrorCallback	m_ErrorCallback;
	CCollisionCallback* m_pPlayerCollision;
	CCustomSimulationShader* m_pCustomSimulationShader;

	//System
	physx::PxFoundation* m_pFoundation;
	physx::PxCooking* m_pCooking;
	physx::PxPhysics* m_pPhysics;
	physx::PxPvd* m_pPvd;
	physx::PxScene* m_pScene;
	physx::PxDefaultCpuDispatcher* m_pDispatcher;

	//Materials
	physx::PxMaterial* m_pPlaneMaterial;

	//Raw Models
	tgCModel* m_pBasketballCourt;
	tgCModel* m_pBasketballHoop;
	tgCModel* m_pBasketball;

	//Cooked Models
	physx::PxTriangleMesh* m_pTriangleBasketballCourt;

	//Plane Properties
	tgCQuad* m_pPlaneQuad;

	//Actors
	physx::PxRigidActor* m_pPlaneActor;
	physx::PxRigidActor* m_pBasketballCourtActor;
	physx::PxRigidActor* m_pBasketballHoopActor;

	//Player
	CPlayer* m_pPlayer;

	//Font
	tgCFont* m_pScoreFont;
};

