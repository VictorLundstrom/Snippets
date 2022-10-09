#include <tgSystem.h>
#include "Managers/CRewindManager.h"

#include "CFlag.h"

#include "CClock.h"
#include "tgLog.h"

CRewindManager::CRewindManager(void) :
	m_ShouldRewind(false),
	m_IntervalLength(0.1),
	m_NextInterval(0.0),
	m_RewindSpeed(50.0f)
{
}

CRewindManager::~CRewindManager(void)
{
	for (SRewindModel* pRewindModel : m_RewindModelArray)
		delete pRewindModel;

	m_RewindModelArray.clear();
}

tgBool CRewindManager::IsRewinding(SRewindModel* pRewindModel)
{
	if (m_ShouldRewind || pRewindModel->m_DoIndividualRewind)
		return true;
	else
		return false;
}

tgBool CRewindManager::FinishedRewinding(SRewindModel* pRewindModel)
{
	if (pRewindModel->m_RewindFinished)
	{
		pRewindModel->m_RewindFinished = false;
		return true;
	}
	else
		return false;
}

void CRewindManager::StartIndividualRewind(const tgCModel* pModel)
{
	SRewindModel* pRewindModel = FindRewindModel(pModel);

	pRewindModel->m_DoIndividualRewind = true;

	m_IndividualRewindArray.push_back(pRewindModel);
}

void CRewindManager::TakeTimeShot()
{
	for (SRewindModel* pRewindModel : m_RewindModelArray)
	{
		if (pRewindModel->m_pTimeShot) //If we already had a timeshot
		{
			delete pRewindModel->m_pTimeShot;
			pRewindModel->m_pTimeShot = nullptr;
		}

		SRewindModel* pTimeShot = new SRewindModel();
		*pTimeShot = *pRewindModel;

		pRewindModel->m_pTimeShot = pTimeShot;

		StoreTimeshotUserData(pRewindModel);
	}
}

void CRewindManager::LoadTimeShot()
{
	for (SRewindModel* pRewindModel : m_RewindModelArray)
	{
		if (pRewindModel->m_pTimeShot == nullptr) //If this RewindModel does NOT have a timeshot
		{
			tgLogWarning("Timeshot was nullptr. Nothing to load \n");
			continue;
		}

		SRewindModel* pTimeShot = pRewindModel->m_pTimeShot;

		*pRewindModel = *pRewindModel->m_pTimeShot; //Replace the RewindModel values with TimeShot values
		delete pTimeShot; //Delete the Timeshot
		pRewindModel->m_pTimeShot = nullptr;

		UpdateTimeShotModels(pRewindModel);
		UpdateTimeShotUserData(pRewindModel);

		pRewindModel->m_RewindFinished = true; //Used to inform Owner classes

		tgLogMessage("Timeshot Sucessfully Loaded \n");
	}
}

void CRewindManager::ClearTimeShot()
{
	for (SRewindModel* pRewindModel : m_RewindModelArray)
	{
		if (pRewindModel->m_pTimeShot == nullptr) //If this RewindModel does NOT have a timeshot
		{
			tgLogWarning("Timeshot was nullptr. Nothing to clear \n");
			continue;
		}

		SRewindModel* pTimeShot = pRewindModel->m_pTimeShot;
		delete pTimeShot; //Delete the Timeshot
		pRewindModel->m_pTimeShot = nullptr;

		tgLogMessage("Timeshot Sucessfully Cleared \n");
	}
}

void CRewindManager::UpdateIndividualRewind(SRewindModel* pRewindModel, const tgFloat DeltaTime)
{
	tgCModel* pModel = pRewindModel->m_pModel;

	if (pRewindModel->m_MatrixArr.size() > 0) //Safety check if RewindModel has any stored matrices
	{
		if (!pRewindModel->m_HasStartInterp)
		{
			pRewindModel->m_StartInterpolation = pModel->GetTransform().GetMatrixWorld();
			pRewindModel->m_HasStartInterp = true;
			pRewindModel->m_FractionLerp = 0.0f;
		}

		pRewindModel->m_FractionLerp += DeltaTime * m_RewindSpeed;

		pModel->GetTransform().GetMatrixLocal().InterpolateLinear(pRewindModel->m_StartInterpolation, pRewindModel->m_MatrixArr.back(), pRewindModel->m_FractionLerp);
		pModel->GetTransform().Update();

		if (pRewindModel->m_FractionLerp > 1.0f)
		{
			pRewindModel->m_StartInterpolation = pModel->GetTransform().GetMatrixWorld();

			pRewindModel->m_MatrixArr.erase(pRewindModel->m_MatrixArr.end() - 1);
			pRewindModel->m_HasStartInterp = false;
		}
	}

	//If has finished Rewinding
	if (pRewindModel->m_MatrixArr.size() == 0)
	{
		EraseIndividualRewinding(pRewindModel);

		pRewindModel->m_DoIndividualRewind = false;
		pRewindModel->m_RewindFinished = true;
	}
}

void CRewindManager::Update(const tgFloat DeltaTime)
{
	tgDouble Lifetime = CClock::GetInstance().GetLifeTime();

	////////////////////////////////////////////////////////////////////////////////////

	for (SRewindModel* pRewindModel : m_IndividualRewindArray)
	{
		UpdateIndividualRewind(pRewindModel, DeltaTime);
	}

	// If Rewind Effect is NOT active
	if (!m_ShouldRewind)
	{
		if (Lifetime < m_NextInterval) //If current time has NOT reached next interval
			return;

		for (SRewindModel* pRewindModel : m_RewindModelArray)
		{
			if (pRewindModel->m_DoIndividualRewind) //If this model is marked for Individual Rewind
				continue;

			if (pRewindModel->m_MatrixArr.size() == MAX_REWIND_STORED_LOCATIONS) //If 100 updates have elapsed since Model was added (10 seconds , 0.1*100)
			{
				pRewindModel->m_MatrixArr.erase(pRewindModel->m_MatrixArr.begin());
			}
			
			pRewindModel->m_MatrixArr.push_back(pRewindModel->m_pModel->GetTransform().GetMatrixWorld());

			StoreRewindUserData(pRewindModel);
		}

		//Set next Interval
		m_NextInterval = Lifetime + m_IntervalLength;
	}
	
	//If Rewind Effect IS active
	if (m_ShouldRewind)
	{
		for (SRewindModel* pRewindModel : m_RewindModelArray)
		{
			if (pRewindModel->m_MatrixArr.size() > 0) //Safety check if RewindModel has any stored matrices
			{
				tgCModel* pModel = pRewindModel->m_pModel;

				if (!pRewindModel->m_HasStartInterp)
				{
					pRewindModel->m_StartInterpolation = pModel->GetTransform().GetMatrixWorld();
					pRewindModel->m_HasStartInterp = true;
					pRewindModel->m_FractionLerp = 0.0f;
				}

				pRewindModel->m_FractionLerp += DeltaTime * m_RewindSpeed;

				pModel->GetTransform().GetMatrixLocal().InterpolateLinear(pRewindModel->m_StartInterpolation, pRewindModel->m_MatrixArr.back(), pRewindModel->m_FractionLerp);
				pModel->GetTransform().Update();

				if (pRewindModel->m_FractionLerp > 1.0f)
				{
					pRewindModel->m_StartInterpolation = pModel->GetTransform().GetMatrixWorld();

					pRewindModel->m_MatrixArr.erase(pRewindModel->m_MatrixArr.end() - 1);
					pRewindModel->m_HasStartInterp = false;

					UpdateRewindUserData(pRewindModel);
				}
			}
		}
		//If our oldest element has finished rewinding, stop the rewind
		if (m_RewindModelArray.front()->m_MatrixArr.size() == 0)
		{
			m_ShouldRewind = false;
			for (SRewindModel* pRewindModel : m_RewindModelArray)
			{
				pRewindModel->m_RewindFinished = true;
			}
		}
	}
}

void CRewindManager::AddRewindModel(tgCModel* pModel, CRewindUserData* pUserData)
{
	if (m_RewindModelArray.size() > MAX_REWIND_MODELS)
	{
		tgLogError("Max Rewind Models Reached");
		return;
	}

	SRewindModel* pRewindModel = new SRewindModel();
	pRewindModel->m_pModel = pModel;
	pRewindModel->m_pRewindUserData = pUserData;


	m_RewindModelArray.push_back(pRewindModel);
}

void CRewindManager::EraseIndividualRewinding(const SRewindModel* pRewindModel)
{
	for (tgUInt32 i = 0; i < m_IndividualRewindArray.size(); i++)
	{
		if (m_IndividualRewindArray[i] == pRewindModel)
		{
			m_IndividualRewindArray.erase(m_IndividualRewindArray.begin() + i);
			return;
		}
	}
}

SRewindModel* CRewindManager::FindRewindModel(const tgCModel* pModel) const
{
	for (SRewindModel* pRewindModel : m_RewindModelArray)
	{
		if (pRewindModel->m_pModel == pModel)
		{
			return pRewindModel;
		}
	}
	tgLogError("Model does not exist in CRewindManager");
	return nullptr;
}

void CRewindManager::UpdateTimeShotModels(SRewindModel *pRewindModel)
{
	tgCModel* pModel = pRewindModel->m_pModel;

	pModel->GetTransform().SetMatrixLocal(pRewindModel->m_MatrixArr.back());
	pModel->GetTransform().Update();
}

void CRewindManager::StoreRewindUserData(SRewindModel* pRewindModel)
{
	if (!pRewindModel->m_pRewindUserData)
		return;

	switch (pRewindModel->m_pRewindUserData->m_RewindModelType)
	{
		case ERewindModelType::Flag:
		{
			CFlag* pFlag = (CFlag*)pRewindModel->m_pRewindUserData->m_pUserData;
			pFlag->StoreRewind(MAX_REWIND_STORED_LOCATIONS);
		}
		break;

		default:
			tgLogError("UserData ModelType not found");
	}
}

void CRewindManager::UpdateRewindUserData(SRewindModel* pRewindModel)
{
	if (!pRewindModel->m_pRewindUserData)
		return;

	switch (pRewindModel->m_pRewindUserData->m_RewindModelType)
	{
		case ERewindModelType::Flag:
		{
			CFlag* pFlag = (CFlag*)pRewindModel->m_pRewindUserData->m_pUserData;
			pFlag->UpdateRewind();
		}
		break;

		default:
			tgLogError("UserData ModelType not found");
	}

}

void CRewindManager::StoreTimeshotUserData(SRewindModel* pRewindModel)
{
	if (!pRewindModel->m_pRewindUserData)
		return;

	switch (pRewindModel->m_pRewindUserData->m_RewindModelType)
	{
		case ERewindModelType::Flag:
		{
			CFlag* pFlag = (CFlag*)pRewindModel->m_pRewindUserData->m_pUserData;
			pFlag->StoreTimeShot();
		}
		break;

		default:
			tgLogError("UserData ModelType not found");
	}
}

void CRewindManager::UpdateTimeShotUserData(SRewindModel* pRewindModel)
{
	if (!pRewindModel->m_pRewindUserData)
		return;

	switch (pRewindModel->m_pRewindUserData->m_RewindModelType)
	{
	case ERewindModelType::Flag:
	{
		CFlag* pFlag = (CFlag*)pRewindModel->m_pRewindUserData->m_pUserData;
		pFlag->LoadTimeShot();
	}
	break;

	default:
		tgLogError("UserData ModelType not found");
	}
}