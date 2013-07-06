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

/*
 * Constructor(s) and destructor
 */
AST_Value::AST_Value ()
: 
   pd_abstract (I_FALSE),
   pd_value_inherits (NULL),
   pd_n_value_inherits (0),
   pd_truncatable (I_FALSE),
   pd_custom (I_FALSE)
{}

AST_Value::AST_Value
(
   idl_bool abstract,
   idl_bool custom,
   idl_bool truncatable,
   UTL_ScopedName *n,
   AST_Value **ih,
   long nih,
   AST_Interface **supports,
   long nsupports,
   const UTL_Pragmas &p
)
   : AST_Decl (AST_Decl::NT_value, n, p),
     AST_Type (AST_Decl::NT_value, n, p),
     UTL_Scope (AST_Decl::NT_value, n, p),
     AST_Interface (I_TRUE, n, supports, nsupports, p),
     pd_abstract (abstract),
     pd_value_inherits (ih),
     pd_n_value_inherits (nih),
     pd_truncatable (truncatable),
     pd_custom (custom)
{
}

/*
 * Private operations
 */

/*
 * Public operations
 */


/*
 * Redefinition of inherited virtual operations
 */

AST_StateMember *AST_Value::fe_add_state_member(AST_StateMember *t)
{
   AST_Decl *d;

   /*
    * Can't add to interface which was not yet defined
    */

   if (!is_defined())
   {
      idl_global->err()->error2(UTL_Error::EIDL_DECL_NOT_DEFINED, this, t);
      return NULL;
   }

   /*
    * Already defined and cannot be redefined? Or already used?
    */
   if ((d = lookup_for_add (t)) != NULL)
   {
      if (!can_be_redefined(d))
      {
         idl_global->err()->error3(UTL_Error::EIDL_REDEF, t, this, d);
         return NULL;
      }

      if (referenced(d))
      {
         idl_global->err()->error3(UTL_Error::EIDL_DEF_USE, t, this, d);
         return NULL;
      }

      if (t->has_ancestor(d))
      {
         idl_global->err()->redefinition_in_scope(t, d);
         return NULL;
      }
   }

   /*
    * Add it to scope
    */
   add_to_scope(t);

   /*
    * Add it to set of locally referenced symbols
    */
   add_to_referenced(t, I_FALSE);

   return t;
}

AST_Initializer *AST_Value::fe_add_initializer(AST_Initializer *t)
{
   AST_Decl *d;

   /*
    * Can't add to interface which was not yet defined
    */

   if (!is_defined())
   {
      idl_global->err()->error2(UTL_Error::EIDL_DECL_NOT_DEFINED, this, t);
      return NULL;
   }

   /*
    * Already defined and cannot be redefined? Or already used?
    */
   if ((d = lookup_for_add (t)) != NULL)
   {
      if (!can_be_redefined(d))
      {
         idl_global->err()->error3(UTL_Error::EIDL_REDEF, t, this, d);
         return NULL;
      }

      if (referenced(d))
      {
         idl_global->err()->error3(UTL_Error::EIDL_DEF_USE, t, this, d);
         return NULL;
      }
   }

   /*
    * Add it to scope
    */
   add_to_scope(t);

   /*
    * Add it to set of locally referenced symbols
    */
   add_to_referenced(t, I_FALSE);

   return t;
}

/*
 * Dump this AST_Value node to the ostream o
 */
void
AST_Value::dump(ostream &o)
{
   long i;

   o << (local() == I_TRUE ? "local" : "") << " valuetype ";
   local_name()->dump(o);
   o << " ";

   if (pd_n_value_inherits > 0)
   {
      o << ": ";

      for (i = 0; i < pd_n_value_inherits; i++)
      {
         pd_value_inherits[i]->local_name()->dump(o);

         if (i < pd_n_value_inherits - 1)
         {
            o << ", ";
         }
      }
   }

   o << " {\n";
   AST_Interface::dump(o);
   idl_global->indent()->skip_to(o);
   o << "}";
}

/*
 * Data accessors
 */

idl_bool AST_Value::is_abstract ()
{
   return pd_abstract;
}

idl_bool AST_Value::is_custom ()
{
   return pd_custom;
}

idl_bool AST_Value::is_truncatable ()
{
   return pd_truncatable;
}

void AST_Value::set_custom (idl_bool v)
{
   pd_custom = v;
}

void AST_Value::set_truncatable (idl_bool v)
{
   pd_truncatable = v;
}

AST_Value **
AST_Value::value_inherits()
{
   return pd_value_inherits;
}

void
AST_Value::set_value_inherits(AST_Value **i)
{
   pd_value_inherits = i;
}

long
AST_Value::n_value_inherits()
{
   return pd_n_value_inherits;
}

void
AST_Value::set_n_value_inherits(long i)
{
   pd_n_value_inherits = i;
}

/*
 * Narrowing methods
 */
IMPL_NARROW_METHODS3(AST_Value, AST_Interface, AST_Type, UTL_Scope)
IMPL_NARROW_FROM_DECL(AST_Value)
IMPL_NARROW_FROM_SCOPE(AST_Value)
