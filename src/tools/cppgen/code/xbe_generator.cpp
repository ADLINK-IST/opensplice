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
// xbe_generator.cc
//
// Implementation of BE generator class
//
// This implements the same protocol as AST_Generator but creates instances
// of the BE-subclassed classes instead of of AST classes

#include "idl.h"
#include "idl_extern.h"
#include "xbe_generator.h"
#include "xbe_globals.h"
#include "xbe_predefined.h"
#include "xbe_module.h"
#include "xbe_exception.h"
#include "xbe_enum.h"
#include "xbe_operation.h"
#include "xbe_structure.h"
#include "xbe_interface.h"
#include "xbe_root.h"
#include "xbe_field.h"
#include "xbe_argument.h"
#include "xbe_union.h"
#include "xbe_expr_val.h"
#include "xbe_constant.h"
#include "xbe_attribute.h"
#include "xbe_array.h"
#include "xbe_string.h"
#include "xbe_typedef.h"
#include "xbe_sequence.h"
#include "xbe_value.h"

/*
 * Constructor
 */

/*
 * Private operations
 */

/*
 * Public operations
 */

/*
 * Inherited operations redefined here
 */

/*
 * Create a BE_PredefinedType node
 */
AST_PredefinedType * be_generator::create_predefined_type
(
   AST_PredefinedType::PredefinedType t,
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
{
   return new be_predefined_type (t, n, p);
}

/*
 * Create a BE_Module node
 */
AST_Module *
be_generator::create_module(UTL_ScopedName *n, const UTL_Pragmas &p)
{
   return (AST_Module *) new be_module(n, p);
}

/*
 * Construct an BE_Root node (a node representing the root of an AST)
 */
AST_Root * be_generator::create_root(UTL_ScopedName *n,
                          const UTL_Pragmas &p)
{
   return (AST_Root *) new be_root (n, p);
}

/*
 * Create a BE_Interface node
 */
AST_Interface *
be_generator::create_interface(idl_bool local,
                               UTL_ScopedName *n,
                               AST_Interface **ih,
                               long nih,
                               const UTL_Pragmas &p)
{
   return (AST_Interface *) new be_interface(local, n, ih, nih, p);
}

/*
 * Create a BE_InterfaceFwd node
 */
AST_InterfaceFwd *
be_generator::create_interface_fwd(idl_bool local,
                                   UTL_ScopedName *n,
                                   const UTL_Pragmas &p)
{
   return (AST_InterfaceFwd *) new be_interface_fwd(local, n, p);
}

/*
 * Create a BE_Exception node
 */
AST_Exception *
be_generator::create_exception(UTL_ScopedName *n, const UTL_Pragmas &p)
{
   return (AST_Exception *) new be_exception(n, p);
}

/*
 * Create a BE_Structure node
 */
AST_Structure *
be_generator::create_structure(UTL_ScopedName *n, const UTL_Pragmas &p)
{
   return (AST_Structure *) new be_structure(n, p);
}

/*
 * Create a BE_Enum node
 */
AST_Enum *
be_generator::create_enum(UTL_ScopedName *n, const UTL_Pragmas &p)
{
   return (AST_Enum *) new be_enum(n, p);
}

/*
 * Create a BE_Operation node
 */
AST_Operation *
be_generator::create_operation(AST_Type *rt,
                               AST_Operation::Flags fl,
                               UTL_ScopedName *n,
                               const UTL_Pragmas &p)
{
   return (AST_Operation *) new be_operation(rt, fl, n, p);
}

/*
 * Create a BE_Field node
 */
AST_Field *
be_generator::create_field(AST_Type *ft, UTL_ScopedName *n, const UTL_Pragmas &p)
{
   return (AST_Field *) new be_field(ft, n, p);
}

/*
 * Create a BE_Argument node
 */
AST_Argument *
be_generator::create_argument(AST_Argument::Direction d,
                              AST_Type *ft,
                              UTL_ScopedName *n,
                              const UTL_Pragmas &p)
{
   return (AST_Argument *) new be_argument(d, ft, n, p);
}

/*
 * Create a BE_Attribute node
 */
AST_Attribute *
be_generator::create_attribute(idl_bool ro,
                               AST_Type *ft,
                               UTL_ScopedName *n,
                               const UTL_Pragmas &p)
{
   return (AST_Attribute *) new be_attribute(ro, ft, n, p);
}

/*
 * Create a BE_Union node
 */
AST_Union *
be_generator::create_union(AST_ConcreteType *dt,
                           UTL_ScopedName *n,
                           const UTL_Pragmas &p)
{
   return (AST_Union *) new be_union(dt, n, p);
}

/*
 * Create a BE_UnionBranch node
 */
AST_UnionBranch *
be_generator::create_union_branch(AST_UnionLabel *lab,
                                  AST_Type *ft,
                                  UTL_ScopedName *n,
                                  const UTL_Pragmas &p)
{
   return (AST_UnionBranch *) new be_union_branch(lab, ft, n, p);
}

/*
 * Create a BE_UnionLabel node
 */
AST_UnionLabel *
be_generator::create_union_label(AST_UnionLabel::UnionLabel ul,
                                 AST_Expression *lv)
{
   return (AST_UnionLabel *) new be_union_label(ul, lv);
}

/*
 * Create a BE_Constant node
 */
AST_Constant *
be_generator::create_constant(AST_Expression::ExprType et,
                              AST_Expression *ev,
                              UTL_ScopedName *n,
                              const UTL_Pragmas &p)
{
   return (AST_Constant *) new be_constant (et, ev, n, p);
}

/*
 * Create a symbolic BE_Expression node
 */
AST_Expression * be_generator::create_expr (UTL_ScopedName *n)
{
   return (AST_Expression *) new be_expr_val(n);
}

/*
 * Create a BE_Expression node denoting a coercion
 */
AST_Expression *
be_generator::create_expr(AST_Expression *b, AST_Expression::ExprType t)
{
   return (AST_Expression *) new be_expr_val(b, t);
}

/*
 * Create a BE_Expression node combining two other AST_Expression nodes
 */
AST_Expression *
be_generator::create_expr(AST_Expression::ExprComb c,
                          AST_Expression *v1,
                          AST_Expression *v2)
{
   return (AST_Expression *) new be_expr_val(c, v1, v2);
}

/*
 * Create a BE_Expression node denoting a long integer
 */
AST_Expression *
be_generator::create_expr(long l)
{
   return (AST_Expression *) new be_expr_val(l);
}

/*
 * Create a BE_Expression node denoting a long integer being used as a boolean
 */
AST_Expression *
be_generator::create_expr(long l, AST_Expression::ExprType t)
{
   return (AST_Expression *) new be_expr_val(l, t);
}

/*
 * Create a BE_Expression node denoting an unsigned long integer
 */
AST_Expression *
be_generator::create_expr(unsigned long l)
{
   return (AST_Expression *) new be_expr_val(l);
}

/*
 * Create a BE_Expression node denoting a char * (encapsulated as a String)
 */
AST_Expression *
be_generator::create_expr(UTL_String *s)
{
   return (AST_Expression *) new be_expr_val(s);
}

/*
 * Create a BE_Expression node denoting a character
 */
AST_Expression *
be_generator::create_expr(char c)
{
   return (AST_Expression *) new be_expr_val(c);
}

/*
 * Create a BE_Expression node denoting a 64-bit floating point number
 */

AST_Expression * be_generator::create_expr (double d, const char * str)
{
   return (AST_Expression*) new be_expr_val (d, str);
}

/*
 * Create a BE_EnumVal node
 */
AST_EnumVal *
be_generator::create_enum_val(unsigned long v,
                              UTL_ScopedName *n,
                              const UTL_Pragmas &p)
{
   return (AST_EnumVal *) new be_enum_val(v, n, p);
}

/*
 * Create a BE_Array node
 */
AST_Array *
be_generator::create_array(UTL_ScopedName *n,
                           unsigned long ndims,
                           UTL_ExprList *dims)
{
   return (AST_Array *) new be_array(n, ndims, dims);
}

/*
 * Create a BE_Sequence node
 */
AST_Sequence *
be_generator::create_sequence(AST_Expression *v, AST_Type *bt)
{
   return (AST_Sequence *) new be_sequence(v, bt);
}

/*
 * Create a BE_String node
 */
AST_String *
be_generator::create_string(AST_Expression *v)
{
   return (AST_String *) new be_string(v);
}

/*
 * Create a BE_string node for a wide string
 */
AST_String *
be_generator::create_wstring(AST_Expression *v)
{
   if (BE_Globals::map_wide)
   {
      return (AST_String *) new be_string(v);
   }
   else
   {
      return (AST_String *) new be_string(v, sizeof(wchar_t));
   }
}

/*
 * Create a BE_Typedef node
 */
AST_Typedef *
be_generator::create_typedef(AST_Type *bt, UTL_ScopedName *n, const UTL_Pragmas &p)
{
   return (AST_Typedef *) new be_typedef(bt, n, p);
}

AST_Opaque*
be_generator::create_opaque(UTL_ScopedName *n, const UTL_Pragmas& p)
{
   assert(FALSE);
   return 0;
   // return (AST_Opaque*) new be_opaque(n,p);
}

AST_Value *be_generator::create_valuetype
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
{
   return new be_value
   (
      abstract,
      custom,
      truncatable,
      n,
      ih,
      nih,
      supports,
      nsupports,
      p
   );
}

AST_ValueFwd *be_generator::create_valuetype_fwd
(
   idl_bool abstract,
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
{
   return new be_value_fwd (abstract, n, p);
}

AST_StateMember *be_generator::create_state_member
(
   idl_bool public_access,
   AST_Type *ft,
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
{
   return new be_state_member (public_access, ft, n, p);
}

AST_Initializer *be_generator::create_initializer
(
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
{
   return new be_initializer (n, p);
}

AST_BoxedValue *be_generator::create_boxed_valuetype
(
   UTL_ScopedName *n,
   AST_Type *t,
   const UTL_Pragmas &p
)
{
   return new be_boxed_valuetype (n, t, p);
}
