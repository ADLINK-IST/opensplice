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
#ifndef _AST_VALUE_AST_VALUE_HH
#define _AST_VALUE_AST_VALUE_HH


#include "idl_fwd.h"
#include "idl_narrow.h"
#include "ast_type.h"
#include "utl_scope.h"
#include "ast_decl.h"


class AST_Value : public virtual AST_Interface
{

public:
   // Operations

   // Constructor(s)
   AST_Value();
   AST_Value
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
   virtual ~AST_Value()
   {}

   // Data Accessors
   idl_bool is_abstract ();
   idl_bool is_custom ();
   idl_bool is_truncatable ();

   void set_custom (idl_bool v);
   void set_truncatable (idl_bool v);

   AST_Value **value_inherits();
   
   void set_value_inherits(AST_Value **v);
   long n_value_inherits();
   void set_n_value_inherits(long i);

   // Narrowing
   DEF_NARROW_METHODS3(AST_Value, AST_Interface, AST_Type, UTL_Scope);
   DEF_NARROW_FROM_DECL(AST_Value);
   DEF_NARROW_FROM_SCOPE(AST_Value);

   // AST Dumping
   virtual void dump(ostream &o);

private:
   // Data
   idl_bool pd_abstract;

   AST_Value **pd_value_inherits; // Inherited values
   // This is an array of pointers
   // to the inherited values
   long pd_n_value_inherits; // How many of them?
   idl_bool pd_truncatable;

   idl_bool pd_custom;

   // Scope Management Protocol
   friend int yyparse();

   virtual AST_StateMember *fe_add_state_member(AST_StateMember *m);
   virtual AST_Initializer *fe_add_initializer(AST_Initializer *i);
};

#endif           // _AST_VALUE_AST_VALUE_HH
