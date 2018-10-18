/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef _AST_BOXEDVALUE_HH
#define _AST_BOXEDVALUE_HH


#include "idl_fwd.h"
#include "idl_narrow.h"
#include "ast_type.h"
#include "utl_scope.h"
#include "ast_decl.h"


class AST_BoxedValue : public virtual AST_Type
{

public:
   // Operations

   // Constructor(s)
   AST_BoxedValue();
   AST_BoxedValue
   (
      UTL_ScopedName *n,
      AST_Type *t,
      const UTL_Pragmas &p
   );
   virtual ~AST_BoxedValue()
   {}

   // Data Accessors
   AST_Type* boxed_type ();

   // Narrowing
   DEF_NARROW_METHODS1(AST_BoxedValue, AST_Type);

   // AST Dumping
   virtual void dump(ostream &o);

private:
   // Data
   AST_Type *pd_boxed_type;
};

#endif           // _AST_BOXEDVALUE_HH
