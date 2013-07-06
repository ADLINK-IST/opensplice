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


