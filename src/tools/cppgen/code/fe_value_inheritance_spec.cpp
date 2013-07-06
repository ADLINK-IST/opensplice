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

#include "fe_private.h"

/*
 * Constructor(s) and destructor
 */

FE_ValueInheritanceSpec::FE_ValueInheritanceSpec
(
   idl_bool truncatable,
   UTL_NameList *ihl,
   UTL_NameList *supl
)
   : pd_n_inherits (0),
     pd_n_supports (0),
     pd_truncatable (truncatable)
{
   compile_value_inheritance(ihl);
   compile_inheritance(supl);
}

/*
 * Private operations
 */

/*
 * Compute the list of top-level values this one inherits from
 */
void
FE_ValueInheritanceSpec::compile_value_inheritance(UTL_NameList *nl)
{
   UTL_NamelistActiveIterator *l = new UTL_NamelistActiveIterator(nl);
   AST_Decl *d;
   AST_Value *i;
   long k;
   long count = 0;
   bool present;

   while (!(l->is_done()))
   {
      count++;
      l->next();
   }

   delete l;

   // count is an upper bound on inherited values
   pd_inherits = new AST_Value * [count];

   l = new UTL_NamelistActiveIterator(nl);

   /*
    * Compute expanded flattened non-repeating list of values
    * which this one inherits from
    */

   while (!(l->is_done()))
   {
      /*
       * Check that scope stack is valid
       */

      if (idl_global->scopes()->top() == NULL)
      {
         idl_global->err()->lookup_error(l->item());
         return ;
      }

      /*
       * Look it up
       */
      d = idl_global->scopes()->top()->lookup_by_name(l->item(), I_TRUE);

      /*
       * Not found?
       */
      if (d == NULL)
      {
         idl_global->err()->lookup_error(l->item());
         return ;
      }

      /*
       * Not a value?
       */

      while (d->node_type() == AST_Decl::NT_typedef)
         d = AST_Typedef::narrow_from_decl(d)->base_type();

      if (d->node_type() != AST_Decl::NT_value)
      {
         idl_global->err()->value_inheritance_error(d);
         return ;
      }

      /*
       * OK, cast to an value
       */
      i = AST_Value::narrow_from_decl(d);

      if (i == NULL)
         idl_global->err()->value_inheritance_error(d);

      /*
       * Forward declared value?
       */
      if (!i->is_defined())
      {
         idl_global->err()->value_inheritance_fwd_error(i);
         return ;
      }

      /*
       * OK, see if we have to add this to the list of values
       * inherited from
       */
      present = FALSE;

      for (k = 0; k < pd_n_inherits; k++)
      {
         present = (pd_inherits[k] == i);

         if (present)
         {
            break;
         }
      }

      if (!present)
      {
         pd_inherits[pd_n_inherits++] = i;
      }

      /*
       * Next element in header list
       */
      l->next();
   }

   delete l;
}

/*
 * Compute the list of top-level values this one inherits from
 */
void
FE_ValueInheritanceSpec::compile_inheritance(UTL_NameList *nl)
{
   UTL_NamelistActiveIterator *l = new UTL_NamelistActiveIterator(nl);
   AST_Decl *d;
   AST_Interface *i;
   long k;
   long count = 0;
   bool present;

   while (!(l->is_done()))
   {
      count++;
      l->next();
   }

   delete l;

   // count is an upper bound on inherited values
   pd_supports = new AST_Interface * [count];

   l = new UTL_NamelistActiveIterator(nl);

   /*
    * Compute expanded flattened non-repeating list of values
    * which this one inherits from
    */

   while (!(l->is_done()))
   {
      /*
       * Check that scope stack is valid
       */

      if (idl_global->scopes()->top() == NULL)
      {
         idl_global->err()->lookup_error(l->item());
         return ;
      }

      /*
       * Look it up
       */
      d = idl_global->scopes()->top()->lookup_by_name(l->item(), I_TRUE);

      /*
       * Not found?
       */
      if (d == NULL)
      {
         idl_global->err()->lookup_error(l->item());
         return ;
      }

      /*
       * Not an interface?
       */

      while (d->node_type() == AST_Decl::NT_typedef)
         d = AST_Typedef::narrow_from_decl(d)->base_type();

      if (d->node_type() != AST_Decl::NT_interface)
      {
         idl_global->err()->value_inheritance_error(d);
         return ;
      }

      /*
       * OK, cast to an value
       */
      i = AST_Interface::narrow_from_decl(d);

      if (i == NULL)
         idl_global->err()->value_inheritance_error(d);

      /*
       * Forward declared value?
       */
      if (!i->is_defined())
      {
         idl_global->err()->value_inheritance_fwd_error(i);
         return ;
      }

      /*
       * OK, see if we have to add this to the list of values
       * inherited from
       */
      present = FALSE;

      for (k = 0; k < pd_n_supports; k++)
      {
         present = (pd_supports[k] == i);

         if (present)
         {
            break;
         }
      }

      if (!present)
      {
         pd_supports[pd_n_supports++] = i;
      }

      /*
       * Next element in header list
       */
      l->next();
   }

   delete l;
}

/*
 * Public operations
 */

/*
 * Redefinition of inherited virtual operations
 */

/*
 * Data accessors
 */

AST_Value ** FE_ValueInheritanceSpec::inherits()
{
   return pd_inherits;
}

long FE_ValueInheritanceSpec::n_inherits()
{
   return pd_n_inherits;
}

AST_Interface **FE_ValueInheritanceSpec::supports()
{
   return pd_supports;
}

long FE_ValueInheritanceSpec::n_supports()
{
   return pd_n_supports;
}

idl_bool FE_ValueInheritanceSpec::truncatable()
{
   return pd_truncatable;
}
