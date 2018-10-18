/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
