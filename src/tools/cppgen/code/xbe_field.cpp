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

