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
#include "xbe_type.h"
#include "xbe_globals.h"
#include "xbe_utils.h"
#include "xbe_predefined.h"
#include "xbe_array.h"
#include "xbe_sequence.h"

// uncommenting this outputs ascii rep for the TypeCode marshalling.
//#define DEBUG_TCREP

extern DDS_StdString NameToString(UTL_ScopedName* name, const char * sep);

VarType
MakeVarType(AST_Argument::Direction dir)
{
   switch (dir)
   {
      case AST_Argument::dir_IN :
         return VT_InParam;

      case AST_Argument::dir_OUT :
         return VT_OutParam;

      case AST_Argument::dir_INOUT :
         return VT_InOutParam;

      default :
      {
         assert(pbfalse);
         return VT_Return;
      }
   }
}

const pbbool VT_Const = pbtrue;
const pbbool VT_NonConst = pbfalse;
const pbbool VT_Reference = pbtrue;
const pbbool VT_NonReference = pbfalse;
const int VT_Var = 0;
const int VT_Pointer = 1;

// --------------------------------------------------------------------
//  BE_TYPEMAP IMPLEMENTATION
// --------------------------------------------------------------------
IMPL_NARROW_METHODS0(be_TypeMap)

void
be_TypeMap::VarSignature( VarType vt,
                          const DDS_StdString& varTypeName,
                          pbbool isConst,
                          int levelsOfIndirection,
                          pbbool isReference)
{
   _signatures[vt]._varTypeName = varTypeName;
   _signatures[vt]._isConst = isConst;
   _signatures[vt]._levelsOfIndirection = levelsOfIndirection;
   _signatures[vt]._isReference = isReference;
}

void
be_TypeMap::VarSignature(VarType vt, const be_VarSignature& signature)
{
   _signatures[vt] = signature;
}

const be_VarSignature&
be_TypeMap::VarSignature(VarType vt) const
{
   return _signatures[vt];
}

DDS_StdString
be_TypeMap::VarTypeName(VarType vt) const
{
   return _signatures[vt]._varTypeName;
}

pbbool
be_TypeMap::IsReference(VarType vt) const
{
   return _signatures[vt]._isReference;
}

int
be_TypeMap::LevelsOfIndirection(VarType vt) const
{
   return _signatures[vt]._levelsOfIndirection;
}

pbbool
be_TypeMap::IsConst(VarType vt) const
{
   return _signatures[vt]._isConst;
}

DDS_StdString
be_TypeMap::MakeSignature(VarType vt, const DDS_StdString& className) const
{
   DDS_StdString ret;

   if (IsConst(vt))
   {
      ret = "const ";
   }

   ret += BE_Globals::RelativeScope(className, VarTypeName(vt));

   for (int i = 0; i < LevelsOfIndirection(vt); i++)
   {
      ret += "*";
   }

   if (IsReference(vt))
   {
      ret += "&";
   }

   return ret;
}


// --------------------------------------------------------------------
//  BE_TYPE IMPLEMENTATION
// --------------------------------------------------------------------
IMPL_NARROW_METHODS1(be_Type, be_TypeMap)

be_Type::be_Type ()
:
   m_typecode (new ProtoTypeCode ()),
   generated (pbfalse),
   _hasTypeDef (pbfalse)
{
}

pbbool
be_Type::IsObjectType()
{
   be_predefined_type * pdt = (be_predefined_type *)narrow((long) & be_predefined_type::type_id);
   return (pdt && ( pdt->pt() == AST_PredefinedType::PT_object));
}

pbbool
be_Type::IsLocalObjectType()
{
   be_predefined_type * pdt = (be_predefined_type *)narrow((long) & be_predefined_type::type_id);
   return (pdt && ( pdt->pt() == AST_PredefinedType::PT_local_object));
}

pbbool
be_Type::IsTypeCodeType()
{
   be_predefined_type * pdt = (be_predefined_type *)narrow((long) & be_predefined_type::type_id);
   return (pdt && ( pdt->pt() == AST_PredefinedType::PT_typecode));
}

pbbool
be_Type::IsPseudoObject()
{
   be_predefined_type * pdt = (be_predefined_type *)narrow((long) & be_predefined_type::type_id);
   return (pdt && ( pdt->pt() == AST_PredefinedType::PT_pseudo ));
}

DDS_StdString be_Type::Scope (const DDS_StdString& name) const
{
   DDS_StdString ret = enclosingScope;
   DDS_StdString sep = "::";

   if (ret.length ())
   {
      ret += sep;
   }

   return ret + name;
}

DDS_StdString be_Type::ScopedName () const
{
   return Scope (localName);
}

DDS_StdString
be_Type::TypedefName()
{
   DDS_StdString noColons = (const char *)ScopedName(); // make own copy

   if (strchr((char *)ScopedName(), ':'))
   {
      ColonColonToBar((char *)noColons);
   }

   return noColons;
}

DDS_StdString
be_Type::EnclosingScopeString(AST_Decl * decl)
{
   DDS_StdString ret;

   assert(decl);

   if (decl)
   {
      UTL_Scope * enclosingScope = decl->defined_in();
      AST_Decl * enclosingDecl;

      if (enclosingScope)
      {
         if ((enclosingDecl = (AST_Decl*)enclosingScope->narrow((long) & AST_Decl::type_id)))
         {
            ret = NameToString(enclosingDecl->name(), 0);
         }
         else
         {
            DDSError((DDS_StdString)"Can't narrow enclosing scope for " + NameToString(decl->name(), 0));
         }
      }
   }

   return ret;
}

be_Type *
be_Type::_narrow(AST_Type * atype)
{
   be_Type * ret = 0;

   if (atype)
   {
      ret = (be_Type*)atype->narrow((long) & be_Type::type_id);
   }

   return ret;
}

DDS_StdString
be_Type::SyncStreamOut(const DDS_StdString& arg, const DDS_StdString& out, VarType /*vt*/) const
{
   return out + " << " + arg + ";";
}

DDS_StdString
be_Type::SyncStreamIn(const DDS_StdString& arg, const DDS_StdString& in, VarType /*vt*/) const
{
   return in + " >> " + arg + ";";
}

DDS_StdString
be_Type::StructStreamOut(const DDS_StdString& arg, const DDS_StdString& out) const
{
   return out + " << " + arg + ";";
}

DDS_StdString
be_Type::StructStreamIn(const DDS_StdString& arg, const DDS_StdString& in) const
{
   return in + " >> " + arg + ";";
}

DDS_StdString
be_Type::UnionStreamOut(const DDS_StdString& arg, const DDS_StdString& out) const
{
   return StructStreamOut(arg, out);
}

DDS_StdString
be_Type::UnionStreamIn(const DDS_StdString& arg, const DDS_StdString& in) const
{
   return StructStreamIn(arg, in);
}

void be_Type::GenerateGlobalTypedef (be_ClientHeader & source)
{
}

void be_Type::GeneratePutGetOps (be_ClientHeader& source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   // we have to generate even core marshaled ops
   // for use by dependent types, e.g., an
   // array of unions needs the union's ops declared

   // first putval and getval

   os << tab << DLLMACRO << "void " << m_tc_put_val << nl;
   os << tab << "(" << nl;
   tab.indent ();
   os << tab << "DDS::Codec::OutStream & os," << nl;
   os << tab << "const void * arg," << nl;
   os << tab << "DDS::ParameterMode mode" << nl;
   if (XBE_Ev::generate ())
   {
      os << tab << XBE_Ev::arg (XBE_ENV_ARGN, false) << nl;
   }
   tab.outdent ();
   os << tab << ");" << nl << nl;
   os << tab << DLLMACRO << "void " << m_tc_get_val << nl;
   os << tab << "(" << nl;
   tab.indent ();
   os << tab << "DDS::Codec::InStream & is," << nl;
   os << tab << "void * arg," << nl;
   os << tab << "DDS::ParameterMode mode" << nl;
   if (XBE_Ev::generate ())
   {
      os << tab << XBE_Ev::arg (XBE_ENV_ARGN, false) << nl;
   }
   tab.outdent ();
   os << tab << ");" << nl << nl;
}

void be_Type::GenerateStreamOps (be_ClientHeader& source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   // insertion

   os << tab << DLLMACRO << "inline void" << nl
   << "IOP::put(DDS::Codec::OutStream& os, const "
   << ScopedName() << "& v" << XBE_Ev::arg (XBE_ENV_ARGN) << ")" << nl;
   os << "{" << nl;
   tab.indent();
   os << tab << "DDS::Codec::Param putArg = ";
   os << "{ " << Scope(TypeCodeTypeName()) << ", ";
   os << "(" << TypeName() << "*)" << "&v, DDS::PARAM_IN ";
   os << "};" << nl;
   os << tab << "os.put (&putArg, 1" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent();
   os << "}" << nl << nl;

   //
   // extraction
   //
   os << DLLMACRO << "inline void" << nl
   << "IOP::get(DDS::Codec::InStream& is, "
   << ScopedName() << "& v" << XBE_Ev::arg (XBE_ENV_ARGN) << ")" << nl;
   os << "{" << nl;
   tab.indent();
   os << tab << "DDS::Codec::Param getArg = ";
   os << "{ " << Scope(TypeCodeTypeName()) << ", ";
   os << "&v, DDS::PARAM_OUT };" << nl;
   os << tab << "is.get (&getArg, 1" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent();
   os << "}" << nl << nl;
}

DDS_StdString
be_Type::InRequestArgumentDeclaration(be_Type& btype, const DDS_StdString& arg, VarType vt)
{
   DDS_StdString ret = btype.TypeName();

   if (!IsFixedLength() && (vt == VT_Return || vt == VT_OutParam))
   {
      ret += "_var";
   }

   return ret + " " + arg + ";";
}

DDS::ULong
be_Type::get_elem_size()
{
   return 0;
}

DDS::ULong
be_Type::get_elem_alignment()
{
   return 0;
}

DDS::ULong
be_Type::get_OS_elem_alignment()
{
   return get_elem_alignment();
}

const DDS_StdString & be_Type::any_op_id ()
{
   return m_any_op_id;
}

DDS::Boolean be_Type::declare_for_stub
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
      os << tab << TypeName () << " " << arg << ";" << nl;
   }

   return TRUE;
}

DDS::Boolean be_Type::declare_for_struct_put
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   return FALSE;
}

DDS::Boolean be_Type::declare_for_struct_get
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   return FALSE;
}

DDS::Boolean be_Type::declare_for_union_put
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   return declare_for_struct_put (os, tab, sptr, fld, uid);
}

DDS::Boolean be_Type::declare_for_union_get
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   return declare_for_struct_get (os, tab, sptr, fld, uid);
}

ostream & be_Type::put_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   os << tab << "DDS::Codec::Param putArg" << uid << " = { " << Scope(TypeCodeTypeName())
      << ", &" << arg << "[" << index << "], DDS::PARAM_IN };" << nl << nl;
   os << tab << "os.put (&putArg" << uid << ", 1" 
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_Type::get_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   os << tab << "DDS::Codec::Param getArg" << uid << " = { " << Scope(TypeCodeTypeName())
      << ", &" << arg << "[" << index << "], DDS::PARAM_OUT };" << nl << nl;
   os << tab << "is.get (&getArg" << uid << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_Type::put_for_array
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   os << tab << "DDS::Codec::Param putArg" << uid << " = { " << Scope(TypeCodeTypeName())
      << ", &" << arg << "[" << index << "], DDS::PARAM_IN };" << nl << nl;
   os << tab << "os.put(&putArg" << uid << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_Type::get_for_array
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   os << tab << "DDS::Codec::Param getArg" << uid << " = { " << Scope(TypeCodeTypeName())
      << ", &" << arg << "[" << index << "], DDS::PARAM_OUT };" << nl << nl;
   os << tab << "is.get (&getArg" << uid << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_Type::put_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   // declare the put param

   os << tab << "DDS::Codec::Param put" << fld << uid << " = ";

   // populate it

   make_put_param_for_struct (os, tab, sptr, fld, uid);

   // finish the declaration

   os << ";" << nl << nl;

   // call put

   os << tab << "os.put (&put" << fld << uid << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_Type::get_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   // declare the get param

   os << tab << "DDS::Codec::Param get" << fld << uid << " = ";

   // populate it

   make_get_param_for_struct(os, tab, sptr, fld, uid);

   // finish the declaration

   os << ";" << nl << nl;

   // call get

   os << tab << "is.get(&get" << fld << uid 
      << ", 1" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

DDS::Boolean be_Type::make_get_param_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & argname,
   VarType vt
) const
{
   DDS::Boolean ret = FALSE;

   if (vt == VT_InOutParam || vt == VT_OutParam || vt == VT_Return)
   {
      os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";

      switch (vt)
      {
         case VT_InOutParam:
         os << "&" << argname << ", DDS::PARAM_INOUT ";
         break;

         case VT_OutParam:
         os << "&" << argname << ", DDS::PARAM_OUT ";
         break;

         case VT_Return:
         os << "&" << argname << ", DDS::PARAM_OUT ";
         break;

         default:
         assert (0);
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_Type::make_put_param_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & argname,
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
         os << "(void*)&" << argname << ", DDS::PARAM_IN ";
         break;

         case VT_InOutParam:
         os << "(void*)&" << argname << ", DDS::PARAM_INOUT ";
         break;

         default:
         assert (0);
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_Type::make_put_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   assert(0); // this method must always be overridden by descendant
   return FALSE;
}

DDS::Boolean be_Type::make_get_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   assert(0); // this method must always be overridden by descendant
   return FALSE;
}


DDS::Boolean be_Type::make_put_param_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   assert(0); // this method must always be overridden by descendant
   return FALSE;
}

DDS::Boolean
be_Type::make_get_param_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   assert(0); // this method must always be overridden by descendant
   return FALSE;
}


void be_Type::generate_tc_ctor_val (be_Source & source)
{
   ostream & os = source.Stream ();

   // declare ctor body
   os << "static void * " << m_tc_ctor_val << " ()" << nl;
   os << "{" << nl;
   os << "   return new " << ScopedName () << ";" << nl;
   os << "}" << nl << nl;
}


void be_Type::generate_tc_dtor_val (be_Source & source, pbbool isCounted)
{
   ostream & os = source.Stream ();

   // declare dtor body
   os << "static void " << m_tc_dtor_val << " (void * arg)" << nl;
   os << "{" << nl;

   // Cast and delete as appropriate

   if (isCounted)
   {
      os << "   DDS::release";
   }
   else
   {
      os << "   delete"; 
   }
   os << " ((" << ScopedName() << "*) arg);" << nl;
   os << "}" << nl << nl;
}


void
be_Type::generate_tc_put_val(
   be_Source & source)
{}


void
be_Type::generate_tc_get_val(
   be_Source & source)
{}


void be_Type::generate_tc_assign_val (be_Source & source)
{
   ostream & os = source.Stream ();

   // declare assign body

   os << "static void " << m_tc_assign_val << " (void * dest, void * src)" << nl;
   os << "{" << nl;

   // just cast and assign

   os << "   *(" << ScopedName () << "*) dest = *(" << ScopedName () << "*) src;" << nl;
   os << "}" << nl << nl;
}

void be_Type::FinishProtoTypeCode ()
{}
