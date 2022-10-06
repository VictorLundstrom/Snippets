#pragma once

#include "Specialization/SPoint.h"
#include "Specialization/EAnimationState.h"

#include "tgLog.h"

#include <tgMemoryDisable.h>
#include <vector>
#include <deque>
#include <tgMemoryEnable.h>

class CPathfinding;
class CPlayer;
struct SPoint;

namespace bt
{
   enum Status //Return statuses for behaviors.
   {
      BH_INVALID, //Default value
      BH_SUCCESS,
      BH_FAILURE,
      BH_RUNNING,
      BH_ABORTED
   };

   // ----------------------------------------------------------------------------

   class Behavior //Base class for actions, conditions and composites.
   {
   public:
      Behavior() : m_eStatus(BH_INVALID) {}

      virtual ~Behavior() {}

      virtual void OnInitialize() {}
      virtual Status Update() = 0;
      virtual void OnTerminate(Status) {}

      Status Tick() //Wrapper that calls, Init, Update, Terminate. Also Returns a status: Running, Success or Failure
      {
         if (m_eStatus != BH_RUNNING)
            OnInitialize();

         m_eStatus = Update();

         if (m_eStatus != BH_RUNNING)
            OnTerminate(m_eStatus);

         return m_eStatus;
      }

      bool IsRunning() const { return m_eStatus == BH_RUNNING; }
      Status GetStatus() const { return m_eStatus; }
      tgCString GetName() const { return m_Name; }
      void SetName(const tgCString &rName) { m_Name = rName; }

      void Abort() //Called when Behavior gets interrupted to perform cleanup.
      {
         OnTerminate(BH_ABORTED);
         m_eStatus = BH_ABORTED;
      }

   private:
      Status m_eStatus;
      tgCString m_Name;
   };

   // ----------------------------------------------------------------------------

   class BehaviorTree
   {
   public:
      BehaviorTree(Behavior* pRoot) //Constructor
      {
         m_pRoot = pRoot;
      }

      void UpdateTree()
      {
         m_pRoot->Tick();
      }

   protected:
      Behavior* m_pRoot;
   };

   // ----------------------------------------------------------------------------

   // //Decorator
   //class Decorator : public Behavior
   //{
   //public:
   //   Decorator(Behavior* pChild) : m_pChild(pChild) {}
   //protected:
   //   Behavior* m_pChild;
   //};

   //Composite
   class Composite : public Behavior
   {
   public:
      void AddChild(Behavior* child) { m_Children.push_back(child); }
      /*void RemoveChild(Behavior*);
      void ClearChildren();*/
   protected:
      typedef std::vector<Behavior*> Behaviors;
      Behaviors m_Children;
   private:
      void OnTerminate(Status status) override
      {
         if (status == Status::BH_ABORTED)
         {
            for (Behaviors::iterator it = m_Children.begin(); it != m_Children.end(); ++it)
            {
               Behavior& b = **it;
               if (b.IsRunning())
               {
                  b.Abort();
               }
            }
         }
      }
   };

   //Sequence
   class Sequence : public Composite
   {
   protected:
      virtual void OnInitialize() override
      {
         m_CurrentChild = m_Children.begin();
      }
      virtual Status Update() override
      {
         //Keep going until a child behavior says it's running
         while (true)
         {
            Status s = (*m_CurrentChild)->Tick();

            //If child fails or keeps running, return
            if (s != BH_SUCCESS) return s;

            //Move on until we hit the end of the array.
            if (++m_CurrentChild == m_Children.end())
               return BH_SUCCESS;
         }
         return BH_INVALID; //Unexpected loop exit
      }

      Behaviors::iterator m_CurrentChild;
   };

   //Filter
   class Filter : public Sequence
   {
      void AddCondition(Behavior* condition)
      {
         m_Children.insert(m_Children.begin(), condition);
      }
      void AddAction(Behavior* action)
      {
         m_Children.push_back(action);
      }
   };

   //Selectors
   class Selector : public Composite
   {
   protected:
      virtual void OnInitialize()
      {
         m_CurrentChild = m_Children.begin();
      }
      virtual Status Update()
      {
         //Keep going until a child behavior says it's running.
         while (true)
         {
            Status s = (*m_CurrentChild)->Tick();

            //if child succeeds or keeps running, return.
            if (s != BH_FAILURE) return s;

            //Continue search for fallback until the last child.
            if (++m_CurrentChild == m_Children.end())
               return BH_FAILURE;
         }
         return BH_INVALID; //Unexpected loop exit.
      }

      Behaviors::iterator m_CurrentChild;

   };

   //ActiveSelector
   class ActiveSelector : public Selector 
   {
   public:
      ActiveSelector(const tgCString& rName)
      {
         SetName(rName);
      }
   protected:
      
      virtual void OnInitialize()
      {
         m_CurrentChild = m_Children.end();
      }

      virtual Status Update()
      {
         Behaviors::iterator previous = m_CurrentChild;
         Selector::OnInitialize(); //Set m_CurrentChild to first child
         Status result = Selector::Update(); //Tick children starting with the first. If child returns Success or Running, select that one.

         if (previous != m_Children.end() && m_CurrentChild != previous) //(If iterator is in bounds) AND (if m_CurrentChild HAS changed since last update)
            (*previous)->Abort(); //Terminate our previous child. (Has to Terminate everything under this child aswell)

         return result;
      }
   };

   //ActiveSequence
   class ActiveSequence : public Sequence
   {
   public:
      ActiveSequence(const tgCString& rName)
      {
         SetName(rName);
      }

      virtual Status Update() 
      {
        m_CurrentChild = m_Children.begin();
        Status status = Sequence::Update();

        //DEBUG
        if (status == BH_RUNNING)
        {
           tgCString name = GetName();
           tgLogMessage(name + "\n");
        }

        return status;


      }
   };

   //////////////////////////////////// Custom Behaviors ////////////////////////////////////

   //Conditions
   class ConditionPlayerVisible : public Behavior
   {
   public:
      ConditionPlayerVisible(const tgCString& rName, tgBool *pPlayerVisible)
      : m_pPlayerVisible(pPlayerVisible)
      {
         SetName(rName);
      }
   private:
      Status Update() override;

      tgBool* m_pPlayerVisible;
   };

   class ConditionPlayerInRange : public Behavior
   {
   public:
      ConditionPlayerInRange(const tgCString& rName, tgBool* pInShootRange)
         : m_pInShootRange(pInShootRange)
      {
         SetName(rName);
      }
   private:
      Status Update() override;

      tgBool* m_pInShootRange;
   };

   class ConditionHaveSuspectedLocation : public Behavior
   {
   public:
      ConditionHaveSuspectedLocation(const tgCString& rName, tgBool* pHasSuspectedLocation)
         :
         m_pHasSuspectedLocation(pHasSuspectedLocation)
      {
         SetName(rName);
      }
   private:
      Status Update() override;

      tgBool* m_pHasSuspectedLocation;
   };

  
   //Actions
   class ActionFireAtPlayer : public Behavior
   {
   public:
      ActionFireAtPlayer(const tgCString& rName, EAnimationState *pState ):
         m_pState(pState)
      {
         SetName(rName);
      }
   private:
      void OnInitialize() override;

      Status Update() override;

      EAnimationState* m_pState;
   };

   class ActionMoveToPlayer : public Behavior
   {
   public:
      ActionMoveToPlayer(const tgCString& rName, CPathfinding* pPathfinder, tgBool* pReachedGoal, tgCV3D* pPlayerPos, tgBool* pQueueIsSet, std::deque<tgCV3D>* pPathQueue, tgBool* pHasSuspectedLocation, tgCV3D* pLastKnownPlayerPos)
      : 
         m_pPathfinder(pPathfinder),
         m_pReachedGoal(pReachedGoal),
         m_pPlayerPos(pPlayerPos),
         m_pQueueIsSet(pQueueIsSet),
         m_pPathQueue(pPathQueue),
         m_pHasSuspectedLocation(pHasSuspectedLocation),
         m_pLastKnownPlayerPos(pLastKnownPlayerPos)
      {
         SetName(rName);
      }
   private:
      Status Update() override;
      void OnTerminate(Status) override;

      CPathfinding* m_pPathfinder;
      tgBool* m_pReachedGoal;
      tgCV3D* m_pPlayerPos;
      tgBool* m_pQueueIsSet;
      std::deque<tgCV3D>* m_pPathQueue;
      tgBool* m_pHasSuspectedLocation;
      tgCV3D* m_pLastKnownPlayerPos;
   };

   class ActionMoveToLastKnownPos : public Behavior
   {
   public:
      ActionMoveToLastKnownPos(const tgCString& rName , CPathfinding* pPathfinder, tgBool* pReachedGoal, tgBool* pQueueIsSet, std::deque<tgCV3D>* pPathQueue, tgBool* pHasSuspectedLocation, tgCV3D* pLastKnownPlayerPos) :
         m_pPathfinder(pPathfinder),
         m_pReachedGoal(pReachedGoal),
         m_pQueueIsSet(pQueueIsSet),
         m_pPathQueue(pPathQueue),
         m_pHasSuspectedLocation(pHasSuspectedLocation),
         m_pLastKnownPlayerPos(pLastKnownPlayerPos),
         m_pLastKnownPlayerGoal(NULL)
      {
         SetName(rName);
      }
   private:
      void OnInitialize() override;

      Status Update() override;

      void OnTerminate(Status) override;

      CPathfinding* m_pPathfinder;
      tgBool* m_pReachedGoal;
      tgBool* m_pQueueIsSet;
      std::deque<tgCV3D>* m_pPathQueue;
      tgBool* m_pHasSuspectedLocation;
      tgCV3D* m_pLastKnownPlayerPos;
      SPoint* m_pLastKnownPlayerGoal;
   };

   class ActionMoveToRandomPos : public Behavior
   {
   public:
      ActionMoveToRandomPos(const tgCString& rName, CPathfinding* pPathfinder, tgBool* pReachedGoal, tgBool* pQueueIsSet, std::deque<tgCV3D>* pPathQueue) :
         m_pPathfinder(pPathfinder),
         m_pReachedGoal(pReachedGoal),
         m_pQueueIsSet(pQueueIsSet),
         m_pPathQueue(pPathQueue),
         m_pPatrolGoal(NULL)
      {
         SetName(rName);
      }
   private:
      void OnInitialize() override;

      Status Update() override;

      void OnTerminate(Status) override;

      CPathfinding* m_pPathfinder;
      tgBool* m_pReachedGoal;
      SPoint* m_pPatrolGoal;
      tgBool* m_pQueueIsSet;
      std::deque<tgCV3D>* m_pPathQueue;
   };

   class ActionLookAround : public Behavior
   {
   public:
      ActionLookAround(const tgCString& rName, tgBool* pReachedGoal):
         m_pReachedGoal(pReachedGoal)
      {
         SetName(rName);
      }
   private:
      Status Update() override;
      void OnTerminate(Status) override;

      tgBool* m_pReachedGoal;
   };
}