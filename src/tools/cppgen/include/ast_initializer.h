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
#ifndef _AST_INITIALIZER_HH
#define _AST_INITIALIZER_HH


// Representation of operation declaration:

/*
** DEPENDENCIES: ast_decl.hh, utl_scope.hh, ast_type.hh, utl_strlist.hh,
**   utl_exceptlist.hh, utl_scoped_name.hh
**
** USE: included from ast.hh
*/

#include "idl_fwd.h"
#include "idl_narrow.h"
#include "utl_list.h"
#include "ast_decl.h"
#include "utl_scope.h"
#include "utl_scoped_name.h"


class AST_Initializer : public virtual AST_Decl, public virtual UTL_Scope
{

public:
   // Initializers

   // Constructor(s)
   AST_Initializer();
   AST_Initializer
   (
      UTL_ScopedName *n,
      const UTL_Pragmas &p
   );
   virtual ~AST_Initializer()
   {}

   // Data Accessors
   UTL_ExceptList *exceptions();

   // Narrowing
   DEF_NARROW_METHODS2(AST_Initializer, AST_Decl, UTL_Scope);
   DEF_NARROW_FROM_DECL(AST_Initializer);
   DEF_NARROW_FROM_SCOPE(AST_Initializer);

   // AST Dumping
   virtual void dump(ostream &o);

private:
   UTL_ExceptList *pd_exceptions;  // Exceptions raised

   // Scope Management Protocol
   friend int yyparse();

   virtual AST_Argument *fe_add_argument(AST_Argument *a);
   // Add context
   virtual UTL_NameList *fe_add_exceptions(UTL_NameList *e);
   // exceptions

};

#endif           // _AST_INITIALIZER_HH
