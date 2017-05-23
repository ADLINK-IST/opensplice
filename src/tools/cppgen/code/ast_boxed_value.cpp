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
