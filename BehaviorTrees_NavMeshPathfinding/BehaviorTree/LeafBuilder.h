#pragma once

   template <class parent_type, class behavior_type>
   class LeafBuilder
   {
   public:
      // Constructor
      LeafBuilder(parent_type* pParent, behavior_type* pLeafBehavior)
         : m_pParent(pParent)
         , m_pLeafBehavior(pLeafBehavior)
      { }

      ~LeafBuilder(void) = default;

      // Calls a member function of the underlying behavior for any number of parameters
      template <typename ret, typename obj, typename... param_types>
      LeafBuilder& Invoke(ret(obj::* func)(param_types...), param_types... params)
      {
         // Dereference function pointer and pass in arguments
         m_pLeafBehavior->*func(params...);

         // Return leaf builder 
         return *this;
      }

      // When done with node, return its parent
      parent_type& End(void)
      {
         return *m_pParent;
      }

   private:
      parent_type* m_pParent;
      behavior_type* m_pLeafBehavior;
   };
