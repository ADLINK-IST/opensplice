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

#include "xbe.h"
#include "xbe_literals.h"
#include "xbe_array.h"
#include "xbe_enum.h"
#include "xbe_root.h"
#include "xbe_sequence.h"
#include "xbe_typedef.h"

// -------------------------------------------------
//  BE_TYPEDEF IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS4(be_typedef, AST_Typedef, be_CodeGenerator,
                     be_DispatchableType, be_Type)
IMPL_NARROW_FROM_DECL(be_typedef)

be_typedef::be_typedef()
{
}

be_typedef::be_typedef (AST_Type *bt, UTL_ScopedName *n, const UTL_Pragmas &p)
:
   AST_Decl (AST_Decl::NT_typedef, n, p),
   AST_Typedef (bt, n, p),
   m_generateBase (FALSE),
   m_generated (FALSE),
   m_baseBeType (0)
{
   AST_Type* astType = base_type();
   be_array* ba;
   be_sequence* bs;

   localName = local_name()->get_string();
   enclosingScope = be_Type::EnclosingScopeString(this);
   m_baseBeType = get_base_be_type();

   //
   // make sure the base type has a name (if anonymous)
   //

   bs = (be_sequence*)astType->narrow((long) & be_sequence::type_id);
   if (bs)
   {
      m_generateBase = bs->SetName(enclosingScope, localName);
   }
   else if ((ba = (be_array*)astType->narrow((long) & be_array::type_id)))
   {
      m_generateBase = ba->SetName(enclosingScope, localName);
   }

   //
   // now initialize the base's type and typemap
   //
   m_baseBeType->Initialize();

   m_baseBeType->HasTypeDef (pbtrue);

   InitializeTypeMap (this);

   m_typecode = m_baseBeType->m_typecode;
   m_typecode->id = get_decl_pragmas().get_repositoryID()->get_string();
   m_typecode->name_of_type = localName;

   DDS_StdString scopedname = NoColons(enclosingScope + "_" + localName);

   TypeCodeTypeName(BE_Globals::TCPrefix + localName);

   MetaTypeTypeName(BE_Globals::MTPrefix + scopedname);

   TypeCodeRepName(BE_Globals::TCRepPrefix + scopedname);

   TypeCodeBaseName(BE_Globals::TCBasePrefix + scopedname);
}

be_Type *
be_typedef::get_base_be_type() const
{
   if (!m_baseBeType)
   {
      be_typedef * nctd = (be_typedef*)this;
      AST_Type * atype = nctd->base_type();

      assert(atype);

      nctd->m_baseBeType = be_Type::_narrow(atype);
   }

   assert(m_baseBeType);

   return m_baseBeType;
}

void
be_typedef::Initialize()
{
   be_Type * basetype = get_base_be_type();

   //
   // initialize the typedef'S map using its base type'S rules
   //
   basetype->Initialize();
   basetype->InitializeTypeMap(this);
}

void
be_typedef::Generate(be_ClientHeader& source)
{
   if (BE_Globals::ignore_interfaces && IsInterfaceDependant ())
   {
      return;
   }

   if (!Generated())
   {
      be_Type * btype;

      m_generated = TRUE;

      if (base_type() &&
          (btype = (be_Type*)base_type()->narrow((long) & be_Type::type_id)))
      {
         if (m_generateBase)
         {
            be_CodeGenerator * cg;
            if ((cg = (be_CodeGenerator *) btype->narrow((long) & be_CodeGenerator::type_id)))
            {
               cg->isAtModuleScope (this->isAtModuleScope());
            }
            btype->GenerateType(source);
         }
         else
         {
            btype->GenerateTypedefs(enclosingScope, *this, source);

            be_root::AddTypedef(*this);
            be_root::AddTypecode(*this);
         }
      }
      else
      {
         assert(pbfalse);
      }
   }
}

void
be_typedef::InitializeTypeMap(be_Type* t)
{
   idlType = t;

   assert(t);

   //
   // initialize the typedef's map using its base type'S rules
   //
   m_baseBeType->InitializeTypeMap(t);

}

DDS::Boolean
be_typedef::IsFixedLength() const
{
   return m_baseBeType->IsFixedLength();
}

DDS::Boolean
be_typedef::IsFixedLengthPrimitiveType() const
{
   return FALSE;
}

DDS::Boolean be_typedef::IsInterfaceDependant () const
{
   return m_baseBeType->IsInterfaceDependant ();
}

void
be_typedef::GenerateTypedefs( const DDS_StdString &scope,
                              const be_typedef& alias,
                              be_ClientHeader& source)
{
   m_baseBeType->GenerateTypedefs(scope, alias, source);
}

DDS::Boolean
be_typedef::IsPrimitiveType() const
{
   return m_baseBeType->IsPrimitiveType();
}

DDS::Boolean
be_typedef::IsEnumeratedType() const
{
   return m_baseBeType->IsEnumeratedType();
}

DDS::Boolean
be_typedef::IsStructuredType() const
{
   return m_baseBeType->IsStructuredType();
}

DDS::Boolean
be_typedef::IsStringType() const
{
   return m_baseBeType->IsStringType();
}

DDS::Boolean be_typedef::IsArrayType() const
{
   return m_baseBeType->IsArrayType();
}

DDS::Boolean be_typedef::IsSequenceType() const
{
   return m_baseBeType->IsSequenceType();
}

DDS::Boolean be_typedef::IsInterfaceType() const
{
   return m_baseBeType->IsInterfaceType();
}

DDS::Boolean be_typedef::IsValueType() const
{
   return m_baseBeType->IsValueType();
}

DDS::Boolean be_typedef::IsReturnedByVar () const
{  
   return m_baseBeType->IsReturnedByVar ();
}

DDS::Boolean
be_typedef::IsExceptionType() const
{
   return m_baseBeType->IsExceptionType();
}

DDS_StdString
be_typedef::Allocater(const DDS_StdString& var) const
{
   return m_baseBeType->Allocater(var);
}

DDS_StdString
be_typedef::Releaser(const DDS_StdString& var) const
{
   return m_baseBeType->Releaser(var);
}

DDS_StdString
be_typedef::Assigner(const DDS_StdString& lhs, const DDS_StdString& rhs) const
{
   return m_baseBeType->Assigner(lhs, rhs);
}

DDS_StdString be_typedef::Duplicater
(
   const DDS_StdString& arg,
   const DDS_StdString& val,
   const DDS_StdString& currentScope,
   const pbbool isConst
) const
{
   return m_baseBeType->Duplicater (arg, val, currentScope, isConst);
}

DDS_StdString
be_typedef::Initializer(const DDS_StdString& var, VarType vt) const
{
   AST_Type * basetype = ((be_typedef*)this)->base_type();
   be_enum * basetype_enum = 0;
   be_Type * type;
   DDS_StdString ret;

   assert(basetype && basetype->narrow((long)&be_Type::type_id));

   // This conditional statement here is kludgy.  See comment

   basetype_enum = (be_enum*)basetype->narrow((long) & be_enum::type_id);
   if (basetype_enum)
   {
      ret = var + " = (" + typeName + ")0;";
   }
   else if ((basetype && (type = (be_Type*)basetype->narrow((long) & be_Type::type_id))))
   {
      ret = type->Initializer(var, vt);
   }

   return ret;
}

DDS_StdString
be_typedef::NullReturnArg()
{
   return m_baseBeType->NullReturnArg();
}

DDS_StdString
be_typedef::SyncStreamOut(const DDS_StdString& arg, const DDS_StdString &out, VarType vt) const
{
   return m_baseBeType->SyncStreamOut(arg, out, vt);
}

DDS_StdString
be_typedef::SyncStreamIn(const DDS_StdString& arg, const DDS_StdString& in, VarType vt) const
{
   return m_baseBeType->SyncStreamIn(arg, in, vt);
}

DDS_StdString
be_typedef::StructStreamIn(const DDS_StdString& arg, const DDS_StdString &in) const
{
   return m_baseBeType->StructStreamIn(arg, in);
}

DDS_StdString
be_typedef::StructStreamOut(const DDS_StdString& arg, const DDS_StdString &out) const
{
   return m_baseBeType->StructStreamOut(arg, out);
}

void
be_typedef::GenerateStreamOps(be_ClientHeader&)
{}

be_typedef *
be_typedef::_narrow(AST_Type * atype)
{
   be_typedef * ret = 0;

   if (atype)
   {
      ret = (be_typedef*)atype->narrow((long) & be_typedef::type_id);
   }

   return ret;
}

AST_Type *
be_typedef::_astBase(AST_Type * ttype)
{
   AST_Type * ret = ttype;

   if (ret)
   {
      AST_Typedef * atd;

      while ((atd = (AST_Typedef*)ret->narrow((long) & AST_Typedef::type_id)))
      {
         ret = atd->base_type();
      }
   }

   return ret;
}

be_Type *
be_typedef::_beBase(AST_Type * ttype)
{
   AST_Type * tmp;

   tmp = _astBase(ttype);

   return be_Type::_narrow(tmp);
}

void
be_typedef::generate_tc_ctor_val(
   be_Source & source)
{}

void be_typedef::generate_tc_dtor_val
(
   be_Source & source,
   pbbool isCounted
)
{}

void
be_typedef::generate_tc_put_val(be_Source & source)
{}

void
be_typedef::generate_tc_get_val(be_Source & source)
{}


void
be_typedef::generate_tc_assign_val(
   be_Source & source)
{}


DDS_StdString
be_typedef::InRequestArgumentDeclaration(
   be_Type& btype,
   const DDS_StdString& arg,
   VarType vt)
{
   return m_baseBeType->InRequestArgumentDeclaration(btype, arg, vt);
}

DDS_StdString
be_typedef::UnionStreamOut( const DDS_StdString& arg,
                            const DDS_StdString& out) const
{
   return m_baseBeType->UnionStreamOut(arg, out);
}

DDS_StdString
be_typedef::UnionStreamIn( const DDS_StdString& arg,
                           const DDS_StdString& in) const
{
   return m_baseBeType->UnionStreamIn(arg, in);
}


DDS_StdString
be_typedef::kind_string()
{
   return m_baseBeType->kind_string();
}

DDS::ULong
be_typedef::get_elem_size()
{
   return m_baseBeType->get_elem_size();
}

DDS::ULong
be_typedef::get_elem_alignment()
{
   return m_baseBeType->get_elem_alignment();
}

DDS::Boolean be_typedef::declare_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & scope,
   VarType vt
)
{
   return m_baseBeType->declare_for_stub (os, tab, arg, scope, vt);
}

DDS::Boolean
be_typedef::declare_for_struct_put(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return m_baseBeType->declare_for_struct_put(os, tab, sptr, fld, uid);
}

DDS::Boolean
be_typedef::declare_for_struct_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return m_baseBeType->declare_for_struct_get(os, tab, sptr, fld, uid);
}

DDS::Boolean
be_typedef::make_get_param_for_stub(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & fld,
   VarType vt) const
{
   return m_baseBeType->make_get_param_for_stub(os, tab, fld, vt);
}

DDS::Boolean
be_typedef::make_put_param_for_stub(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & fld,
   VarType vt) const
{
   return m_baseBeType->make_put_param_for_stub(os, tab, fld, vt);
}


DDS::Boolean
be_typedef::make_put_param_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return m_baseBeType->make_put_param_for_struct(os, tab, sptr, fld, uid);
}

DDS::Boolean
be_typedef::make_get_param_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return m_baseBeType->make_get_param_for_struct(os, tab, sptr, fld, uid);
}

DDS::Boolean
be_typedef::make_put_param_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return m_baseBeType->make_put_param_for_union(os, tab, sptr, fld, uid);
}

DDS::Boolean
be_typedef::make_get_param_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return m_baseBeType->make_get_param_for_union(os, tab, sptr, fld, uid);
}

ostream &
be_typedef::put_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return m_baseBeType->put_for_struct(os, tab, sptr, fld, uid);
}

ostream &
be_typedef::get_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return m_baseBeType->get_for_struct(os, tab, sptr, fld, uid);
}

ostream & be_typedef::put_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   return m_baseBeType->put_for_union (os, tab, sptr, fld, uid);
}

ostream &
be_typedef::get_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return m_baseBeType->get_for_union(os, tab, sptr, fld, uid);
}

ostream &
be_typedef::put_for_sequence(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   return m_baseBeType->put_for_sequence(os, tab, arg, index, uid);
}

ostream &
be_typedef::get_for_sequence(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   return m_baseBeType->get_for_sequence(os, tab, arg, index, uid);
}

ostream &
be_typedef::put_for_array(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   return m_baseBeType->put_for_sequence(os, tab, arg, index, uid);
}

ostream &
be_typedef::get_for_array(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   return m_baseBeType->get_for_sequence(os, tab, arg, index, uid);
}

be_CppType
be_typedef::CppTypeWhenSequenceMember() const
{
   return m_baseBeType->CppTypeWhenSequenceMember();
}

be_DispatchableType::en_HowStoredInDispatcher
be_typedef::HowStoredInDispatcher(
   const be_ArgumentDirection& direction) const
{
   // YO BEN these next three lines are a temporary substitute for
   // declaring m_baseBeType as be_DispatchableType
   be_DispatchableType* pDispatchable = (be_DispatchableType*)
                                        m_baseBeType->narrow((long) & be_DispatchableType::type_id);
   assert(pDispatchable != 0);

   return pDispatchable->HowStoredInDispatcher(direction);
}

void
be_typedef::InitializeInDispatcher(
   ostream& os,
   be_Tab& tab,
   const be_CppName& argName,
   const be_ArgumentDirection& direction) const
{
   // YO BEN these next three lines are a temporary substitute for
   // declaring m_baseBeType as be_DispatchableType
   be_DispatchableType* pDispatchable = (be_DispatchableType*)
                                        m_baseBeType->narrow((long) & be_DispatchableType::type_id);
   assert(pDispatchable != 0);

   pDispatchable->InitializeInDispatcher(os, tab, argName, direction);
}

DDS::Boolean
be_typedef::is_core_marshaled()
{
   return m_baseBeType->is_core_marshaled();
}

