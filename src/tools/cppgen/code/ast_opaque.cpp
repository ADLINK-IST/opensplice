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
