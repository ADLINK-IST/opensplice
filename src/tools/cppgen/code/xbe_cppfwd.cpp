/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "xbe_cppfwd.h"
#include "xbe_interface.h"
#include "xbe_root.h"

TList<be_CppFwdDecl*> * be_CppFwdDecl::sm_fwdDeclList = new TList<be_CppFwdDecl*>;
const char * const be_CppFwdDecl::keywords[3] = { "class", "struct", "class" };

void be_CppFwdDecl::Generate (be_ClientHeader& source) const
{
   ostream& os = source.Stream();
   be_Tab tab (source);

   if (m_declType == be_CppFwdDecl::INTERFACE)
   {
      if (!BE_Globals::ignore_interfaces)
      {
         be_interface::GeneratePtrAndRef (source, m_beType->ScopedName(),
                                          m_beType->LocalName());
         be_interface::GenerateVarOutAndMgr (source, m_beType->ScopedName(),
                                             m_beType->LocalName());
         be_interface_fwd::Generated (m_beType->ScopedName());
      }
   }
   else if (!BE_Globals::ignore_interfaces ||
            !m_beType->IsInterfaceDependant ())
   {
      if (BE_Globals::isocpp_new_types)
      {
        os << tab << "class " << m_beType->LocalName() << ';' << nl;
      }
      else
      {
        os << tab << keywords[m_declType] << " " << m_beType->LocalName() << ';' << nl;
      }
   }
}

void be_CppFwdDecl::Add
(
   const DeclType declType,
   be_Type * beType,
   const be_CppEnclosingScope & cppScope
)
{
   TList<be_CppFwdDecl *>::iterator iter;
   bool found = false;
   for (iter = sm_fwdDeclList->begin(); iter != sm_fwdDeclList->end(); ++iter)
   {
      const be_CppFwdDecl& fwdDecl = *(*iter);
      if (fwdDecl.m_beType->ScopedName () == beType->ScopedName())
      {
         found = true;
         break;
      }
   }
   if (!found)
   {
      sm_fwdDeclList->push_back (new be_CppFwdDecl (declType, beType, cppScope));
   }
}

void be_CppFwdDecl::Remove
(
   be_Type * beType
)
{
   TList<be_CppFwdDecl *>::iterator iter;

   for (iter = sm_fwdDeclList->begin(); iter != sm_fwdDeclList->end(); ++iter)
   {
      if ((*iter)->m_beType == beType)
      {
         sm_fwdDeclList->erase (iter);
         break;
      }
   }
}

// generate all be_CppFwdDecls created so far within cppScope

void be_CppFwdDecl::GenerateAllWithinScope
(
   be_ClientHeader & source,
   const be_CppEnclosingScope & cppScope
)
{
   TList <be_CppFwdDecl*>::iterator iter;

   for (iter = sm_fwdDeclList->begin(); iter != sm_fwdDeclList->end(); ++iter)
   {
      be_CppFwdDecl& fwdDecl = *(*iter);

      // only generate once

      if ((fwdDecl.m_generated == false) && (fwdDecl.m_cppScope == cppScope))
      {
         fwdDecl.Generate (source);
         fwdDecl.m_generated = true;
      }
   }
}

bool be_CppFwdDecl::IsAlreadyDeclared
(
   const DDS_StdString & keyword,
   const DDS_StdString & name,
   const be_CppEnclosingScope & cppScope
)
{
   TList<be_CppFwdDecl *>::iterator iter;

   for (iter = sm_fwdDeclList->begin(); iter != sm_fwdDeclList->end(); ++iter)
   {
      const be_CppFwdDecl& fwdDecl = *(*iter);

      if ( (fwdDecl.m_cppScope == cppScope) &&
           (fwdDecl.m_beType->ScopedName() == name) )
      {
         return ( (fwdDecl.m_generated) ? true : false );
      }
   }

   return false;
}
