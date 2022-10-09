#include <tgSystem.h>
#include "CFlag.h"

#include "Managers/CModelManager.h"
#include "Managers/CRewindManager.h"

#include "tgCDebugManager.h"

CFlag::CFlag(tgCString UniqueName, const tgCV3D& Position) :
m_pModel(nullptr),
m_pRewindModel(nullptr),
m_pRewindUserData(nullptr),
m_TimeShotFlag(tgCMesh::FLAG_NONE),
m_PickupRadius(1.0f),
m_PickupHitbox(Position, m_PickupRadius),
m_ColorInterp(0.0f),
m_Darken(false),
m_BuffSpeed(1.0f)
{
    //Load Flag Model
    m_pModel = CModelManager::GetInstance().LoadModel("data/models/rock_01", UniqueName, true);

    m_pModel->GetTransform().GetMatrixLocal().Pos = Position;
    m_pModel->GetMesh(0)->GetSubMesh(0)->Material.SetColor(tgCColor::Silver);
    m_pModel->GetMesh(1)->SetFlags(tgCMesh::FLAG_NONE); //Not to be confused with this Flag class

    m_pModel->GetTransform().Update();

    m_pRewindUserData = new CRewindUserData(ERewindModelType::Flag, (void*)this);

    CRewindManager::GetInstance().AddRewindModel(m_pModel, m_pRewindUserData); //Add this model to Rewind Manager along with RewindUserData for more custom Rewind functionality.
    m_pRewindModel = CRewindManager::GetInstance().FindRewindModel(m_pModel);
}

CFlag::~CFlag()
{
   if (m_pRewindUserData)
      delete m_pRewindUserData;
}

void CFlag::UpdateColor(const tgFloat DeltaTime)
{
    tgCDebugManager::GetInstance().AddLineSphere(m_PickupHitbox, tgCColor::Olive);

    tgCMesh::SSubMesh* pSubMesh = m_pModel->GetMesh(0)->GetSubMesh(0);

    if (m_Darken == false)
    {
        m_ColorInterp += tgMathInterpolateLinear(0, 1, DeltaTime*0.5f);

        if (m_ColorInterp > 0.95f)
        {
           m_Darken = true;
        }
    }
    else
    {
        m_ColorInterp -= tgMathInterpolateLinear(0, 1, DeltaTime*0.5f);

        if (m_ColorInterp < 0.05f)
        {
           m_Darken = false;
        }
    }

    pSubMesh->Material.GetColor().g = m_ColorInterp;
    pSubMesh->Material.GetColor().b = m_ColorInterp;
}
void CFlag::ResetColor()
{
   m_Darken = false;
   m_ColorInterp = 0.0f;
   m_pModel->GetMesh(0)->GetSubMesh(0)->Material.SetColor(tgCColor::Silver);
}

tgBool CFlag::HitPlayer(const tgCV3D& rPlayerPos)
{
   if (m_PickupHitbox.PointInside(rPlayerPos))
      return true;

   return false;
}

void CFlag::StoreTimeShot()
{
   m_TimeShotFlag = m_pModel->GetMesh(0)->GetFlags();
}
void CFlag::StoreRewind(const tgUInt32 RewindHistorySize)
{
   if (m_RewindHistoryArr.size() == RewindHistorySize) 
   {
      m_RewindHistoryArr.erase(m_RewindHistoryArr.begin());
   }

   m_RewindHistoryArr.push_back(m_pModel->GetMesh(0)->GetFlags());
}

void CFlag::UpdateRewind()
{
  m_pModel->GetMesh(0)->SetFlags(m_RewindHistoryArr.back());
  m_RewindHistoryArr.erase(m_RewindHistoryArr.end() - 1);
}

void CFlag::LoadTimeShot()
{
   m_pModel->GetMesh(0)->SetFlags(m_TimeShotFlag);
}
