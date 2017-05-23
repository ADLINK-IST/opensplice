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
AST_StateMember::AST_StateMember()
      : pd_public_access(true)
{}

AST_StateMember::AST_StateMember
(
   bool public_access,
   AST_Type *ft,
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
  : AST_Decl (AST_Decl::NT_state_member, n, p),
    AST_Field (AST_Decl::NT_state_member, ft, n, p),
    pd_public_access (public_access)
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
 * Dump this AST_StateMember to the ostream o
 */
void
AST_StateMember::dump(ostream &o)
{
   o << (pd_public_access ? "public" : "private") << " state member ";
   AST_Field::dump(o);
}

/*
 * Data accessors
 */

bool AST_StateMember::public_access ()
{
   return pd_public_access;
}

/*
 * Narrowing methods
 */
IMPL_NARROW_METHODS1(AST_StateMember, AST_Field)
IMPL_NARROW_FROM_DECL(AST_StateMember)
