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

#include "idl.h"
#include "idl_extern.h"

/*
 * Constructor(s) and destructor
 */
AST_BoxedValue::AST_BoxedValue ()
: 
   pd_boxed_type (NULL)
{}

AST_BoxedValue::AST_BoxedValue
(
   UTL_ScopedName *n,
   AST_Type *t,
   const UTL_Pragmas &p
)
: 
   AST_Type (AST_Decl::NT_value, n, p),
   pd_boxed_type (t)
{
}

/*
 * Private operations
 */

/*
 * Public operations
 */


/*
 * Redefinition of inherited virtual operations
 */

/*
 * Dump this AST_BoxedValue node to the ostream o
 */
void
AST_BoxedValue::dump(ostream &o)
{
   o << "boxed valuetype ";
   local_name()->dump(o);
   o << " {\n";
   if (pd_boxed_type)
   {
      pd_boxed_type->dump(o);
   }
   idl_global->indent()->skip_to(o);
   o << "}";
}

/*
 * Data accessors
 */

AST_Type *AST_BoxedValue::boxed_type ()
{
   return pd_boxed_type;
}

/*
 * Narrowing methods
 */
IMPL_NARROW_METHODS1(AST_BoxedValue, AST_Type)
