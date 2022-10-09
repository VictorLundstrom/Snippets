#pragma once

#include "tgCMatrix.h"
#include "tgCModel.h"

#define	MAX_REWIND_STORED_LOCATIONS 100 //10 stored per second, which means its stored for a total of 10 seconds. (10*10)

#define	MAX_REWIND_MODELS 20

//////////////////////////////////////////////////////////////////////////

enum class ERewindModelType
{
	Flag = 0
};

class CRewindUserData //Userdata class that can optionally be passed along with a model when adding it to this rewind manager
{
public:
	CRewindUserData(ERewindModelType RewindModelType, void* pUserData) : m_RewindModelType(RewindModelType), m_pUserData(pUserData) {}
	ERewindModelType m_RewindModelType;
	void* m_pUserData;
};

struct SRewindModel	//Pointer to a model with additional properties and data to support rewinding
{
	tgCModel* m_pModel = nullptr;
	SRewindModel* m_pTimeShot = nullptr;
	CRewindUserData* m_pRewindUserData = nullptr;
	std::vector<tgCMatrix> m_MatrixArr;
	tgCMatrix m_StartInterpolation = tgCMatrix::Identity;
	tgFloat m_FractionLerp = 0.0f;
	tgBool m_HasStartInterp = false;
	tgBool m_DoIndividualRewind = false;
	tgBool m_TimeShotTaken = false;
	tgBool m_RewindFinished = false;
};

//////////////////////////////////////////////////////////////////////////

class CRewindManager : public tgCSingleton< CRewindManager >
{
public:
	// Constructor / Destructor
	CRewindManager(void);
	~CRewindManager(void);

	void AddRewindModel(tgCModel* pModel, CRewindUserData* pUserData = nullptr); //Links a model to this rewind manager, making it susceptible to rewind effect

	tgBool IsRewinding(SRewindModel* pRewindModel); //Checks wether the specified SRewindModel is currently in the rewind effect
	tgBool FinishedRewinding(SRewindModel* pRewindModel); //Checks wether the specified SRewindModel has finished it's rewind effect

	void StartRewind() { m_ShouldRewind = true; } //Rewinds ALL SRewindModel instances
	void StartIndividualRewind(const tgCModel* pModel); //Rewinds the SRewindModel that is linked to the specified model.

	void TakeTimeShot(); //Saves and stores data for ALL SRewindModel instances.
	void LoadTimeShot(); //Restores ALL SRewindModels to whatever properties they had at the stored TimeShot.
	void ClearTimeShot(); //Clears the stored TimeShot

	void Update(const tgFloat DeltaTime);

	SRewindModel* FindRewindModel(const tgCModel* pModel) const; //Returns the SRewindModel that is linked to the specified model

private:

	//Individual Rewind
	void UpdateIndividualRewind(SRewindModel* pRewindModel, const tgFloat DeltaTime); //Updates and performs Rewind Effect for the specified SRewindModel
	void EraseIndividualRewinding(const SRewindModel* pRewindModel); //Removes the specified SRewindModel from the IndividualRewindArray
	
	//Userdata Rewind
	void StoreRewindUserData(SRewindModel* pRewindModel); //Stores userdata depending on what ERewindModelType is set within the RewindUserData object
	void UpdateRewindUserData(SRewindModel* pRewindModel); //Updates userdata depending on what ERewindModelType is set within the RewindUserData object, during Rewind Effect

	//Timeshot
	void UpdateTimeShotModels(SRewindModel* pRewindModel); //Sets the matrix for the specified SRewindModel, Used in LoadTimeShot()

	//Timeshot Userdata
	void StoreTimeshotUserData(SRewindModel* pRewindModel); //Stores userdata depending on what ERewindModelType is set within the RewindUserData object, Used in TakeTimeShot()
	void UpdateTimeShotUserData(SRewindModel* pRewindModel); //Updates userdata depending on what ERewindModelType is set within the RewindUserData object, Used in LoadTimeShot()


	// Rewind Models
	std::vector<SRewindModel*> m_RewindModelArray;
	std::vector<SRewindModel*> m_IndividualRewindArray;

	tgBool m_ShouldRewind;

	const tgDouble m_IntervalLength;
	tgDouble m_NextInterval;
	const tgFloat m_RewindSpeed;
};