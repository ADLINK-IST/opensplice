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
AST_Initializer::AST_Initializer ()
    : pd_exceptions (NULL)
{
}

AST_Initializer::AST_Initializer
(
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
  : AST_Decl (AST_Decl::NT_init, n, p),
    UTL_Scope (AST_Decl::NT_init, n, p),
    pd_exceptions (NULL)
{
}

/*
 * Add these exceptions (identified by name) to this scope.
 * This looks up each name to resolve it to the name of a known
 * exception, and then adds the referenced exception to the list
 * of exceptions that this operation can raise.
 *
 * NOTE: No attempt is made to ensure that exceptions are mentioned
 *       only once..
 */
UTL_NameList * AST_Initializer::fe_add_exceptions(UTL_NameList *t)
{
   UTL_NamelistActiveIterator *nl_i;
   UTL_ScopedName *nl_n;
   AST_Exception *fe;
   AST_Decl *d;

   idl_global->scopes()->top();
   pd_exceptions = NULL;
   nl_i = new UTL_NamelistActiveIterator(t);

   while (!(nl_i->is_done()))
   {
      nl_n = nl_i->item();
      d = lookup_by_name(nl_n, I_TRUE);

      if (d == NULL || d->node_type() != AST_Decl::NT_except)
      {
         idl_global->err()->lookup_error(nl_n);
         delete nl_i;
         return NULL;
      }

      fe = AST_Exception::narrow_from_decl(d);

      if (fe == NULL)
      {
         idl_global->err()->error1(UTL_Error::EIDL_ILLEGAL_RAISES, this);
         return NULL;
      }

      if (pd_exceptions == NULL)
         pd_exceptions = new UTL_ExceptList(fe, NULL);
      else
         pd_exceptions->nconc(new UTL_ExceptList(fe, NULL));

      nl_i->next();
   }

   delete nl_i;

   return t;
}

/*
 * Add this AST_Argument node (an operation argument declaration)
 * to this scope
 */
AST_Argument *AST_Initializer::fe_add_argument(AST_Argument *t)
{
   AST_Decl *d;

   /*
    * Already defined and cannot be redefined? Or already used?
    */

   if ((d = lookup_by_name_local (t->local_name ())) != NULL)
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

/*
 * Dump this AST_Initializer node (an operation) to the ostream o
 */
void AST_Initializer::dump(ostream &o)
{
   UTL_ScopeActiveIterator *i;
   UTL_ExceptlistActiveIterator *ei;
   AST_Decl *d;
   AST_Exception *e;

   i = new UTL_ScopeActiveIterator(this, IK_decls);

   o << " ";

   local_name()->dump(o);

   o << "(";

   while (!(i->is_done()))
   {
      d = i->item();
      d->dump(o);
      i->next();

      if (!(i->is_done()))
         o << ", ";
   }

   delete i;
   o << ")";

   if (pd_exceptions != NULL)
   {
      o << " raises(";
      ei = new UTL_ExceptlistActiveIterator(pd_exceptions);

      while (!(ei->is_done()))
      {
         e = ei->item();
         ei->next();
         e->local_name()->dump(o);

         if (!(ei->is_done()))
            o << ", ";
      }

      delete ei;
      o << ")";
   }
}

/*
 * Data accessors
 */

UTL_ExceptList * AST_Initializer::exceptions()
{
   return pd_exceptions;
}

// Narrowing

IMPL_NARROW_METHODS2(AST_Initializer, AST_Decl, UTL_Scope)
IMPL_NARROW_FROM_DECL(AST_Initializer)
IMPL_NARROW_FROM_SCOPE(AST_Initializer)
