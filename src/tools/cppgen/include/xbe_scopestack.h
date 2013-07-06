/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef XBE_SCOPESTACK_H_INCLUDED
#define XBE_SCOPESTACK_H_INCLUDED

#include "xbe_cppscope.h"

class be_CppScopeStack
{
public:

   inline void Push (const be_CppEnclosingScope & cppScope)
   {
      m_stack.push_back (cppScope);
   }

   inline void Pop ()
   {
      m_stack.pop_back ();
   }

   inline const be_CppEnclosingScope Top () const
   {
      if (m_stack.size () == 0)
      {
         return be_CppEnclosingScope ();
      }
      else
      {
         return be_CppEnclosingScope (m_stack);
      }
   }

   inline DDSString ToString () const
   {
      DDSString result;
      bool notFirst = false;
      DDSVector<be_CppEnclosingScope>::const_iterator iter;

      for (iter = m_stack.begin (); iter != m_stack.end (); ++iter)
      {
         if (notFirst)
         {
            result += BE_Globals::ScopeSeparator ();
         }
         notFirst = true;
         result += (*iter).ToString ();
      }
      return result;
   }

   inline bool IsCurrentScope (const be_CppEnclosingScope & cppScope) const
   {
      return Top().ToString () == cppScope.ToString ();
   }

private:

   DDSVector<be_CppEnclosingScope> m_stack;
};

extern be_CppScopeStack g_cppScopeStack;
extern be_CppScopeStack g_feScopeStack;

#endif
