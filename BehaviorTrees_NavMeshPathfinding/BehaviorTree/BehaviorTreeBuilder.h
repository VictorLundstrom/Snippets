#pragma once
#include "LeafBuilder.h"
#include "CompositeBuilder.h"
#include "BehaviorTree.h"

   class BehaviorTreeBuilder
   {
   public:
      BehaviorTreeBuilder(void)
         : m_pRoot(nullptr)
      { }
      ~BehaviorTreeBuilder(void) = default;

      // Leaf node construction
      template <class node_type, typename... param_types>
      LeafBuilder<BehaviorTreeBuilder, node_type> Node(param_types... params)
      {
         // Allocate node and set it as root
         node_type* pNode = new node_type(params...);
         m_pRoot = pNode;

         // Return leaf builder object
         return LeafBuilder<BehaviorTreeBuilder, node_type>(this, pNode);
      }

      // Composite node construction
      template <class composite_type, typename... param_types>
      CompositeBuilder<BehaviorTreeBuilder> Composite(param_types... params)
      {
         // Allocate node and set it as root
         composite_type* pNode = new composite_type(params...);
         m_pRoot = pNode;

         return CompositeBuilder<BehaviorTreeBuilder>(this, pNode);
      }

      // Ends construction and returns the behavior tree
      bt::BehaviorTree* End(void)
      {
         return new bt::BehaviorTree(m_pRoot);
      }

   private:
      bt::Behavior* m_pRoot;
   };
