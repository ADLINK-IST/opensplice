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
