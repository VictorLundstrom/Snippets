#include "Camera/Controllers/ICameraController.h"

#include "PxRigidDynamic.h"

class CPlayerController : public ICameraController
{
public:

	// Constructor / Destructor
	CPlayerController(const tgUInt32 InputEvents, physx::PxRigidDynamic* pPlayerActor);
	~CPlayerController(void);

	//////////////////////////////////////////////////////////////////////////

		// Update player controller
	void	Update(void);
	void	InputEvent(const tgInput::EType Type, const tgInput::SEvent* pEvent);

	//////////////////////////////////////////////////////////////////////////

	void	SetMovementSpeed(const tgFloat Speed) { m_MovementSpeed = Speed; }
	void	SetMouseSensitivity(const tgFloat Sensitivity) { m_MouseSensitivity = Sensitivity; }

	void	SetInvertX(const tgBool InvertX) { InvertX ? m_InvertX = -1 : m_InvertX = 1; }
	void	SetInvertY(const tgBool InvertY) { InvertY ? m_InvertY = -1 : m_InvertY = 1; }

	//////////////////////////////////////////////////////////////////////////
		// Movement functions
	void	SetCanJump(const tgBool canJump) { m_CanJump = canJump; }
	const tgBool	GetCanJump() { return m_CanJump;}

	void	SetMoveImpulse(const tgCV3D& rMove);
	void	SetJumpImpulse();
	void	Rotate(const tgCV3D& rRotate);
	void	SwitchCameraController(ICameraController* pSpectatorCameraController);
	//////////////////////////////////////////////////////////////////////////

private:

	struct SInput
	{
		tgCV3D	Mouse;

		tgCV3D	MoveLUF;
		tgCV3D	MoveRDB;
		tgFloat	MoveFaster;
		tgFloat	MoveSlower;
		tgBool	PressedJump = false;
		tgBool	PressedShoot = false;
	};

	//////////////////////////////////////////////////////////////////////////

	tgBool m_CanJump;

	SInput		m_Input;

	tgFloat		m_MovementSpeed;
	tgFloat		m_MouseSensitivity;

	tgSInt16	m_InvertX;
	tgSInt16	m_InvertY;

	physx::PxRigidDynamic* m_pActor;
	tgCV3D		m_MoveImpulse;


};	// CPlayerController



