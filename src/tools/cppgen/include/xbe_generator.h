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
#ifndef _XBE_GENERATOR_HH
#define _XBE_GENERATOR_HH

// xbe_generator.hh
//
// Defines BE class for generator
//
// This defines the same protocol as the AST_Generator class but actually
// is implemented to create nodes of AST classes subclassed in this BE

/*
** DEPENDENCIES: AST_Generator.h
**
** USE: Included from be.h
*/

class be_generator
         :
         public AST_Generator
{

public:

   //
   // Operations
   //
   virtual AST_PredefinedType
   *create_predefined_type(AST_PredefinedType::PredefinedType t,
                           UTL_ScopedName *n,
                           const UTL_Pragmas &p);

   virtual AST_Module *create_module(UTL_ScopedName *n, const UTL_Pragmas &p);
   virtual AST_Root *create_root(UTL_ScopedName *n, const UTL_Pragmas &p);
   virtual AST_Interface *create_interface(idl_bool local,
                                           UTL_ScopedName *n,
                                           AST_Interface **ih,
                                           long nih,
                                           const UTL_Pragmas &p);
   virtual AST_InterfaceFwd *create_interface_fwd(idl_bool local,
                                                  UTL_ScopedName *n,
                                                  const UTL_Pragmas &p);
   virtual AST_Exception *create_exception(UTL_ScopedName *n, const UTL_Pragmas &p);
   virtual AST_Structure *create_structure(UTL_ScopedName *n, const UTL_Pragmas &p);
   virtual AST_Enum *create_enum(UTL_ScopedName *n, const UTL_Pragmas &p);
   virtual AST_Operation *create_operation(AST_Type *rt,
                                           AST_Operation::Flags fl,
                                           UTL_ScopedName *n,
                                           const UTL_Pragmas &p);
   virtual AST_Field *create_field(AST_Type *ft, UTL_ScopedName *n,
                                   const UTL_Pragmas &p);
   virtual AST_Argument *create_argument(AST_Argument::Direction d,
                                         AST_Type *ft,
                                         UTL_ScopedName *n,
                                         const UTL_Pragmas &p);
   virtual AST_Attribute *create_attribute(idl_bool ro,
                                           AST_Type *ft,
                                           UTL_ScopedName *n,
                                           const UTL_Pragmas &p);
   virtual AST_Union *create_union(AST_ConcreteType *dt,
                                   UTL_ScopedName *n,
                                   const UTL_Pragmas &p);
   virtual AST_UnionBranch *create_union_branch(AST_UnionLabel *lab,
         AST_Type *ft,
         UTL_ScopedName *n,
         const UTL_Pragmas &p);
   virtual AST_UnionLabel *create_union_label(AST_UnionLabel::UnionLabel ul,
         AST_Expression *lv);
   virtual AST_Constant *create_constant(AST_Expression::ExprType et,
                                         AST_Expression *ev,
                                         UTL_ScopedName *n,
                                         const UTL_Pragmas &p);

   virtual AST_Expression *create_expr (UTL_ScopedName *n);
   virtual AST_Expression *create_expr (AST_Expression *v,
                                        AST_Expression::ExprType t);
   virtual AST_Expression *create_expr (AST_Expression::ExprComb c,
                                        AST_Expression *v1,
                                        AST_Expression *v2);
   virtual AST_Expression *create_expr (long v);
   virtual AST_Expression *create_expr (long v,
                                        AST_Expression::ExprType t);
   virtual AST_Expression *create_expr (unsigned long v);
   virtual AST_Expression *create_expr (UTL_String *s);
   virtual AST_Expression *create_expr (char c);
   virtual AST_Expression *create_expr (double d, const char * str);

   virtual AST_EnumVal *create_enum_val(unsigned long v,
                                        UTL_ScopedName *n,
                                        const UTL_Pragmas &p);
   virtual AST_Array *create_array(UTL_ScopedName *n,
                                   unsigned long ndims,
                                   UTL_ExprList *dims);
   virtual AST_Sequence *create_sequence(AST_Expression *v, AST_Type *bt);
   virtual AST_String *create_string(AST_Expression *v);
   virtual AST_String *create_wstring(AST_Expression *v);
   virtual AST_Typedef *create_typedef(AST_Type *bt,
                                       UTL_ScopedName *n,
                                       const UTL_Pragmas &p);
   virtual AST_Opaque *create_opaque(UTL_ScopedName *n,
                                     const UTL_Pragmas& p);

   virtual AST_Value *create_valuetype
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
   );

   virtual AST_ValueFwd *create_valuetype_fwd
   (
      idl_bool abstract,
      UTL_ScopedName *n,
      const UTL_Pragmas &p
   );

   virtual AST_StateMember *create_state_member
   (
      idl_bool public_access,
      AST_Type *ft,
      UTL_ScopedName *n,
      const UTL_Pragmas &p
   );

   virtual AST_Initializer *create_initializer
   (
      UTL_ScopedName *n,
      const UTL_Pragmas &p
   );

   virtual AST_BoxedValue *create_boxed_valuetype
   (
      UTL_ScopedName *n,
      AST_Type *t,
      const UTL_Pragmas &p
   );
};

#endif           // _BE_GENERATOR_HH
