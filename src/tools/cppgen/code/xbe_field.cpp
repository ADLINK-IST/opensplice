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

#include "xbe_globals.h"
#include "xbe_literals.h"
#include "xbe_field.h"
#include "xbe_argument.h"

#include "cppgen_iostream.h"

// -------------------------------------------------
//  BE_FIELD IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS2(be_field, AST_Field, be_TypeMap)
IMPL_NARROW_FROM_DECL(be_field)


be_field::be_field()
{}

be_field::be_field (AST_Type *ft, UTL_ScopedName *n, const UTL_Pragmas &p)
:
   AST_Decl (AST_Decl::NT_field, n, p),
   AST_Field (ft, n, p),
   m_type (0)
{
   m_type = be_Type::_narrow (field_type ());
   const char * tName = m_type->TypeName ();

   if (tName && strcmp (tName, "DDS::Any") == 0)
   {
      BE_Globals::inc_any = pbtrue;
   }
}

be_field *
be_field::_narrow(AST_Decl * decl)
{
   return (be_field*)decl->narrow((long)&be_field::type_id);
}

DDS::Boolean
be_field::IsFixedLength() const
{
   return m_type->IsFixedLength();
}

DDS::Boolean
be_field::IsFixedLengthPrimitiveType() const
{
   return FALSE;
}
void
be_field::InitializeTypeMap(be_Type* )
{
   idlType = m_type;

   typeName = m_type->TypeName();
   inTypeName = m_type->InTypeName();
   inoutTypeName = m_type->InOutTypeName();
   outTypeName = m_type->OutTypeName();
   returnTypeName = m_type->MakeSignature(VT_Return);
   dmfAdtMemberTypeName = m_type->DMFAdtMemberTypeName();
   structMemberTypeName = m_type->StructMemberTypeName();
   unionMemberTypeName = m_type->UnionMemberTypeName();
   sequenceMemberTypeName = m_type->SequenceMemberTypeName();
   streamOpTypeName = m_type->StreamOpTypeName();
   istreamOpTypeName = m_type->IStreamOpTypeName();
}

void
be_field::initialize()
{
   m_type->Initialize();
   InitializeTypeMap(m_type);
}

