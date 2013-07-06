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
#include "idl_global.h"

void AST_Opaque::opaqueName(UTL_IdList* namen)
{
   opaque_name = namen;
}

UTL_IdList * AST_Opaque::opaqueName() const
{
   return opaque_name;
}

void AST_Opaque::dump(ostream& os)
{
   os << "opaque " << opaque_name << ";\n";
}

AST_Opaque::AST_Opaque()
{}

AST_Opaque::~AST_Opaque()
{
   if (opaque_name)
      delete opaque_name;
}

AST_Opaque::AST_Opaque(UTL_IdList*n, const UTL_Pragmas & p)
:
   AST_Decl (AST_Decl::NT_opaque, n, p),
   opaque_name (n)
{}

IMPL_NARROW_METHODS1(AST_Opaque, AST_Type)
IMPL_NARROW_FROM_DECL(AST_Opaque)
