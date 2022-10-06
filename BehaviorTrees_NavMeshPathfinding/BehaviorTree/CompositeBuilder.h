#pragma once

   template <class parent_type>
   class CompositeBuilder
   {
   public:
      CompositeBuilder(parent_type* pParent, bt::Composite* pComp)
         : m_pParent(pParent)
         , m_pComposite(pComp)
      { }

      ~CompositeBuilder(void) = default;

      // Adds node to the composite's list of children
      template <class node_type, typename... param_types>
      LeafBuilder<CompositeBuilder, node_type> Node(param_types... params)
      {
         // Allocate node and add it to children
         node_type* pNode = new node_type(params...);
         m_pComposite->AddChild(pNode);

         return LeafBuilder<CompositeBuilder<parent_type>, node_type>(this, pNode);
      }

      // Adds composite node to list of children
      template <class composite_type, typename... param_types>
      CompositeBuilder<CompositeBuilder<parent_type> > Composite(param_types... params)
      {
         // Add composite to children
         composite_type* pNode = new composite_type(params...);
         m_pComposite->AddChild(pNode);

         return CompositeBuilder<CompositeBuilder>(this, pNode);
      }

      // When done with node, return its parent
      parent_type& End(void)
      {
         return *m_pParent;
      }

   private:
      parent_type* m_pParent;
      bt::Composite* m_pComposite;

   };
