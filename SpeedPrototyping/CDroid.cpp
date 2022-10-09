#include <tgSystem.h>
#include "CDroid.h"

#include "tgCModel.h"
#include "Managers/CRewindManager.h"
#include "Managers/CModelManager.h"


CDroid::CDroid(void):
	m_pModel(nullptr),
	m_pRewindModel(nullptr),
	m_Position(3.0f, 4.0f, -1.0f),
	m_Rotation(tgCV3D::Zero),
	m_Velocity(0.0f, 9.82f, 0.0f),
	m_MoveSpeed(0.5f),
	m_RotationSpeed(50.0f)
{
	// Load model
	m_pModel = CModelManager::GetInstance().LoadModel("models/droid_big", "Droid", true);

	// Add to Rewind Manager, making it susceptible to Rewind Effect
	CRewindManager::GetInstance().AddRewindModel(m_pModel);
	m_pRewindModel = CRewindManager::GetInstance().FindRewindModel(m_pModel);
}

CDroid::~CDroid(void)
{
	// Destroy model
	if (m_pModel)
		CModelManager::GetInstance().DestroyModel(m_pModel);
}

void CDroid::Update(const tgFloat DeltaTime)
{
	MoveDroid(DeltaTime);
}

void CDroid::MoveDroid(const tgFloat DeltaTime)
{
	if (CRewindManager::GetInstance().IsRewinding(m_pRewindModel)) //If Rewinding effect is ongoing for this SRewindModel
		return;
	else if (CRewindManager::GetInstance().FinishedRewinding(m_pRewindModel)) //If Rewind effect finished for this SRewindModel
		RewindRestoreState();

	m_Position.y -= m_MoveSpeed * DeltaTime;

	m_Rotation.y += m_RotationSpeed * DeltaTime;
	if (m_Rotation.y > 360)
		m_Rotation.y = 0;

	m_pModel->GetTransform().GetMatrixLocal().RotateXYZTranslate(m_Rotation, m_Position, tgCMatrix::COMBINE_REPLACE);
	m_pModel->GetTransform().Update();
}

void CDroid::RewindRestoreState() 
{
	m_Position = m_pModel->GetTransform().GetMatrixWorld().Pos;
	m_Rotation.y = m_pModel->GetTransform().GetMatrixWorld().At.y;
}
