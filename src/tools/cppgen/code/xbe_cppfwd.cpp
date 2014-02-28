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

      if ((fwdDecl.m_generated == I_FALSE) && (fwdDecl.m_cppScope == cppScope))
      {
         fwdDecl.Generate (source);
         fwdDecl.m_generated = I_TRUE;
      }
   }
}

idl_bool be_CppFwdDecl::IsAlreadyDeclared
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
         return ( (fwdDecl.m_generated) ? I_TRUE : I_FALSE );
      }
   }

   return I_FALSE;
}
