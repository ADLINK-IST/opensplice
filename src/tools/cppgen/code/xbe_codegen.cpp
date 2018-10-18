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
#include "idl.h"
#include "idl_extern.h"
#include "xbe_codegen.h"
#include "xbe_module.h"
#include "xbe_interface.h"
#include "xbe_union.h"
#include "xbe_sequence.h"
#include "xbe_array.h"
#include "xbe_structure.h"
#include "xbe_typedef.h"

// -------------------------------------------------
//  BE_CODE_GENERATOR IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS0(be_CodeGenerator)

void
be_CodeGenerator::Generate(be_ClientHeader& source)
{
   UTL_Scope* s = (UTL_Scope*)narrow((long) & UTL_Scope::type_id);
   assert(s);

   // ITERATE THROUGH DECLS
   UTL_ScopeActiveIterator iter(s, UTL_Scope::IK_both);

   while (!(iter.is_done()))
   {
      be_CodeGenerator* cg;

      AST_Decl* d = iter.item();

      if (!d->imported() &&
          (cg = (be_CodeGenerator*)d->narrow((long) & be_CodeGenerator::type_id)))
      {
         cg->Generate(source);
      }

      iter.next();
   }
}

void
be_CodeGenerator::Generate(be_ClientImplementation& source)
{
   UTL_ScopeActiveIterator* i;
   AST_Decl * d;
   UTL_Scope * s = (UTL_Scope*)narrow((long) & UTL_Scope::type_id);

   if (s)
   {
      i = new UTL_ScopeActiveIterator(s, UTL_Scope::IK_decls);

      while (!(i->is_done()))
      {
         be_CodeGenerator * cg;

         d = i->item();

         if (!d->imported() &&
             (cg = (be_CodeGenerator*)d->narrow((long) & be_CodeGenerator::type_id)))
         {
            cg->Generate(source);
         }

         i->next();
      }

      delete i;
   }
   else
   {
      assert(pbfalse);
   }
}

void
be_CodeGenerator::Generate(be_ServerHeader& source)
{
   UTL_ScopeActiveIterator* i;
   AST_Decl * d;
   UTL_Scope * s = (UTL_Scope*)narrow((long) & UTL_Scope::type_id);

   if (s)
   {
      i = new UTL_ScopeActiveIterator(s, UTL_Scope::IK_decls);

      while (!(i->is_done()))
      {
         be_CodeGenerator * cg;

         d = i->item();

         if (!d->imported() &&
             (cg = (be_CodeGenerator*)d->narrow((long) & be_CodeGenerator::type_id)))
         {
            if (!is_local_type(d))
            {
               cg->Generate(source);
            }
         }

         i->next();
      }

      delete i;
   }
   else
   {
      assert(pbfalse);
   }
}

void
be_CodeGenerator::Generate(be_ServerImplementation& source)
{
   UTL_ScopeActiveIterator* i;
   AST_Decl * d;
   UTL_Scope * s = (UTL_Scope*)narrow((long) & UTL_Scope::type_id);

   if (s)
   {
      i = new UTL_ScopeActiveIterator(s, UTL_Scope::IK_decls);

      while (!(i->is_done()))
      {
         be_CodeGenerator * cg;

         d = i->item();

         if (!d->imported() &&
             (cg = (be_CodeGenerator*)d->narrow((long) & be_CodeGenerator::type_id)))

         {
            if (!is_local_type(d))
            {
               cg->Generate(source);
            }
         }

         i->next();
      }

      delete i;
   }
   else
   {
      assert(pbfalse);
   }
}

bool be_CodeGenerator::is_local_type (AST_Decl * d)
{
   AST_Type* ast = (AST_Type*)d->narrow((long) & AST_Type::type_id);
   return ast && ast->local();
}

void be_CodeGenerator::GenerateGlobal (be_ClientHeader& source)
{
}
