/*
 * ast_union.cc - Implementation of class AST_Union
 *
 * AST_Union nodes represent IDL union declarations.
 * AST_Union is a subclass of AST_ConcreteType and of UTL_Scope (the
 * union branches are managed in a scope).
 * AST_Union nodes have a discriminator type (a subclass of AST_ConcreteType),
 * a name (an UTL_ScopedName) and a field denoting the discriminator type if
 * it is a primitive type (the value of this field is from the union
 * AST_Expression::ExprType and serves as a cache). This field is used
 * to compute coercions for labels based on the expected discriminator type.
 */

#include <idl.h>
#include <idl_extern.h>

AST_Union::AST_Union ()
   : pd_defined (false)
{}

AST_Union::AST_Union (UTL_ScopedName * n, const UTL_Pragmas & p)
: 
   AST_Decl (AST_Decl::NT_union, n, p),
   UTL_Scope (AST_Decl::NT_union, n, p),
   pd_disc_type (0),
   pd_udisc_type (AST_Expression::EV_none),
   pd_defined (false)
{
}

/*
 * Look up the default branch in union
 */
AST_UnionBranch *
AST_Union::lookup_default()
{
   UTL_ScopeActiveIterator *i = new UTL_ScopeActiveIterator(this, IK_both);
   AST_UnionBranch *b;
   AST_Decl *d;

   while (!(i->is_done()))
   {
      d = i->item();

      if (d->node_type() == AST_Decl::NT_union_branch)
      {
         b = AST_UnionBranch::narrow_from_decl(d);

         if (b == NULL)
         {
            i->next();
            continue;
         }

         if (b->label() != NULL &&
               b->label()->label_kind() == AST_UnionLabel::UL_default)
         {
            idl_global->err()->error2(UTL_Error::EIDL_MULTIPLE_BRANCH, this, b);
            delete i;
            return b;
         }
      }

      i->next();
   }

   delete i;
   return NULL;
}

/*
 * Look up a branch by label
 */
AST_UnionBranch *
AST_Union::lookup_label(AST_UnionBranch *b)
{
   AST_UnionLabel *l = b->label();
   UTL_ScopeActiveIterator *i;
   AST_Decl *d;
   AST_UnionBranch *fb;

   if (l->label_val() == NULL)
      return b;

   l->label_val()->set_ev(l->label_val()->coerce(pd_udisc_type));

   if (l->label_val()->ev() == NULL)
   {
      idl_global->err()->eval_error(l->label_val());
      return b;
   }

   i = new UTL_ScopeActiveIterator(this, IK_decls);

   while (!(i->is_done()))
   {
      d = i->item();

      if (d->node_type() == AST_Decl::NT_union_branch)
      {
         fb = AST_UnionBranch::narrow_from_decl(d);

         if (fb == NULL)
         {
            i->next();
            continue;
         }

         if (fb->label() != NULL &&
               fb->label()->label_kind() == AST_UnionLabel::UL_label &&
               fb->label()->label_val()->compare(l->label_val()))
         {
            idl_global->err()->error2(UTL_Error::EIDL_MULTIPLE_BRANCH, this, b);
            delete i;
            return b;
         }
      }

      i->next();
   }

   delete i;
   return NULL;
}

/*
 * Look up a branch in an enum which is the discriminator type for this
 * union, based on the label value which must be an enumerator in that
 * enum.
 */
AST_UnionBranch *
AST_Union::lookup_enum(AST_UnionBranch *b)
{
   AST_UnionLabel *l = b->label();
   AST_Expression *v;
   AST_Enum *e = AST_Enum::narrow_from_decl(pd_disc_type);
   UTL_ScopeActiveIterator *i;
   AST_Decl *d;
   AST_UnionBranch *fb;

   if (e == NULL)
      return NULL;

   if (l->label_val() == NULL)
      return b;

   v = l->label_val();

   /*
    * Expecting a symbol label
    */
   if (v->ec() != AST_Expression::EC_symbol)
   {
      idl_global->err()->enum_val_expected(this, l);
      return b;
   }

   /*
    * See if the symbol defines a constant in the discriminator enum
    */
   d = e->lookup_by_name(v->n(), true);

   if (d == NULL || d->defined_in() != e)
   {
      idl_global->err()->enum_val_lookup_failure(this, e, v->n());
      return b;
   }

   /*
    * OK, now see if this symbol is already used as the label of
    * some other branch
    */
   i = new UTL_ScopeActiveIterator(this, IK_decls);

   while (!(i->is_done()))
   {
      d = i->item();

      if (d->node_type() == AST_Decl::NT_union_branch)
      {
         fb = AST_UnionBranch::narrow_from_decl(d);

         if (fb == NULL)
         {
            i->next();
            continue;
         }

         if (fb->label() != NULL &&
               fb->label()->label_kind() == AST_UnionLabel::UL_label &&
               fb->label()->label_val() == l->label_val())
         {
            idl_global->err()->error2(UTL_Error::EIDL_MULTIPLE_BRANCH, this, b);
            delete i;
            return b;
         }
      }

      i->next();
   }

   delete i;
   return NULL;
}

/*
 * Look up a branch by value. This is the top level branch label resolution
 * entry point. It dispatches to the right lookup function depending on the
 * union discriminator type.
 */
AST_UnionBranch *
AST_Union::lookup_branch(AST_UnionBranch *branch)
{
   AST_UnionLabel *l = NULL;


   if (branch != NULL)
      l = branch->label();

   if (l != NULL)
   {
      if (l->label_kind() == AST_UnionLabel::UL_default)
         return lookup_default();

      if (pd_udisc_type == AST_Expression::EV_any)
         /* CONVENTION: indicates enum discr */
         return lookup_enum(branch);

      return lookup_label(branch);
   }

   return NULL;
}

/*
 * Public operations
 */

/*
 * Redefinition of inherited virtual operations
 */

/*
 * Add this AST_UnionBranch node (a node representing one branch in a
 * union declaration) to this scope
 */
AST_UnionBranch *AST_Union::fe_add_union_branch(AST_UnionBranch *t)
{
   //AST_Decl *d;

   /*
    * If this is a malformed branch, don't do anything with it
    */

   if (t == NULL || t->label() == NULL)
      return NULL;

   if (local() != true)
   {
      set_local(t->field_type()->local());
   }

   /*
    * If branch with that label already exists, complain
    */
   if (lookup_branch(t) != NULL)
   {
      idl_global->err()->error2(UTL_Error::EIDL_MULTIPLE_BRANCH, this, t);
      return NULL;
   }

   // This checking was removed because it disallows use
   // of multiple case labels with identical type_specs
   /*
    * If branch with same field name exists, complain
   if ((d = lookup_for_add (t)) != NULL) {
     if (!can_be_redefined(d)) {
       idl_global->err()->error3(UTL_Error::EIDL_REDEF, t, this, d);
       return NULL;
     }
     if (referenced(d)) {
       idl_global->err()->error3(UTL_Error::EIDL_DEF_USE, t, this, d);
       return NULL;
     }
     if (t->has_ancestor(d)) {
       idl_global->err()->redefinition_in_scope(t, d);
       return NULL;
     }
   }
    */

   /*
    * Add it to scope
    */
   add_to_scope(t);

   /*
    * Add it to set of locally referenced symbols
    */
   add_to_referenced(t, false);

   return t;
}

/*
 * Add this AST_Union (manifest union type) to this scope
 */
AST_Union *AST_Union::fe_add_union(AST_Union *t)
{
   AST_Decl *d;

   if (local() != true)
   {
      set_local(t->local());
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
    * Add it to local types
    */
   add_to_local_types(t);

   /*
    * Add it to set of locally referenced symbols
    */
   add_to_referenced(t, false);

   return t;
}

/*
 * Add this AST_Structure node (manifest struct type) to this scope
 */
AST_Structure *AST_Union::fe_add_structure(AST_Structure *t)
{
   AST_Decl *d;

   if (local() != true)
   {
      set_local(t->local());
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
    * Add it to local types
    */
   add_to_local_types(t);

   /*
    * Add it to set of locally referenced symbols
    */
   add_to_referenced(t, false);

   return t;
}

/*
 * Add this AST_Enum node (manifest enum type) to this scope
 */
AST_Enum *AST_Union::fe_add_enum(AST_Enum *t)
{
   AST_Decl *d;

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
    * Add it to local types
    */
   add_to_local_types(t);

   /*
    * Add it to set of locally referenced symbols
    */
   add_to_referenced(t, false);

   return t;
}

/*
 * Add this AST_EnumVal node (enumerator declaration) to this scope.
 * This is done to conform to the C++ scoping rules which declare
 * enumerators in the enclosing scope (in addition to declaring them
 * in the enum itself)
 */
AST_EnumVal *AST_Union::fe_add_enum_val(AST_EnumVal *t)
{
   AST_Decl *d;

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
   add_to_referenced(t, false);

   return t;
}


/*
 * Add an AST_Typedef (a typedef) to the current scope
 */
AST_Typedef *AST_Union::fe_add_typedef(AST_Typedef *t)
{
   AST_Decl *d;

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
   add_to_referenced(t, false);

   return t;
}
/*
 * Dump this AST_Union node to the ostream o
 */
void
AST_Union::dump(ostream &o)
{
   o << "union ";
   local_name()->dump(o);
   o << " switch (";
   pd_disc_type->local_name()->dump(o);
   o << ") {\n";
   UTL_Scope::dump(o);
   idl_global->indent()->skip_to(o);
   o << "}";
}

/*
 * Data accessors
 */

AST_ConcreteType * AST_Union::disc_type ()
{
   return pd_disc_type;
}

void AST_Union::update_type ()
{
   /* Virtual function used to update type info in derived XBE class */
}

void AST_Union::set_disc_type (AST_ConcreteType * dt)
{
   AST_PredefinedType *pdt;

   pd_disc_type = dt;
   if (dt->node_type () == AST_Decl::NT_pre_defined)
   {
      pdt = AST_PredefinedType::narrow_from_decl(dt);

      if (pdt == NULL)
      {
         pd_disc_type = NULL;
         pd_udisc_type = AST_Expression::EV_none;
         return ;
      }

      pd_disc_type = dt;

      switch (pdt->pt())
      {
         case AST_PredefinedType::PT_long:
         pd_udisc_type = AST_Expression::EV_long;
         break;

         case AST_PredefinedType::PT_ulong:
         pd_udisc_type = AST_Expression::EV_ulong;
         break;

         case AST_PredefinedType::PT_short:
         pd_udisc_type = AST_Expression::EV_short;
         break;

         case AST_PredefinedType::PT_ushort:
         pd_udisc_type = AST_Expression::EV_ushort;
         break;

         case AST_PredefinedType::PT_char:
         pd_udisc_type = AST_Expression::EV_char;
         break;

         case AST_PredefinedType::PT_octet:
         pd_udisc_type = AST_Expression::EV_octet;
         break;

         case AST_PredefinedType::PT_boolean:
         pd_udisc_type = AST_Expression::EV_bool;
         break;

         default:
         pd_udisc_type = AST_Expression::EV_none;
         pd_disc_type = NULL;
         break;
      }
   }
   else if (dt->node_type() == AST_Decl::NT_enum)
   {
      pd_udisc_type = AST_Expression::EV_any;
      pd_disc_type = dt;
   }
   else
   {
      pd_udisc_type = AST_Expression::EV_none;
      pd_disc_type = NULL;
   }

   if (pd_disc_type == NULL)
      idl_global->err()->error2(UTL_Error::EIDL_DISC_TYPE, this, dt);

   update_type ();
}

AST_Expression::ExprType AST_Union::udisc_type ()
{
   return pd_udisc_type;
}

// Narrowing
IMPL_NARROW_METHODS2(AST_Union, AST_ConcreteType, UTL_Scope)
IMPL_NARROW_FROM_DECL(AST_Union)
IMPL_NARROW_FROM_SCOPE(AST_Union)
