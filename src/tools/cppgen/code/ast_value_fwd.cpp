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
   bool abstract,
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
   pd_full_definition = idl_global->gen()->create_valuetype(abstract, false, false, n, NULL, -1, NULL, -1, p);
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
