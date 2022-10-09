#pragma once
#include	<tgCV3D.h>

class	 tgCModel;
struct SRewindModel;

class CDroid
{
public:
	//////////////////////////////////////////////////////////////////////////

	// Constructor / Destructor
	CDroid(void);
	~CDroid(void);

	//////////////////////////////////////////////////////////////////////////

	void Update(const tgFloat DeltaTime);

	//////////////////////////////////////////////////////////////////////////

	SRewindModel* GetRewindModel() { return m_pRewindModel; }

private:

	void MoveDroid(const tgFloat DeltaTime); //Makes the droid Fall while Rotating, used to demonstrate the Rewind Effect in a simple way.
	void RewindRestoreState(); //Because we are only updating the Matrix in the RewindManager we also have to update the droid's Pos and Rot variables after we completed the Rewind Effect.

	tgCModel*			m_pModel;
	SRewindModel*		m_pRewindModel;

	tgCV3D				m_Position;
	tgCV3D				m_Rotation;
	tgCV3D				m_Velocity;
	const tgFloat		m_MoveSpeed;
	const tgFloat		m_RotationSpeed;

};
