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
AST_StateMember::AST_StateMember()
      : pd_public_access(I_TRUE)
{}

AST_StateMember::AST_StateMember
(
   idl_bool public_access,
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

idl_bool AST_StateMember::public_access ()
{
   return pd_public_access;
}

/*
 * Narrowing methods
 */
IMPL_NARROW_METHODS1(AST_StateMember, AST_Field)
IMPL_NARROW_FROM_DECL(AST_StateMember)
