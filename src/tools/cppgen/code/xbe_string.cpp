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
#ifdef SCCSID
static char SCCSid[] = "%W% %G%";
#endif

#include "idl.h"
#include "idl_extern.h"
#include "xbe_string.h"
#include "xbe.h"
#include "xbe_literals.h"
#include "xbe_typedef.h"

// -------------------------------------------------
//  BE_STRING IMPLEMENTATION
// -------------------------------------------------

IMPL_NARROW_METHODS3(be_string, AST_String, be_DispatchableType, be_Type)
IMPL_NARROW_FROM_DECL(be_string)

be_string::be_string ()
: m_wide(0)
{
}

be_string::be_string (AST_Expression *v)
:
   AST_Decl
   (
      AST_Decl::NT_string,
      new UTL_ScopedName (new Identifier("string", 1, 0, I_FALSE), NULL)
   ),
   AST_String (v),
   m_wide(0)
{
   localName = "DDS::String";
   m_typecode->kind = DDS::tk_string;
   m_typecode->id = "DDS::String";
   m_any_op_id = m_typecode->id;
   InitializeTypeMap(this);
}

be_string::be_string (AST_Expression *v, long wide)
:
   AST_Decl
   (
      AST_Decl::NT_string,
      (wide == 1)
         ? new UTL_ScopedName (new Identifier ("string", 1, 0, I_FALSE), NULL)
         : new UTL_ScopedName (new Identifier("wstring", 1, 0, I_FALSE), NULL)
   ),
   AST_String (v, wide),
   m_wide(wide)
{
   if (wide)
   {
      m_typecode->kind = DDS::tk_wstring;
      m_typecode->id = "DDS::WString";
      localName = "DDS::WString";
   }
   else
   {
      m_typecode->kind = DDS::tk_string;
      m_typecode->id = "DDS::String";
      localName = "DDS::String";
   }

   InitializeTypeMap (this);
}

void be_string::InitializeTypeMap (be_Type* t)
{
   idlType = t;

   AST_Expression* maxsize = 0;
   be_string* t_string = 0;
   be_typedef* t_typedef = 0;
   DDS_StdString corbaString;
   DDS_StdString stringInOut;
   DDS_StdString stringOut;
   DDS_StdString structStringVar;

   if (m_typecode->kind == DDS::tk_string)
   {
      corbaString = BE_Globals::CorbaScope("String");
      stringInOut = BE_Globals::CorbaScope("String&");
      stringOut = BE_Globals::CorbaScope("String_out");
      structStringVar = (BE_Globals::isocpp_new_types ? "std::string" : BE_Globals::CorbaScope ("String_mgr"));
   }
   else
   {
      corbaString = BE_Globals::CorbaScope ("WString");
      stringInOut = BE_Globals::CorbaScope ("WString&");
      stringOut = BE_Globals::CorbaScope ("WString_out");
      structStringVar = BE_Globals::CorbaScope ("WString_var");
   }

   char size[10];
   os_sprintf (size, "%d", (int) ExprToULong(maxsize));

   t_typedef = (be_typedef*)t->narrow((long) & be_typedef::type_id);
   if (t_typedef)
   {
      AST_Type * t_ast = (AST_Type*)t->narrow((long) & AST_Type::type_id);

      AST_Type * basetype;
      AST_String * realbasetype;

      (void) t_ast;
      assert (t_ast);

      basetype = be_typedef::_astBase(((AST_Typedef*)t->narrow((long) & AST_Typedef::type_id))->base_type());
      assert(basetype);

      realbasetype = (AST_String*)basetype->narrow((long) & AST_String::type_id);
      assert(realbasetype);

      maxsize = realbasetype->max_size();
      corbaString = t->Scope(t->LocalName());
      stringInOut = corbaString + "&";
      stringOut = corbaString + DDSOutExtension;
      t->TypeCodeTypeName(BE_Globals::TCPrefix + t->LocalName());
      //  t->MetaTypeTypeName(BE_Globals::MTPrefix + NameToString(t_ast->name(),"_"));
      t->MetaTypeTypeName("xps_mt_DDS::String");
   }
   else
   {
      t_string = (be_string*)t->narrow((long) & be_string::type_id);
      assert(t_string);
      maxsize = t_string->max_size();
      // YO JFG 2/14/99 modified this
      // t->TypeCodeTypeName((DDS_StdString)"DDSTypeCodeFactory::createTypeCode(DDS::tk_string," + size + ")");

      if (m_typecode->kind == DDS::tk_string)
      {
         t->TypeCodeTypeName ("DDS::_tc_string");
      }
      else
      {
         t->TypeCodeTypeName ("DDS::_tc_wstring");
      }
   }

   t->TypeName(corbaString);

   if (m_typecode->kind == DDS::tk_string)
   {
      t->InTypeName ((DDS_StdString)"const char *"); // NOTE: != const corbaString;
   }
   else
   {
      t->InTypeName ((DDS_StdString)"const WChar *"); // NOTE: != const corbaString;
   }

   t->InOutTypeName (stringInOut);
   t->OutTypeName (stringOut);
   t->ReturnTypeName (stringOut);
   t->DMFAdtMemberTypeName (corbaString);
   t->StructMemberTypeName (structStringVar);
   if (BE_Globals::isocpp_new_types)
   {
      t->UnionMemberTypeName (structStringVar);
      t->SequenceMemberTypeName (structStringVar);
   }
   else
   {
      t->UnionMemberTypeName (corbaString);
      t->SequenceMemberTypeName (corbaString);
   }

   if (m_typecode->kind == DDS::tk_string)
   {
      t->VarSignature(VT_InParam, "DDS::Char", VT_Const, VT_Pointer, VT_NonReference);
   }
   else
   {
      t->VarSignature(VT_InParam, "DDS::WChar", VT_Const, VT_Pointer, VT_NonReference);
   }

   t->VarSignature (VT_InOutParam, stringInOut, VT_NonConst, VT_Var, VT_NonReference);
   t->VarSignature (VT_OutParam, stringOut, VT_NonConst, VT_Var, VT_NonReference);
   t->VarSignature (VT_Return, t->TypeName(), VT_NonConst, VT_Var, VT_NonReference);
}

pbbool be_string::IsFixedLength () const
{
   return pbfalse;
}

pbbool be_string::IsFixedLengthPrimitiveType () const
{
   return pbfalse;
}

void be_string::GenerateTypedefs
(
   const DDS_StdString & /*scope*/ ,
   const be_typedef& alias,
   be_ClientHeader& source
)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   if (BE_Globals::isocpp_new_types)
   {
      os << tab << "typedef ::std::string " << alias.LocalName() << ";" << nl;
   }
   else if (m_typecode->kind == DDS::tk_string)
   {
      // YO BEN should read "DDS::Char*" from some central place; violates OAOO
      os << tab << "typedef DDS::Char* " << alias.LocalName() << ";" << nl;
      os << tab << "typedef "
      << (const char*)BE_Globals::CorbaScope("String_var") << " "
      << alias.LocalName() << DDSVarExtension << ";" << nl;
      os << tab << "typedef "
      << (const char*)BE_Globals::CorbaScope("String_out") << " "
      << alias.LocalName() << DDSOutExtension << ";" << nl;
   }
   else
   {
      // YO BEN should read "DDS::WChar*" from some central place; violates OAOO
      os << tab << "typedef DDS::WChar* " << alias.LocalName() << ";" << nl;
      os << tab << "typedef "
      << (const char*)BE_Globals::CorbaScope("WString_var") << " "
      << alias.LocalName() << DDSVarExtension << ";" << nl;
      os << tab << "typedef "
      << (const char*)BE_Globals::CorbaScope("WString_out") << " "
      << alias.LocalName() << DDSOutExtension << ";" << nl;
   }
}

DDS_StdString be_string::Allocater (const DDS_StdString& arg) const
{
   return Initializer (arg, VT_OutParam);
}

DDS_StdString be_string::Initializer (const DDS_StdString& arg, VarType vartype) const
{
   DDS_StdString ret;

   if (vartype == VT_Return)
   {
      if (m_typecode->kind == DDS::tk_string)
         ret = arg + "(DDS::string_nil);";
      else
         ret = arg + "(DDS::wstring_nil);";
   }
   else
   {
      if (m_typecode->kind == DDS::tk_string)
         ret = arg + " = DDS::string_nil;";
      else
         ret = arg + " = DDS::wstring_nil;";
   }

   return ret;
}

DDS_StdString
be_string::InRequestArgumentDeclaration(be_Type& btype, const DDS_StdString& arg, VarType vt)
{

   DDS_StdString ret = btype.TypeName() + "_var" + " " + arg + ";";
   return ret;
}

DDS_StdString be_string::Releaser (const DDS_StdString & arg) const
{
   if (BE_Globals::isocpp_new_types)
   {
      DDS_StdString str("delete ");
      return str + arg + ";";
   }
   if (m_typecode->kind == DDS::tk_string)
   {
      return BE_Globals::CorbaScope ("string_free") + "(" + arg + ");";
   }
   else
   {
      return BE_Globals::CorbaScope ("wstring_free") + "(" + arg + ");";
   }
}

DDS_StdString
be_string::Assigner(const DDS_StdString& arg, const DDS_StdString& val) const
{
   DDS_StdString ret = arg + " = " + val + ";";

   return ret;
}

DDS_StdString be_string::Duplicater
(
   const DDS_StdString & arg,
   const DDS_StdString & val,
   const DDS_StdString &,
   const pbbool isConst
) const
{
   DDS_StdString ret;

   if (isConst)
   {
      if (BE_Globals::isocpp_new_types)
      {
         ret = arg + " = new std::string(" + val + ");";
      }
      else if (m_typecode->kind == DDS::tk_string)
      {
         ret = arg + " = DDS::string_dup (" + val + ");";
      }
      else
      {
         ret = arg + " = DDS::wstring_dup (" + val + ");";
      }
   }
   else
   {
      if (m_typecode->kind == DDS::tk_string)
      {
         ret = arg + " = " + val + ";";
      }
      else
      {
         ret = arg + " = " + val + ";";
      }
   }

   return ret;
}

DDS_StdString be_string::NullReturnArg ()
{
   DDS_StdString ret;

   if (m_typecode->kind == DDS::tk_string)
   {
      ret = "(char*)0";
   }
   else
   {
      ret = "(WChar*)0";
   }

   return ret;
}

be_string * be_string::_narrow (AST_Type * atype)
{
   be_string * ret = 0;

   if (atype)
   {
      ret = (be_string*)atype->narrow((long) & be_string::type_id);
   }

   return ret;
}

DDS_StdString
be_string::UnionStreamOut(const DDS_StdString& arg, const DDS_StdString& out) const
{
   return out + " << (" + UnionMemberTypeName() + ")" + arg + ";";
}

DDS_StdString be_string::UnionStreamIn
(
   const DDS_StdString & arg,
   const DDS_StdString & in
) const
{
   DDS_StdString tmpvar("_");
   tmpvar += arg;
   DDS_StdString lines;

   if (m_typecode->kind == DDS::tk_string)
   {
      lines = ("{DDS::String tmp; is.get (tmp);");
   }
   else
   {
      lines = ("{DDS::WString tmp; is.get (tmp);");
   }

   lines += " tmp.set_release (FALSE); " + arg + " = tmp; }";

   return lines;
}

DDS_StdString be_string::StructStreamOut
(
   const DDS_StdString & arg,
   const DDS_StdString & out
) const
{
   if (m_typecode->kind == DDS::tk_string)
   {
      return out + " << (DDS::String)" + arg + ";";
   }
   else
   {
      return out + " << (DDS::WString)" + arg + ";";
   }
}

DDS_StdString be_string::StructStreamIn
(
   const DDS_StdString & arg,
   const DDS_StdString & in
) const
{
   DDS_StdString tmpvar("_");
   tmpvar += arg;
   DDS_StdString lines;

   if (m_typecode->kind == DDS::tk_string)
   {
      lines = ("{DDS::String tmp; is.get (tmp);");
   }
   else
   {
      lines = ("{DDS::WString tmp; is.get (tmp);");
   }

   lines += " tmp.set_release((DDS::Boolean) FALSE); " + arg + " = tmp; }";

   return lines;
}

DDS_StdString be_string::SyncStreamOut
(
   const DDS_StdString & arg,
   const DDS_StdString & out,
   VarType
) const
{
   if (m_typecode->kind == DDS::tk_string)
   {
      return out + " << (DDS::String)" + arg + ";";
   }
   else
   {
      return out + " << (DDS::WString)" + arg + ";";
   }
}

DDS_StdString
be_string::SyncStreamIn(const DDS_StdString& arg, const DDS_StdString& in, VarType) const
{
   return in + " >> " + arg + ";";
}

DDS::Boolean
be_string::is_core_marshaled()
{
   return TRUE;
}

DDS::Boolean be_string::declare_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & stubScope,
   VarType vt
)
{
   DDS::Boolean ret = FALSE;

   switch (vt)
   {
      case VT_InParam:
      case VT_InOutParam:
      case VT_OutParam:
      break;

      case VT_Return:
      {
         if (m_typecode->kind == DDS::tk_string)
         {
            os << tab << "DDS::String " << arg << " = 0;" << nl;
         }
         else
         {
            os << tab << "DDS::WString " << arg << " = 0;" << nl;
         }

         ret = TRUE;
      }
      break;

      default:
      assert (0);
   }

   return ret;
}

DDS::Boolean be_string::declare_for_struct_put
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   return TRUE;
}

DDS::Boolean be_string::declare_for_struct_get
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << sptr << "->" << fld << " = (char *) 0;";

   return TRUE;
}

DDS::Boolean be_string::declare_for_union_put
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   return TRUE;
}

DDS::Boolean be_string::declare_for_union_get
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   return TRUE;
}

void be_string::generate_tc_ctor_val (be_Source & source)
{}

void be_string::generate_tc_dtor_val
(
   be_Source & source,
   pbbool isCounted
)
{}

void be_string::generate_tc_put_val (be_Source & source)
{}

void be_string::generate_tc_get_val (be_Source & source)
{}

void be_string::generate_tc_assign_val (be_Source & source)
{}

be_DispatchableType::en_HowStoredInDispatcher
   be_string::HowStoredInDispatcher (const be_ArgumentDirection & dir) const
{
   if (!m_wide && dir == VT_InParam)
   {
      return STORED_AS_STACK_VARIABLE;
   }
   return STORED_IN_STRING_VAR;
}

DDS::Boolean be_string::make_get_param_for_stub
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
      os << tab << "{ " << TypeCodeTypeName () << ", ";

      switch (vt)
      {
         case VT_Return:
         os << "&" << argname << ", DDS::PARAM_OUT ";
         break;

         case VT_InOutParam:
         os << "&" << argname << ", DDS::PARAM_INOUT ";
         break;

         case VT_OutParam:
         os << "&" << argname << ".m_ptr, DDS::PARAM_OUT ";
         break;

         default:
         assert (0);
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_string::make_put_param_for_stub
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
      os << tab << "{ " << TypeCodeTypeName () << ", ";

      switch (vt)
      {
         case VT_InParam:
         os << "&" << argname << ", DDS::PARAM_IN ";
         break;

         case VT_InOutParam:
         os << "&" << argname << ", DDS::PARAM_INOUT ";
         break;

         default:
         assert (0);
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_string::make_put_param_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   /* For structs attribute is implemented as a String_mgr */

   os << tab << "{ " << TypeCodeTypeName () << ", ";
   os << "&" << sptr << "->" << fld << ".m_ptr, DDS::PARAM_IN }";

   return TRUE;
}

DDS::Boolean be_string::make_get_param_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   /* For structs attribute is implemented as a String_mgr */

   os << tab << "{ " << TypeCodeTypeName () << ", ";
   os << "&" << sptr << "->" << fld << ".m_ptr, DDS::PARAM_OUT }";

   return TRUE;
}

DDS::Boolean be_string::make_put_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << TypeCodeTypeName () << ", ";
   os << "&" << sptr << "->" << fld << ", DDS::PARAM_IN }";

   return TRUE;
}

DDS::Boolean be_string::make_get_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << TypeCodeTypeName () << ", ";
   os << "&" << sptr << "->" << fld << ", DDS::PARAM_OUT }";

   return TRUE;
}


ostream & be_string::put_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   tab.indent ();
   os << tab << "os.cdr_put (" << sptr << "->" << fld
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent ();

   return os;
}

ostream & be_string::get_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   DDS_StdString mgrArg = fld;
   DDS_StdString lenArg = fld;

   mgrArg = mgrArg + "Mgr";
   mgrArg += uid;
   lenArg = lenArg + "Len";
   lenArg += uid;

   if (m_typecode->kind == DDS::tk_string)
   {
      os << tab << "DDS::String " << mgrArg
      << "(" << sptr << "->" << fld << ", (DDS::Boolean) FALSE);"
      << nl;
   }
   else
   {
      os << tab << "DDS::WString " << mgrArg
      << "(" << sptr << "->" << fld << ", (DDS::Boolean) FALSE);"
      << nl;
   }

   os << tab << " DDS::ULong " << lenArg << ";" << nl;
   tab.indent();
   os << tab << "is.cdr_get(" << mgrArg << ", "
      << lenArg << ", " << "(DDS::Boolean) FALSE"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent();

   return os;
}

ostream & be_string::put_for_union
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

   make_put_param_for_union (os, tab, sptr, fld, uid);

   // finish the declaration

   os << ";" << nl << nl;

   // call put

   os << tab << "os.put (&put" << fld << uid << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_string::get_for_union
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

   make_get_param_for_union (os, tab, sptr, fld, uid);

   // finish the declaration

   os << ";" << nl << nl;

   // call get

   os << tab << "is.get (&get" << fld << uid << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_string::put_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   DDS_StdString parg = (DDS_StdString)"_arg";
   parg += uid;

   os << tab << "DDS::Codec::Param " << parg << " = { " << Scope (TypeCodeTypeName ())
      << ", &" << arg << "[" << index << "], DDS::PARAM_IN };" << nl;
   os << tab << "os.put (&" << parg << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_string::get_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   DDS_StdString garg = (DDS_StdString)"_getArg_";
   garg += uid;

   os << tab << "DDS::Codec::Param " << garg << " = { " << Scope (TypeCodeTypeName ())
      << ", &" << arg << "[" << index << "], DDS::PARAM_OUT };" << nl;
   os << tab << "is.get (&" << garg << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_string::put_for_array
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   DDS_StdString parg = (DDS_StdString) "_putArg_";
   parg += uid;

   os << tab << "DDS::Codec::Param " << parg << " = { " << Scope (TypeCodeTypeName())
      << ", &(" << arg << "[" << index << "].m_ptr), mode };" << nl;
   os << tab << "os.put (&" << parg << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_string::get_for_array
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   DDS_StdString garg = (DDS_StdString) "_getArg_";
   garg += uid;

   if (m_typecode->kind == DDS::tk_string)
   {
      /*
         In Strings are shallow copied so disable String_mgr
         from releasing string when PARAM_IN.
      */

      os << tab << arg << "[" << index  << "]"  << " = (char*) 0;" << nl;

      os << tab << "DDS::Codec::Param " << garg
         << " = { " << Scope (TypeCodeTypeName ())
         << ", &" << arg << "[" << index  << "].m_ptr"
         << ", mode };" << nl;
      os << tab << "if (mode == DDS::PARAM_IN) "
         << arg << "[" << index  << "].m_rel = 0;" << nl;
   }
   else
   {
      os << tab << "DDS::Codec::Param " << garg << " = { "
         << Scope (TypeCodeTypeName ())
         << ", &(" << arg << "[" << index << "].m_ptr), mode };" << nl;
   }
   os << tab << "is.get (&" << garg << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

DDS_StdString be_string::kind_string ()
{
   if (m_typecode->kind == DDS::tk_string)
   {
      return "DDS::tk_string";
   }
   return "DDS::tk_wstring";
}

DDS::ULong be_string::get_elem_size ()
{
   return 0;
}

DDS::ULong be_string::get_elem_alignment ()
{
   return 4;
}

pbbool be_string::IsWide () const
{
   return (m_typecode->kind != DDS::tk_string);
}
