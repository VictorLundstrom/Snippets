#pragma once
#include "tgCSphere.h"

#include "tgMemoryDisable.h"
#include <vector>
#include "tgMemoryEnable.h"

struct SRewindModel;
class  CRewindUserData;

class CFlag
{
public:
	CFlag(tgCString UniqueName, const tgCV3D& rPosition);
	~CFlag();

	void UpdateColor(const tgFloat DeltaTime); //Lerps the Color of this Flag to create a Shining/Blinking Effect
	void ResetColor(); //Resets the Color of this Flag, called after Rewind Effect has completed.

	//GetSet
	tgCModel* GetModel() const { return m_pModel; }
	SRewindModel* GetRewindModel() { return m_pRewindModel; }
	tgBool HitPlayer(const tgCV3D& rPlayerPos); //Checks wether the player has collided with this Flag's hitbox.
	void ApplyBuff(tgFloat* pSpeed) { *pSpeed += m_BuffSpeed; } //Simple buff that increases movement speed of the player, called when the player collides with this Flag.

	void UpdateRewind(); //Updates wether Flag is picked up or not during Rewind Effect, decided by the Flag's stored history. Called as a Userdata function in Rewind Manager

	//TimeShot
	void StoreTimeShot(); //Stores wether flag was picked up or not at the time of TimeShot. Called as a Userdata function in Rewind Manager
	void StoreRewind(const tgUInt32 RewindHistorySize); //Stores wether flag was picked up or not in a history array. Called as a Userdata function in Rewind Manager
	void LoadTimeShot(); //Sets this Flag to it's state stored in TimeShot. Called as a Userdata function in Rewind Manager

private:
	tgCModel*		m_pModel;
	SRewindModel*	m_pRewindModel;
	CRewindUserData* m_pRewindUserData;

	std::vector<tgUInt32> m_RewindHistoryArr;

	//Timeshot
	tgUInt32 m_TimeShotFlag;

	const tgFloat	m_PickupRadius;
	tgCSphere		m_PickupHitbox;
	tgFloat			m_ColorInterp;
	tgBool			m_Darken;

	//Buff
	const tgFloat m_BuffSpeed;
};

