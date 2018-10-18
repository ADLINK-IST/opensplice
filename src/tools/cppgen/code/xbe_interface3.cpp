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
#include "xbe.h"
#include "xbe_literals.h"
#include "xbe_root.h"
#include "xbe_operation.h"
#include "xbe_attribute.h"
#include "xbe_interface.h"

void
be_interface::GenerateImpureImplementations(be_ClientImplementation& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   // GENERATE IMPURE IMPLEMENTATIONS
   UTL_ScopeActiveIterator * it;

   it = new UTL_ScopeActiveIterator(this, UTL_Scope::IK_decls);

   for (;!(it->is_done());it->next())
   {
      be_operation * op;
      be_attribute * at;
      AST_Decl * d;

      d = it->item();

      op = (be_operation*)d->narrow((long) & be_operation::type_id);
      if (op)
      {
         op->GenerateImpureRequestCall(source);
      }
      else if ((at = (be_attribute*)d->narrow((long) & be_attribute::type_id)))
      {
         os << tab;
         at->GenerateImpureRequestCall(source);
      }

   }

   delete it;
}

void be_interface::GenerateRequestDispatchers
(
   be_ServerImplementation & source,
   const DDS_StdString & implbasename
)
{
   Be_OpMap::iterator it;

   //
   // iterate through all supported (NOT inherited) operations
   // and attributes
   //

   for (it = m_lops.begin(); it != m_lops.end(); it++)
   {
      if (it.valid())
      {
         be_operation * op;
         be_attribute * at;
         AST_Decl * d = (AST_Decl*)(*it).m_opdecl;

         if (d)
         {
            op = be_operation::_narrow (d);
            if (op)
            {
               op->GenerateDispatcher(source, implbasename);
            }
            else if ((at = be_attribute::_narrow (d)))
            {
               if ((*it).m_opname[(unsigned)1] == 'g')   // get attribute
               {
                  at->GenerateGetDispatcher(source, implbasename);
               }
               else if ((*it).m_opname[(unsigned)1] == 's')  // set attribute
               {
                  at->GenerateSetDispatcher(source, implbasename);
               }
               else
               {
                  assert (FALSE);
               }
            }
         }
         else
         {
            // NOTE: object operations will not have decls
            // cerr << "no decl for " << (*it).m_opname << endl;
         }
      }
   }
}


