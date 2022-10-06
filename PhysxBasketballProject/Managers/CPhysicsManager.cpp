#include <tgSystem.h>

#include "Managers/CPhysicsManager.h"
#include "Managers/CModelManager.h"
#include "CApplication.h"
#include "Camera/CCamera.h"
#include "CPlayer.h"
#include "CCollisionCallback.h"
#include "CCustomSimulationShader.h"
#include "EFilterGroup.h"

#include "tgLog.h"
#include "tgCFontManager.h"
#include "tgCQuadManager.h"

#include "tgMemoryDisable.h"
#include "cooking/PxCooking.h"
#include "tgMemoryEnable.h"

CPhysicsManager::CPhysicsManager():
	m_pPlayerCollision(nullptr),
	m_pCustomSimulationShader(nullptr),
	m_pFoundation(nullptr),
	m_pCooking(nullptr),
	m_pPhysics(nullptr),
	m_pPvd(nullptr),
	m_pScene(nullptr),
	m_pDispatcher(nullptr),
	m_pPlaneMaterial(nullptr),
	m_pBasketballCourt(nullptr),
	m_pBasketballHoop(nullptr),
	m_pBasketball(nullptr),
	m_pTriangleBasketballCourt(nullptr),
	m_pPlaneQuad(nullptr),
	m_pPlaneActor(nullptr),
	m_pBasketballCourtActor(nullptr),
	m_pBasketballHoopActor(nullptr),
	m_pPlayer(nullptr),
	m_pScoreFont(nullptr)

{
	//Load Font
	m_pScoreFont = tgCFontManager::GetInstance().Create("fonts/proggytiny_14_normal");

	//Load Models
	m_pBasketballCourt = CModelManager::GetInstance().LoadModel("data/models/basketball_court", "BasketballCourt", false);
	m_pBasketballHoop = CModelManager::GetInstance().LoadModel("data/models/basketball_hoop", "BasketballHoop", false);

	InitPhysics();

	CreateBasketballCourt();
	CreateBasketballHoop();
	InitPlayer();

}

CPhysicsManager::~CPhysicsManager()
{
	CleanupPhysics();
}

void CPhysicsManager::InitPhysics()
{
	//Param Variables
	m_pPlayerCollision = new CCollisionCallback();
	bool RecordMemoryAllocations = true;
	physx::PxTolerancesScale ToleranceScale = physx::PxTolerancesScale();

	//Create Foundation
	m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_Allocator, m_ErrorCallback);
	if (!m_pFoundation)
	{
		tgLogError("PxCreateFoundation failed!");
		return;
	}

	//Create Pvd
	m_pPvd = physx::PxCreatePvd(*m_pFoundation);
	physx::PxPvdTransport* pTransport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_pPvd->connect(*pTransport, physx::PxPvdInstrumentationFlag::eALL);

	//Create Physics
	m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, ToleranceScale, RecordMemoryAllocations, m_pPvd);
	if (!m_pPhysics)
	{
		tgLogError("PxCreatePhysics failed!");
		return;
	}

	//Create Scene
	physx::PxSceneDesc sceneDesc(m_pPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
	m_pDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = m_pDispatcher;
	sceneDesc.filterShader = CCustomSimulationShader::CustomSimulationFilterShader;
	sceneDesc.simulationEventCallback = m_pPlayerCollision;
	m_pScene = m_pPhysics->createScene(sceneDesc);

	//Set scenePvdflags
	physx::PxPvdSceneClient* pvdClient = m_pScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	//Create Plane Material
	m_pPlaneMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.4f);

	//Create plane
	CreatePlane();

	//Init Cooking
	m_pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxCookingParams(m_pPhysics->getTolerancesScale()));
	if (!m_pCooking)
		tgLogError("PxCreateCooking failed!");

}

void CPhysicsManager::StepPhysics(const tgFloat Deltatime)
{
	m_pScene->simulate(Deltatime);
	m_pScene->fetchResults(true);

	UpdateDynamicModels();
	m_pPlayer->UpdateCamPos();
}

void CPhysicsManager::RenderPhysics()
{
	physx::PxU32 nbActors = m_pScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC);
	if (nbActors)
	{
		std::vector<physx::PxActor*> actors(nbActors);
		m_pScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC, &actors[0], nbActors);

		for (physx::PxActor* actor : actors)
		{
			switch (actor->getType())
			{
				case physx::PxActorType::eRIGID_DYNAMIC:
				{
					tgCModel* pUserData = (tgCModel*)actor->userData;
					pUserData->Render();
					break;
				}
				case physx::PxActorType::eRIGID_STATIC:
				{
					if (actor->userData == m_pPlaneQuad) // if its the plane
					{
						tgCQuad* pUserData = (tgCQuad*)actor->userData; 
						if (pUserData)
							pUserData->Render();
					}
					else if(actor->userData) //If it's anything besides the plane
					{
						tgCModel* pUserData = (tgCModel*)actor->userData;
						if (pUserData)
							pUserData->Render();
					}
					break;
				}
				default:
				{
					tgLogError("Error at RenderPhysics() switch case");
				}
			}
		}
	}
}

void CPhysicsManager::RenderScore(tgUInt32 RenderTargetWidth, tgUInt32 RenderTargetHeight)
{
	tgCString ScoreText = tgCString("Score: %i", m_pPlayer->GetScore());

	tgCV2D SizeTextScore = tgCV2D((tgFloat)m_pScoreFont->GetTextWidth(ScoreText), (tgFloat)m_pScoreFont->GetTextHeight(ScoreText));

	const tgCV2D	RenderTargetSize = tgCV2D((tgFloat)RenderTargetWidth, (tgFloat)RenderTargetHeight);
	const tgFloat	GameX = tgMathFloor((RenderTargetSize.x * 0.5f) + SizeTextScore.x * -0.5f);
	const tgFloat	GameY = tgMathFloor((RenderTargetSize.y * 0.5f) + SizeTextScore.y * -2.5f);

	m_pScoreFont->AddText(ScoreText, tgCV2D(GameX, GameY), tgCV2D::Zero, tgCColor::White);
	m_pScoreFont->Render();
}


void CPhysicsManager::ShootBall()
{
	//Camera
	const CApplication& rApplication = CApplication::GetInstance();
	tgCMatrix TgcamMatrix = rApplication.Get3DCamera()->GetCamera()->GetTransform().GetMatrixWorld(); //Make copy of 3DCam World Matrix
	
	TgcamMatrix.Pos += (TgcamMatrix.At.Normalized()); //Add slight offset

	const physx::PxMat44 PxCamMatrix = TgMatToPxMat(TgcamMatrix);
	const physx::PxTransform PxCamTransform = physx::PxTransform(PxCamMatrix);

	physx::PxMaterial* pMaterial = m_pPhysics->createMaterial(0.9f, 0.9f, 0.4f);

	const tgCString uniqueName = tgCString("Basketball %i", (tgUInt32)m_pScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC));
	tgCModel* pModel = CModelManager::GetInstance().LoadModel("data/models/basketball.tfm", uniqueName, false);
	tgCMesh* pMesh = pModel->GetMesh(0);

	pMesh->GetTransform().GetMatrixLocal().Scale(tgCV3D(0.35f, 0.35f, 0.35f), tgCMatrix::COMBINE_REPLACE);
	pModel->Update();

	physx::PxShape* pShape = m_pPhysics->createShape(physx::PxSphereGeometry(0.17f), *pMaterial, true);

	physx::PxRigidDynamic* pBody = m_pPhysics->createRigidDynamic(PxCamTransform);
	pBody->attachShape(*pShape);

	SetupFiltering(pBody, FilterGroup::eBALL, FilterGroup::eGround | FilterGroup::ePLAYER);

	physx::PxRigidBodyExt::updateMassAndInertia(*pBody, 10.0f);
	m_pScene->addActor(*pBody);

	pBody->addForce(PxCamMatrix.column2.getXYZ()* 1.5f, physx::PxForceMode::eIMPULSE);

	//link and store the specific model instance for this actor
	pBody->userData = (void*)pModel;

}

void CPhysicsManager::CreateBasketballCourt()
{
	m_pTriangleBasketballCourt = CreateTriangleMesh(m_pBasketballCourt);

	physx::PxMaterial* pBasketballCourtMaterial = m_pPhysics->createMaterial(0.9f, 0.9f, 0.4f);

	physx::PxShape* pShape = m_pPhysics->createShape(physx::PxTriangleMeshGeometry(m_pTriangleBasketballCourt), *pBasketballCourtMaterial, true);

	physx::PxTransform Location = physx::PxTransform(m_pPlaneActor->getGlobalPose().p + physx::PxVec3(0, 2, 0));

	physx::PxRigidStatic* pStaticBasketballCourt = physx::PxCreateStatic(*m_pPhysics, Location, *pShape);
	m_pScene->addActor(*pStaticBasketballCourt);

	m_pBasketballCourtActor = static_cast<physx::PxRigidActor*>(pStaticBasketballCourt);
	SetupFiltering(m_pBasketballCourtActor, FilterGroup::Enum::eGround, FilterGroup::Enum::ePLAYER | FilterGroup::Enum::eBALL);

	physx::PxTransform ActorTransform = m_pBasketballCourtActor->getGlobalPose();

	tgCMatrix tgMatrix = PxTransToTgMat(ActorTransform);
	tgMatrix.RotateZ(90.0f, tgCMatrix::COMBINE_PRE_MULTIPLY);

	m_pBasketballCourt->GetTransform().SetMatrixLocal(tgMatrix);
	m_pBasketballCourt->GetTransform().Update();

	//link and store the specific instance of this Model for the Basketballcourt actor
	m_pBasketballCourtActor->userData = (void*)m_pBasketballCourt;
}

void CPhysicsManager::CreateBasketballHoop()
{
	physx::PxMaterial* pBasketballHoopMaterial = m_pPhysics->createMaterial(0.9f, 0.9f, 0.0f);

	std::vector<physx::PxShape*> Shapes = CreateTriangleMeshShapes(m_pBasketballHoop, pBasketballHoopMaterial, true);

	const physx::PxTransform Location = physx::PxTransform(m_pBasketballCourtActor->getGlobalPose().p + physx::PxVec3(0, 0, -6));

	physx::PxRigidStatic* pStaticBasketballHoop = m_pPhysics->createRigidStatic(Location);


	for (tgUInt32 i = 0; i < Shapes.size(); i++)
	{
		physx::PxShape* pShape = Shapes[i];
		pStaticBasketballHoop->attachShape(*pShape);
	}
	SetupFiltering(pStaticBasketballHoop, FilterGroup::eGround, FilterGroup::ePLAYER);

	//Add TriggerBox
	physx::PxBoxGeometry Box(0.2f, 0.2f, 0.2f);
	physx::PxShape* pTrigger = m_pPhysics->createShape(Box, *pBasketballHoopMaterial, true, physx::PxShapeFlag::eTRIGGER_SHAPE);
	physx::PxTransform BoxTransform(physx::PxIdentity);
	BoxTransform.p = Shapes[1]->getLocalPose().p + physx::PxVec3(0.0f, -0.4f, 0.0f); //Location is where shape[1] is(the hoop) + an offset
	pTrigger->setLocalPose(BoxTransform);

	physx::PxFilterData TriggerFilterData;
	TriggerFilterData.word0 = FilterGroup::eTRIGGER;
	TriggerFilterData.word1 = FilterGroup::eBALL;

	pTrigger->setSimulationFilterData(TriggerFilterData);
	pStaticBasketballHoop->attachShape(*pTrigger);
	//

	m_pScene->addActor(*pStaticBasketballHoop);

	m_pBasketballHoopActor = static_cast<physx::PxRigidActor*>(pStaticBasketballHoop);
	m_pPlayerCollision->SetBasketballCourtRef(m_pBasketballHoopActor);

	physx::PxTransform ActorTransform = m_pBasketballHoopActor->getGlobalPose();

	tgCMatrix tgMatrix = PxTransToTgMat(ActorTransform);
	tgMatrix.RotateZ(90.0f, tgCMatrix::COMBINE_PRE_MULTIPLY);

	m_pBasketballHoop->GetTransform().SetMatrixLocal(tgMatrix);
	m_pBasketballHoop->GetTransform().Update();

	//link and store the specific instance of this Model for the Basketballhoop actor
	m_pBasketballHoopActor->userData = (void*)m_pBasketballHoop;

}

void CPhysicsManager::UpdateDynamicModels()
{
	const physx::PxU32 NbActors = m_pScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC);

	if (NbActors)
	{
		std::vector<physx::PxActor*> Actors(NbActors);
		m_pScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, &Actors[0], NbActors);

		for (physx::PxActor* pActor : Actors)
		{
			//Convert
			const physx::PxRigidActor* pRigidActor = static_cast<physx::PxRigidActor*>(pActor);
			const physx::PxTransform ActorTransform = pRigidActor->getGlobalPose();
			const tgCMatrix tgMatrix = PxTransToTgMat(ActorTransform);

			//fetch the linked model with this actor and update it's matrix
			tgCModel* pUserData = (tgCModel*)pActor->userData;
			pUserData->GetTransform().SetMatrixLocal(tgMatrix);
			pUserData->Update();
		}
	}
}

void CPhysicsManager::CreatePlane()
{
	physx::PxRigidStatic* pGroundPlane = physx::PxCreatePlane(*m_pPhysics, physx::PxPlane(0, 1, 0, 0), *m_pPlaneMaterial);
	m_pScene->addActor(*pGroundPlane);

	m_pPlaneActor = static_cast<physx::PxRigidActor*>(pGroundPlane);
	SetupFiltering(m_pPlaneActor, FilterGroup::Enum::eGround, FilterGroup::Enum::ePLAYER);
	const physx::PxTransform ActorTransform = m_pPlaneActor->getGlobalPose();

	tgCMatrix tgMatrix = PxTransToTgMat(ActorTransform);
	tgMatrix.RotateX(90.0f, tgCMatrix::COMBINE_PRE_MULTIPLY);

	m_pPlaneQuad = tgCQuadManager::GetInstance().Create("Plane", tgMatrix.Pos, tgCV2D(100, 100), tgCColor::Blue);
	m_pPlaneQuad->GetTransform().SetMatrixLocal(tgMatrix);
	m_pPlaneQuad->GetTransform().Update();

	//link and store the specific instance of this Quad for the PlaneActor
	m_pPlaneActor->userData = (void*)m_pPlaneQuad;
}

physx::PxTriangleMesh* CPhysicsManager::CreateTriangleMesh(tgCModel* pModel)
{
	physx::PxTriangleMesh* pOutput = nullptr;

	physx::PxTriangleMeshDesc TriangleMeshDesc{};

	physx::PxDefaultMemoryOutputStream WriteBuffer;

	for (tgUInt32 i = 0; i < pModel->GetNumMeshes(); i++)
	{
		const tgCMesh* pMesh = pModel->GetMesh(i);

		TriangleMeshDesc.points.count = pMesh->GetNumTotalVertices();
		TriangleMeshDesc.points.data = pMesh->GetVertexArray();
		TriangleMeshDesc.triangles.count = pMesh->GetNumTotalTriangles();
		TriangleMeshDesc.triangles.data = pMesh->GetIndexArray();

		TriangleMeshDesc.points.stride = sizeof(tgCMesh::SVertex);
		TriangleMeshDesc.triangles.stride = 3 * sizeof(tgUInt32);
		TriangleMeshDesc.flags.set(physx::PxMeshFlag::eFLIPNORMALS);

		physx::PxTriangleMeshCookingResult::Enum Result;
		tgBool Status = m_pCooking->cookTriangleMesh(TriangleMeshDesc, WriteBuffer, &Result);
		if (!Status)
		{
			tgLogError("cooking status is false");
		}

		physx::PxDefaultMemoryInputData ReadBuffer(WriteBuffer.getData(), WriteBuffer.getSize());
		pOutput = m_pPhysics->createTriangleMesh(ReadBuffer);

		if (pOutput == nullptr)
		{
			tgCString Modelname = pModel->GetAssetName();
			tgLogError("%s Model has no mesh", &Modelname);
		}
	}
	
	return pOutput;
}

std::vector<physx::PxShape*> CPhysicsManager::CreateTriangleMeshShapes(tgCModel* pModel, physx::PxMaterial* pMaterial, bool isExclusive, physx::PxShapeFlags shapeFlags)
{
	std::vector<physx::PxShape*> Output;

	const tgSInt32 GroupID = pModel->GetGroupHierarchyIDFromName("HoopPhysics");

	for (tgUInt32 i = 0; i < pModel->GetNumMeshes(); i++)
	{
		const tgCMesh* pMesh = pModel->GetMesh(i);
		const tgUInt32 MeshID = pMesh->GetHierarchyID();
		const tgSInt32 MeshParentID = pModel->GetHierarchy(MeshID)->ParentID;
		if (MeshParentID != GroupID)
			continue;
		
		physx::PxTriangleMeshDesc TriangleMeshDesc{};
		physx::PxDefaultMemoryOutputStream WriteBuffer;

		TriangleMeshDesc.points.count = pMesh->GetNumTotalVertices();
		TriangleMeshDesc.points.data = pMesh->GetVertexArray();
		TriangleMeshDesc.triangles.count = pMesh->GetNumTotalTriangles();
		TriangleMeshDesc.triangles.data = pMesh->GetIndexArray();

		TriangleMeshDesc.points.stride = sizeof(tgCMesh::SVertex);
		TriangleMeshDesc.triangles.stride = 3 * sizeof(tgUInt32);
		TriangleMeshDesc.flags.set(physx::PxMeshFlag::eFLIPNORMALS);

		physx::PxTriangleMeshCookingResult::Enum Result;
		tgBool Status = m_pCooking->cookTriangleMesh(TriangleMeshDesc, WriteBuffer, &Result);
		if (!Status)
		{
			tgLogError("cooking status is false");
		}

		//left up at length to get scale
		tgCMatrix MeshWorldMatrix = pMesh->GetTransform().GetMatrixWorld();

		physx::PxMeshScale MeshScale( physx::PxVec3(MeshWorldMatrix.Left.Length(), MeshWorldMatrix.Up.Length(), MeshWorldMatrix.At.Length()) );

		physx::PxDefaultMemoryInputData ReadBuffer(WriteBuffer.getData(), WriteBuffer.getSize());
		physx::PxTriangleMesh* pTriangleMesh = m_pPhysics->createTriangleMesh(ReadBuffer);
		physx::PxShape* pShape = m_pPhysics->createShape(physx::PxTriangleMeshGeometry(pTriangleMesh, MeshScale), *pMaterial, isExclusive, shapeFlags);

		tgCMatrix meshLocalMatrix = pMesh->GetTransform().GetMatrixLocal();
		meshLocalMatrix.Pos *= tgCV3D(MeshScale.scale.x, MeshScale.scale.y, MeshScale.scale.z);
		physx::PxMat44 PhysxMatrix = TgMatToPxMat(meshLocalMatrix);

		physx::PxTransform ShapeTransform = physx::PxTransform(PhysxMatrix);
		pShape->setLocalPose(ShapeTransform);

		Output.push_back(pShape);
	}

	return Output;
}

void CPhysicsManager::InitPlayer()
{
	physx::PxTransform Transform = physx::PxTransform(physx::PxIdentity);
	Transform.p = m_pPlaneActor->getGlobalPose().p + physx::PxVec3(0, 20, 0);
	Transform.q = physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1));

	m_pPlayer = new CPlayer(m_pPhysics, m_pScene, Transform);

	m_pPlayerCollision->SetPlayerRef(m_pPlayer);
}

void CPhysicsManager::SetupFiltering(physx::PxRigidActor* pActor, physx::PxU32 filterGroup, physx::PxU32 filterMask)
{
	physx::PxFilterData FilterData;
	FilterData.word0 = filterGroup; // word0 = own ID
	FilterData.word1 = filterMask;  // word1 = ID mask to filter pairs that trigger a contact callback;
	const physx::PxU32 NumShapes = pActor->getNbShapes();
	std::vector<physx::PxShape*> Shapes (NumShapes);
	pActor->getShapes(&Shapes[0], NumShapes);
	for (physx::PxShape* pShape: Shapes)
	{
		pShape->setSimulationFilterData(FilterData);
	}
}


tgCMatrix CPhysicsManager::PxTransToTgMat(const physx::PxTransform& rTransform) const
{
	//Convert Physx Transform to Physx Matrix
	physx::PxMat44 pxMatrix = physx::PxMat44(rTransform);

	//Convert Physx Matrix to Tengine Matrix
	tgCMatrix tgMat = tgCMatrix::Identity;
	tgMat.Left = tgCV3D(-pxMatrix[1].x, -pxMatrix[1].y, -pxMatrix[1].z);
	tgMat.Up = tgCV3D(pxMatrix[0].x, pxMatrix[0].y, pxMatrix[0].z);
	tgMat.At = tgCV3D(pxMatrix[2].x, pxMatrix[2].y, pxMatrix[2].z);
	tgMat.Pos = tgCV3D(pxMatrix[3].x, pxMatrix[3].y, pxMatrix[3].z);

	return tgMat;
}

physx::PxMat44 CPhysicsManager::TgMatToPxMat(tgCMatrix TgMatrix) const
{
	//Convert Physx Matrix to Tengine Matrix
	physx::PxMat44 pxMat = physx::PxMat44(physx::PxIdentity);

	pxMat[0].x = TgMatrix.Left.x;
	pxMat[0].y = TgMatrix.Left.y;
	pxMat[0].z = TgMatrix.Left.z;

	pxMat[1].x = TgMatrix.Up.x;
	pxMat[1].y = TgMatrix.Up.y;
	pxMat[1].z = TgMatrix.Up.z;

	pxMat[2].x = TgMatrix.At.x;
	pxMat[2].y = TgMatrix.At.y;
	pxMat[2].z = TgMatrix.At.z;

	pxMat[3].x = TgMatrix.Pos.x;
	pxMat[3].y = TgMatrix.Pos.y;
	pxMat[3].z = TgMatrix.Pos.z;

	return pxMat;
}

physx::PxTransform CPhysicsManager::TgPosToPxTrans(const tgCV3D& rPos) const
{
	physx::PxTransform PxOutput(physx::PxIdentity);

	PxOutput.p.x = rPos.x;
	PxOutput.p.y = rPos.y;
	PxOutput.p.z = rPos.z;

	return PxOutput;
}


void CPhysicsManager::CleanupPhysics()
{
	if(m_pPlayer)
		delete m_pPlayer;

	m_pScene->release();
	m_pDispatcher->release();
	m_pPhysics->release();
	physx::PxPvdTransport* pTransport = m_pPvd->getTransport();
	m_pPvd->release();
	pTransport->release();

	m_pCooking->release();
	m_pFoundation->release();

	if (m_pPlaneQuad)
		tgCQuadManager::GetInstance().Destroy(&m_pPlaneQuad);

	delete m_pPlayerCollision;

	tgLogMessage("Cleaned up PhysX.\n");

	tgCFontManager::GetInstance().Destroy(&m_pScoreFont);
}


