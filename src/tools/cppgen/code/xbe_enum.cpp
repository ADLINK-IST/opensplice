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
#include "xbe_enum.h"
#include "xbe_root.h"
#include "xbe_utils.h"
#include "xbe_typedef.h"

#if defined(_WIN32)
#include <direct.h>
#endif

// -------------------------------------------------
//  BE_ENUM IMPLEMENTATIN
// -------------------------------------------------
IMPL_NARROW_METHODS4(be_enum, AST_Enum, be_CodeGenerator, be_DispatchableType,
                     be_Type)
IMPL_NARROW_FROM_DECL(be_enum)
IMPL_NARROW_FROM_SCOPE(be_enum)

be_enum::be_enum()
{
   isAtModuleScope(pbfalse); 
}

be_enum::be_enum (UTL_ScopedName *n, const UTL_Pragmas &p)
:
   AST_Decl (AST_Decl::NT_enum, n, p),
   UTL_Scope (AST_Decl::NT_enum, n, p),
   AST_Enum (n, p)
{
   isAtModuleScope(pbfalse);
   DDS_StdString barScopedName = NameToString(name(), "_");

   m_swapCall = "SWAP32";
   localName = local_name()->get_string();
   enclosingScope = be_Type::EnclosingScopeString(this);
   m_typecode->kind = DDS::tk_enum;
   m_typecode->id = get_decl_pragmas().get_repositoryID()->get_string();
   m_typecode->name_of_type = localName;
   m_tc_put_val = (DDS_StdString) "DDS_put_" + barScopedName + "_param";
   m_tc_get_val = (DDS_StdString) "DDS_get_" + barScopedName + "_param";
   m_any_op_id = barScopedName;
   InitializeTypeMap(this);
   be_root::AddTypeThatNeedsProtoTypeCodeFinished(*this);
}

void be_enum::InitializeTypeMap(be_Type* t)
{
   idlType = t;

   if (t)
   {
      AST_Type* t_ast = (AST_Type*)t->narrow((long) & AST_Type::type_id);

      assert(t_ast);

      t->TypeName(NameToString(t_ast->name()));
      t->InTypeName(t->TypeName());
      t->InOutTypeName(t->TypeName() + "&");
      t->OutTypeName(t->TypeName() + "&");
      t->ReturnTypeName(t->TypeName());
      t->DMFAdtMemberTypeName(t->TypeName());
      t->StructMemberTypeName(t->TypeName());
      t->UnionMemberTypeName(t->TypeName());
      t->SequenceMemberTypeName(t->TypeName());

      t->VarSignature(VT_InParam, t->TypeName(), VT_NonConst, VT_Var, VT_NonReference);
      t->VarSignature(VT_InOutParam, t->TypeName(), VT_NonConst, VT_Var, VT_Reference);
      t->VarSignature(VT_OutParam, t->TypeName(), VT_NonConst, VT_Var, VT_Reference);
      t->VarSignature(VT_Return, t->TypeName(), VT_NonConst, VT_Var, VT_NonReference);

      t->TypeCodeTypeName(BE_Globals::TCPrefix + t->LocalName());

      DDS_StdString scopebar = NameToString(t_ast->name(), "_");
      t->TypeCodeBaseName(BE_Globals::TCBasePrefix + scopebar);
      t->TypeCodeRepName(BE_Globals::TCRepPrefix + scopebar);
      t->MetaTypeTypeName(BE_Globals::MTPrefix + scopebar);
   }
   else
   {
      assert(0);
   }
}

pbbool
be_enum::IsFixedLength() const
{
   return pbtrue;
}

pbbool
be_enum::IsFixedLengthPrimitiveType() const
{
   return pbfalse;
}

void
be_enum::GenerateTypedefs( const DDS_StdString& scope,
                           const be_typedef& alias,
                           be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   DDS_StdString relTypeName = BE_Globals::RelativeScope(scope, typeName);

   os << tab << "typedef " << (const char*)relTypeName << " "
   << alias.LocalName() << ";" << nl;
}

void
be_enum::GenerateType(be_ClientHeader& source)
{
   Generate(source);
}

DDS_StdString
be_enum::Allocater(const DDS_StdString& arg) const
{
   DDS_StdString ret = arg + ";";

   return ret;
}

DDS_StdString
be_enum::Initializer(const DDS_StdString& arg, VarType) const
{
   return arg + " = (" + typeName + ")0;";
}

DDS_StdString
be_enum::Releaser(const DDS_StdString& arg) const
{
   return (DDS_StdString)"delete(" + arg + ");";
}

DDS_StdString
be_enum::Assigner(const DDS_StdString& arg, const DDS_StdString& val) const
{
   return arg + " = " + val + ";";
}

DDS_StdString be_enum::Duplicater
(
   const DDS_StdString &,
   const DDS_StdString &,
   const DDS_StdString &,
   const pbbool
) const
{
   // enums should not be duplicated.
   assert(pbfalse);
   return (DDS_StdString)"";
}


DDS_StdString
be_enum::NullReturnArg()
{
   return (DDS_StdString)"(" + TypeName() + ")0";
}

void
be_enum::Generate(be_ClientHeader& source)
{
   if (!Generated())
   {
      UTL_ScopeActiveIterator* i = new UTL_ScopeActiveIterator(this, IK_decls);
      ostream & os = source.Stream();
      unsigned long expectedValue = 0;
      be_Tab tab(source);
      AST_Decl * d;

      Generated(pbtrue);

      os << tab << "enum " << *local_name() << nl;
      os << tab << "{" << nl;
      tab.indent();

      while (!(i->is_done()))
      {
         d = i->item ();
         if (d)
         {
            be_enum_val * ev = (be_enum_val*) d->narrow ((long) & be_enum_val::type_id);

            if (ev)
            {
               os << tab << *ev->local_name();

               if (ev->Value() != expectedValue)
               {
                  expectedValue = ev->Value();
                  os << " = " << expectedValue;
               }

               expectedValue++;

               i->next();

               if (!(i->is_done()))
               {
                  os << ",";
               }

               os << nl;
            }
            else
            {
               DDSError(NameToString(name()) + " contains non-enum val");
               assert(pbfalse);
            }
         }
         else
         {
            DDSError(NameToString(name()) + " declaration is corrupted");
            assert(pbfalse);
         }
      }

      //os << tab << "DDS_DCPS_FORCE_ENUM32(__" << *local_name() << ")" << nl;

      tab.outdent();
      os << tab << "};" << nl;

      delete i;

      // GENERATE STREAMING OPERATORS
      be_root::AddAnyOps(*this);
      be_root::AddPutGetOps(*this);
      be_root::AddStreamOps(*this);
      be_root::AddTypedef(*this);
      be_root::AddTypecode(*this);
   }
}

be_enum *
be_enum::_narrow(AST_Type * atype)
{
   be_enum * ret = 0;

   if (atype)
   {
      ret = (be_enum*)atype->narrow((long) & be_enum::type_id);
   }

   return ret;
}

void
be_enum::FinishProtoTypeCode()
{
   // flesh out typecode
   // since enums names are all in a scope and not added directly
   // we go through the scope and add them all here
   UTL_Scope* s = (UTL_Scope*)narrow((long) & UTL_Scope::type_id);
   assert(s);

   UTL_ScopeActiveIterator* i = 0;
   i = new UTL_ScopeActiveIterator(s, UTL_Scope::IK_decls);

   if (s->nmembers() > 0)
   {
      for ( ; !(i->is_done()); i->next())
      {
         AST_Decl* d = i->item();
         assert(d);

         m_typecode->member_names.push_back(d->local_name()->get_string());
      }

      delete i;
   }
}

DDS_StdString be_enum::kind_string ()
{
   return "DDS::tk_enum";
}

DDS::ULong be_enum::get_elem_size ()
{
   return 4;
}

DDS::ULong be_enum::get_elem_alignment ()
{
   return 4;
}

// -------------------------------------------------
//  BE_ENUM_VAL IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS1(be_enum_val, AST_EnumVal)
IMPL_NARROW_FROM_DECL(be_enum_val)
be_enum_val::be_enum_val()
{}

be_enum_val::be_enum_val
   (unsigned long v, UTL_ScopedName *n, const UTL_Pragmas &p)
: 
   AST_Decl (AST_Decl::NT_enum_val, n, p),
   AST_Constant
   (
      AST_Expression::EV_ulong,
      AST_Decl::NT_enum_val,
      new AST_Expression(v),
      n,
      p
   )
{}

unsigned long be_enum_val::Value ()
{
   AST_Expression * enumExpr;
   AST_Expression::AST_ExprValue * exprValue;

   enumExpr = constant_value ();
   if (enumExpr)
   {
      exprValue = enumExpr->eval (AST_Expression::EK_positive_int);
      if (exprValue)
      {
         assert (exprValue->et == AST_Expression::EV_ulong);
         return exprValue->u.eval;
      }
      else
      {
         assert(pbfalse);
      }
   }
   else
   {
      assert(pbfalse);
   }

   return 0;
}

DDS::Boolean
be_enum::is_core_marshaled()
{
   return TRUE;
}

DDS::Boolean be_enum::make_get_param_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   VarType vt
) const
{
   DDS::Boolean ret = FALSE;

   if (vt == VT_InOutParam || vt == VT_OutParam || vt == VT_Return)
   {
      os << tab << "{ " << Scope (TypeCodeTypeName()) << ", ";

      switch (vt)
      {
         case VT_InOutParam:
         os << "&" << arg << ", DDS::PARAM_INOUT ";
         break;

         case VT_OutParam:
         os << "&" << arg << ", DDS::PARAM_OUT ";
         break;

         case VT_Return:
         os << "&" << arg << ", DDS::PARAM_OUT ";
         break;

         default: assert (0);
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_enum::make_put_param_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   VarType vt
) const
{
   DDS::Boolean ret = FALSE;

   if (vt == VT_InParam || vt == VT_InOutParam)
   {
      os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";

      switch (vt)
      {
         case VT_InParam:
         os << "(void*)&" << arg << ", DDS::PARAM_IN ";
         break;

         case VT_InOutParam:
         os << "&" << arg << ", DDS::PARAM_INOUT ";
         break;

         default: assert (0);
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean
be_enum::make_put_param_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   os << tab << "{ " << Scope(TypeCodeTypeName()) << ", "
   << "(void*)&" << sptr << "->" << fld << ", DDS::PARAM_IN "
   << "}";

   return TRUE;
}

DDS::Boolean
be_enum::make_get_param_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   os << tab << "{ " << Scope(TypeCodeTypeName()) << ", "
   << "&" << sptr << "->" << fld << ", DDS::PARAM_OUT "
   << "}";

   return TRUE;
}

DDS::Boolean
be_enum::make_put_param_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   os << tab << "{ " << Scope(TypeCodeTypeName()) << ", "
   << "(void*)&" << sptr << "->" << fld << ", DDS::PARAM_IN "
   << "}";

   return TRUE;
}

DDS::Boolean
be_enum::make_get_param_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   os << tab << "{ " << Scope(TypeCodeTypeName()) << ", "
   << "&" << sptr << "->" << fld << ", DDS::PARAM_OUT "
   << "}";

   return TRUE;
}

DDS::Boolean
be_enum::declare_for_struct_put(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return FALSE;
}

DDS::Boolean
be_enum::declare_for_struct_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return FALSE;
}

ostream & be_enum::put_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   tab.indent ();
   os << tab << "os.cdr_put (*(DDS::ULong*)&" << sptr
      << "->" << fld << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent ();

   return os;
}

ostream & be_enum::get_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   tab.indent ();
   os << tab << "is.cdr_get (*(DDS::ULong*)&" << sptr
      << "->" << fld << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent ();

   return os;
}

ostream &
be_enum::put_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return put_for_struct(os, tab, sptr, fld, uid);
}

ostream &
be_enum::get_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return get_for_struct(os, tab, sptr, fld, uid);
}

ostream & be_enum::put_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   // declare the put param

   os << tab << "DDS::Codec::Param put" << arg << uid << " = ";

   // populate it

   os << "{ " << Scope(TypeCodeTypeName()) << ", ";
   os << "(void*)&" << arg << "[" << index << "], DDS::PARAM_IN ";
   os << "}";
   os << ";" << nl << nl;

   // call put

   os << tab << "os.put (&put" << arg << uid << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_enum::get_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   // declare the get param

   os << tab << "DDS::Codec::Param get" << arg << uid << " = ";

   // populate it

   os << "{ " << Scope(TypeCodeTypeName()) << ", ";
   os << "&" << arg << "[" << index << "], DDS::PARAM_OUT ";
   os << "}";
   os << ";" << nl << nl;

   // call get

   os << tab << "is.get(&get" << arg << uid << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_enum::put_for_array
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   // declare the put param

   os << tab << "DDS::Codec::Param put = ";

   // populate it

   os << "{ " << Scope(TypeCodeTypeName()) << ", ";
   os << "(void*)&" << arg << "[" << index << "], DDS::PARAM_IN ";
   os << "}";
   os << ";" << nl << nl;

   // call put

   os << tab << "os.put (&put, 1" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_enum::get_for_array
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   // declare the get param

   os << tab << "DDS::Codec::Param get  = ";

   // populate it

   os << "{ " << Scope(TypeCodeTypeName()) << ", ";
   os << "&" << arg << "[" << index << "], DDS::PARAM_OUT ";
   os << "}";
   os << ";" << nl << nl;

   // call get

   os << tab << "is.get (&get, 1" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

void be_enum::generate_tc_ctor_val (be_Source & source)
{}

void be_enum::generate_tc_dtor_val
(
   be_Source & source,
   pbbool isCounted
)
{}

void be_enum::generate_tc_put_val (be_Source & source)
{}

void be_enum::generate_tc_get_val (be_Source & source)
{}

void be_enum::generate_tc_assign_val (be_Source & source)
{}


DDS::Boolean be_enum::declare_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & stubScope,
   VarType vt
)
{
   if (vt == VT_Return)
   {
      os << " " << TypeName () << " " << arg << ";" << nl;
   }

   return TRUE;
}

be_DispatchableType::en_HowStoredInDispatcher
be_enum::HowStoredInDispatcher(const be_ArgumentDirection&) const
{
   return STORED_AS_STACK_VARIABLE;
}

DDS_StdString
be_enum::swap_call(const DDS_StdString & arg)
{
   DDS_StdString ret = "(" + NoColons(ScopedName()) + ")" + m_swapCall + "(*" + arg + ")";

   return ret;
}
