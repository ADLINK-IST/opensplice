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
