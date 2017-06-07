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
#include "idl.h"
#include "idl_extern.h"
#include "xbe_globals.h"
#include "xbe_literals.h"
#include "xbe_module.h"
#include "xbe_interface.h"
#include "xbe_utils.h"
#include "xbe_scopestack.h"
#include "xbe_cppfwd.h"
#include "xbe_genlist.h"

// -------------------------------------------------
//  BE_MODULE IMPLEMENTATION
// -------------------------------------------------

IMPL_NARROW_METHODS2(be_module, AST_Module, be_CodeGenerator)
IMPL_NARROW_FROM_DECL(be_module)
IMPL_NARROW_FROM_SCOPE(be_module)

be_module::be_module ()
{
   isAtModuleScope (pbfalse);
}

be_module::be_module (UTL_ScopedName * n, const UTL_Pragmas & p)
:
   AST_Decl (AST_Decl::NT_module, n, p),
   UTL_Scope (AST_Decl::NT_module, n, p),
   m_cppScope (g_feScopeStack.Top (), *n)
{
   isAtModuleScope (pbfalse);
}

void be_module::Generate (be_ClientHeader& source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   UTL_ScopeActiveIterator * i;
   AST_Decl * d;

   os << tab << "namespace" << " " << *local_name () << nl;
   os << tab << "{" << endl;

   source.Indent ();

   g_cppScopeStack.Push (m_cppScope);

   be_CppFwdDecl::GenerateAllWithinScope (source, m_cppScope);

   i = new UTL_ScopeActiveIterator (this, UTL_Scope::IK_decls);

   while (!(i->is_done ()))
   {
      be_CodeGenerator * cg;

      d = i->item ();

      if (!d->imported () && (cg = (be_CodeGenerator*)
                              d->narrow((long) & be_CodeGenerator::type_id)))
      {
         cg->isAtModuleScope (pbtrue);
         cg->Generate (source);
      }

      i->next();
   }

   delete i;

   g_cppScopeStack.Pop ();

   source.Outdent();
   os << tab << "}";
   os << nl;
}

void be_module::Generate (be_ServerImplementation& source)
{
   be_CodeGenerator::Generate (source);
}

void be_module::Generate (be_ClientImplementation& impl)
{
   be_CodeGenerator::Generate (impl);
}

void be_module::Generate (be_ServerHeader & source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   UTL_ScopeActiveIterator * i;
   AST_Decl * d;
   static int depth = 0;

   os << nl << tab << "namespace ";

   if (depth++ == 0)
   {
      os << DDSPOAImplPrefix;
   }

   os << * local_name () << nl;
   os << tab << "{" << nl;
   source.Indent();

   // Generate POA_module class declaration
   UTL_Scope * s = (UTL_Scope *) narrow ((long) & UTL_Scope::type_id);
   if (s)
   {
      i = new UTL_ScopeActiveIterator (s, UTL_Scope::IK_decls);
      while (!i->is_done ())
      {
         be_CodeGenerator * cg;
         d = i->item ();
         if (!d->imported () &&
             (cg = (be_CodeGenerator *) d->narrow ((long) & be_CodeGenerator::type_id)))
         {
            cg->isAtModuleScope(pbtrue);
            cg->Generate (source);
         }
         i->next ();
      }
      delete i;
   }

   depth--;
   source.Outdent();
   os << tab << "}";
   os << nl;
}
