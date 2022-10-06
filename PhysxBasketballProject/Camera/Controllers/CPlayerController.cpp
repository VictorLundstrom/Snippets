#include <tgSystem.h>
#include "CPlayerController.h"

#include "CApplication.h"
#include "Camera/CCamera.h"
#include "Managers/CPhysicsManager.h"

#include "CClock.h"
#include <tgMath.h>
#include <tgCPluginInput.h>

CPlayerController::CPlayerController(const tgUInt32 InputEvents, physx::PxRigidDynamic* pPlayerActor)
	: ICameraController(InputEvents)
	, m_CanJump(false)
	//, m_Input
	, m_MovementSpeed(3.0f)
	, m_MouseSensitivity(0.2f)
	, m_InvertX(-1)
	, m_InvertY(1)
	, m_MoveImpulse(tgCV3D::Zero)
	, m_pActor(pPlayerActor)
{
	// Reset input
	m_Input.Mouse = tgCV3D::Zero;
	m_Input.MoveLUF = tgCV3D::Zero;
	m_Input.MoveRDB = tgCV3D::Zero;
	m_Input.MoveFaster = 1.0f;
	m_Input.MoveSlower = 1.0f;

	// Add input listener
	tgInput::AddListener(this);
}

CPlayerController::~CPlayerController(void)
{
	// Remove input listener
	tgInput::RemoveListener(this);
}


////////////////////////////// Update //////////////////////////////
//                                                                //
//  Info:
//                                                                //
//*/////////////////////////////////////////////////////////////////
void
CPlayerController::Update(void)
{
	if (m_ActiveInput)
	{
		// Rotate
		if ( (m_Input.Mouse != tgCV3D::Zero))
		{
			Rotate(m_Input.Mouse);
		}

		//////////////////////////////////////////////////////////////////////////

		// Move
		if ((m_Input.MoveLUF != tgCV3D::Zero) || (m_Input.MoveRDB != tgCV3D::Zero))
		{
			const tgFloat	SpeedMultiplier = m_MovementSpeed * m_Input.MoveFaster * m_Input.MoveSlower;

			SetMoveImpulse((m_Input.MoveLUF + m_Input.MoveRDB) * SpeedMultiplier);
		}

		// Jump
		if (m_Input.PressedJump)
		{
			SetJumpImpulse();
			m_Input.PressedJump = false;
		}
		if (m_Input.PressedShoot)
		{
			CPhysicsManager::GetInstance().ShootBall();
			m_Input.PressedShoot = false;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	// Reset mouse
	m_Input.Mouse = tgCV3D::Zero;

	//////////////////////////////////////////////////////////////////////////

		// Update matrix
	m_Matrix.RotateXYZTranslate(m_Rotation, m_Position, tgCMatrix::COMBINE_REPLACE);

}	// */ // Update


////////////////////////////// InputEvent //////////////////////////////
//                                                                    //
//  Info:
//                                                                    //
//*/////////////////////////////////////////////////////////////////////
void
CPlayerController::InputEvent(const tgInput::EType Type, const tgInput::SEvent* pEvent)
{
	if (!m_ActiveInput)
		return;

	//////////////////////////////////////////////////////////////////////////

		// Rotate using mouse
	if (Type == tgInput::TYPE_MOUSE_MOVE_RELATIVE)
	{
		m_Input.Mouse.x += -pEvent->CurrPos.y * m_InvertX * m_MouseSensitivity;
		m_Input.Mouse.y += -pEvent->CurrPos.x * m_InvertY * m_MouseSensitivity;
	}

	//////////////////////////////////////////////////////////////////////////

	// Check for key down and ignore key repeats
	if ((Type == tgInput::TYPE_KEY_DOWN) && (pEvent->Keyboard.Repeat == 0))
	{
		if (pEvent->Keyboard.VKey == 'A')			m_Input.MoveLUF.x = 1.0f;
		if (pEvent->Keyboard.VKey == 'D')			m_Input.MoveRDB.x = -1.0f;
		if (pEvent->Keyboard.VKey == 'W')			m_Input.MoveLUF.z = 1.0f;
		if (pEvent->Keyboard.VKey == 'S')			m_Input.MoveRDB.z = -1.0f;
		if (pEvent->Keyboard.VKey == 'F')			m_Input.PressedShoot = true;
		if (pEvent->Keyboard.VKey == VK_SPACE)		m_Input.PressedJump = true;
		if (pEvent->Keyboard.VKey == VK_CONTROL)	m_Input.MoveFaster = 10.0f;
		if (pEvent->Keyboard.VKey == VK_SHIFT)		m_Input.MoveSlower = 0.1f;
	}

	// Check for key up
	if (Type == tgInput::TYPE_KEY_UP)
	{
		if (pEvent->Keyboard.VKey == 'A')			m_Input.MoveLUF.x = 0.0f;
		if (pEvent->Keyboard.VKey == 'D')			m_Input.MoveRDB.x = 0.0f;
		if (pEvent->Keyboard.VKey == 'W')			m_Input.MoveLUF.z = 0.0f;
		if (pEvent->Keyboard.VKey == 'S')			m_Input.MoveRDB.z = 0.0f;
		if (pEvent->Keyboard.VKey == VK_CONTROL)	m_Input.MoveFaster = 1.0f;
		if (pEvent->Keyboard.VKey == VK_SHIFT)		m_Input.MoveSlower = 1.0f;
	}

}	// */ // InputEvent


//////////////////////////////////////////////////////////////////////////

void CPlayerController::SetMoveImpulse(const tgCV3D& rMove)
{
	const tgFloat DeltaTime = (tgFloat)CClock::GetInstance().GetDeltaTimeReal();

	m_MoveImpulse += m_Matrix.Left * (rMove.x * DeltaTime);
	m_MoveImpulse += m_Matrix.Up * (rMove.y * DeltaTime);
	m_MoveImpulse += m_Matrix.At * (rMove.z * DeltaTime);

	m_pActor->addForce(physx::PxVec3(m_MoveImpulse.x, 0, m_MoveImpulse.z) * 10, physx::PxForceMode::eVELOCITY_CHANGE);

	m_MoveImpulse = tgCV3D::Zero;
}

void CPlayerController::SetJumpImpulse()
{
	if (m_CanJump)
	{
		m_pActor->addForce(physx::PxVec3(0, 100, 0), physx::PxForceMode::eIMPULSE);
		m_CanJump = false;
	}
}

void CPlayerController::Rotate(const tgCV3D& rRotate)
{
	m_Rotation += rRotate;
}	// */ // Rotate

void CPlayerController::SwitchCameraController(ICameraController* pSpectatorCameraController)
{
	CCamera* pCam3D = CApplication::GetInstance().Get3DCamera();

	if (pCam3D->GetCameraController() != pSpectatorCameraController)
	{
		this->SetActiveInput(false);
		pCam3D->SetCameraController(pSpectatorCameraController);
		pSpectatorCameraController->SetActiveInput(true);
	}
	else
	{
		pSpectatorCameraController->SetActiveInput(false);
		pCam3D->SetCameraController(this);
		this->SetActiveInput(true);
	}
}
