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
   virtual AST_Interface *create_interface
   (
      bool local,
      bool abstract,
      UTL_ScopedName *n,
      AST_Interface **ih,
      long nih,
      const UTL_Pragmas &p
   );
   virtual AST_InterfaceFwd * create_interface_fwd
   (
      bool local,
      bool abstract,
      UTL_ScopedName * n,
      const UTL_Pragmas & p
   );
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
   virtual AST_Attribute *create_attribute(bool ro,
                                           AST_Type *ft,
                                           UTL_ScopedName *n,
                                           const UTL_Pragmas &p);
   virtual AST_Union *create_union
   (
      UTL_ScopedName *n,
      const UTL_Pragmas &p
   );
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
      bool abstract,
      bool custom,
      bool truncatable,
      UTL_ScopedName *n,
      AST_Value **ih,
      long nih,
      AST_Interface **supports,
      long nsupports,
      const UTL_Pragmas &p
   );

   virtual AST_ValueFwd *create_valuetype_fwd
   (
      bool abstract,
      UTL_ScopedName *n,
      const UTL_Pragmas &p
   );

   virtual AST_StateMember *create_state_member
   (
      bool public_access,
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
