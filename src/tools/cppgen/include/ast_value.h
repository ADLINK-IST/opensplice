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
   virtual ~AST_Value()
   {}

   // Data Accessors
   bool is_abstract ();
   bool is_custom ();
   bool is_truncatable ();

   void set_custom (bool v);
   void set_truncatable (bool v);

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
   bool pd_abstract;

   AST_Value **pd_value_inherits; // Inherited values
   // This is an array of pointers
   // to the inherited values
   long pd_n_value_inherits; // How many of them?
   bool pd_truncatable;

   bool pd_custom;

   // Scope Management Protocol
   friend int yyparse();

   virtual AST_StateMember *fe_add_state_member(AST_StateMember *m);
   virtual AST_Initializer *fe_add_initializer(AST_Initializer *i);
};

#endif           // _AST_VALUE_AST_VALUE_HH
