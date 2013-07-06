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
AST_ValueFwd::AST_ValueFwd()
      : pd_full_definition(NULL)
{}

AST_ValueFwd::AST_ValueFwd
(
   idl_bool abstract,
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
   : AST_Decl(AST_Decl::NT_value_fwd, n, p),
     AST_Type(AST_Decl::NT_value_fwd, n, p)
{
   /*
    * Create a dummy placeholder for the forward declared value. This
    * value node is not yet defined (n_inherits < 0), so some operations
    * will fail
    */
   pd_full_definition = idl_global->gen()->create_valuetype(abstract, I_FALSE, I_FALSE, n, NULL, -1, NULL, -1, p);
   /*
    * Record the node in a list to be checked after the entire AST has been
    * parsed. All nodes in the list must have n_inherits >= 0, else this
    * indicates that a full definition was not seen for this forward
    * delcared value
    */
   // AST_record_fwd_interface(this);
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
 * Dump this AST_ValueFwd node to the ostream o
 */
void
AST_ValueFwd::dump(ostream &o)
{
   o << "value ";
   local_name()->dump(o);
}

/*
 * Data accessors
 */

AST_Value *AST_ValueFwd::full_definition()
{
   return pd_full_definition;
}

void AST_ValueFwd::set_full_definition(AST_Value *val)
{
   pd_full_definition = val;
}

/*
 * Narrowing methods 
 */
IMPL_NARROW_METHODS1(AST_ValueFwd, AST_Type)
IMPL_NARROW_FROM_DECL(AST_ValueFwd)
