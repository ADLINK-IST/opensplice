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
#include "xbe_predefined.h"
#include "xbe_utils.h"
#include "xbe_typedef.h"

#include "cppgen_iostream.h"

// -------------------------------------------------
//  BE_PREDEFINED_TYPE IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS3(be_predefined_type, AST_PredefinedType,
                     be_DispatchableType, be_Type)
IMPL_NARROW_FROM_DECL(be_predefined_type)

be_predefined_type::be_predefined_type
(
   AST_PredefinedType::PredefinedType t,
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
:
   AST_Decl (AST_Decl::NT_pre_defined, n, p),
   AST_PredefinedType (t, n, p)
{
   enclosingScope = "DDS";
   m_isReturnedByVar = pbfalse;

   switch (t)
   {
      case PT_long:
      {
         localName = "Long";
         m_typecode->kind = DDS::tk_long;
         m_typecode->id = "DDS::Long";
         m_elemSize = 4;
         m_elemAlign = 4;
         m_swapCall = "SWAP32";
         break;
      }
      case PT_ulong:
      {
         localName = "ULong";
         m_typecode->kind = DDS::tk_ulong;
         m_typecode->id = "DDS::ULong";
         m_elemSize = 4;
         m_elemAlign = 4;
         m_swapCall = "SWAP32";
         break;
      }
      case PT_longlong:
      {
         localName = "LongLong";
         m_typecode->kind = DDS::tk_longlong;
         m_typecode->id = "DDS::LongLong";
         m_elemSize = 8;
         m_elemAlign = 8;
         m_swapCall = "SWAP64";
         break;
      }
      case PT_ulonglong:
      {
         localName = "ULongLong";
         m_typecode->kind = DDS::tk_ulonglong;
         m_typecode->id = "DDS::ULongLong";
         m_elemSize = 8;
         m_elemAlign = 8;
         m_swapCall = "SWAP64";
         break;
      }
      case PT_short:
      {
         localName = "Short";
         m_typecode->kind = DDS::tk_short;
         m_typecode->id = "DDS::Short";
         m_elemSize = 2;
         m_elemAlign = 2;
         m_swapCall = "SWAP16";
         break;
      }
      case PT_ushort:
      {
         localName = "UShort";
         m_typecode->kind = DDS::tk_ushort;
         m_typecode->id = "DDS::UShort";
         m_elemSize = 2;
         m_elemAlign = 2;
         m_swapCall = "SWAP16";
         break;
      }
      case PT_float:
      {
         localName = "Float";
         m_typecode->kind = DDS::tk_float;
         m_typecode->id = "DDS::Float";
         m_elemSize = 4;
         m_elemAlign = 4;
         m_swapCall = "SWAP32";
         break;
      }
      case PT_double:
      {
         localName = "Double";
         m_typecode->kind = DDS::tk_double;
         m_typecode->id = "DDS::Double";
         m_elemSize = 8;
         m_elemAlign = 8;
         m_swapCall = "SWAP64";
         break;
      }
      case PT_longdouble:
      {
         localName = "LongDouble";
         m_typecode->kind = DDS::tk_longdouble;
         m_typecode->id = "DDS::LongDouble";
         m_elemSize = 0;
         m_elemAlign = 8;
         m_swapCall = "SWAP128";
         break;
      }
      case PT_char:
      {
         localName = "Char";
         m_typecode->kind = DDS::tk_char;
         m_typecode->id = "DDS::Char";
         m_elemSize = 1;
         m_elemAlign = 1;
         m_swapCall = "";
         break;
      }
      case PT_wchar:
      {
         if (BE_Globals::map_wide)
         {
            localName = "Char";
            m_typecode->kind = DDS::tk_char;
            m_typecode->id = "DDS::Char";
            m_elemSize = 1;
            m_elemAlign = 1;
            m_swapCall = "";
         }
         else
         {
            /* wchar type not of fixed size */

            localName = "WChar";
            m_typecode->kind = DDS::tk_wchar;
            m_typecode->id = "DDS::WChar";
            m_elemSize = 0;
            m_elemAlign = 0;
            m_swapCall = "";
         }
         break;
      }
      case PT_boolean:
      {
         /* bool type not of fixed size */

         localName = "Boolean";
         m_typecode->kind = DDS::tk_boolean;
         m_typecode->id = "DDS::Boolean";
         m_elemSize = 0;
         m_elemAlign = 0;
         m_swapCall = "";
         break;
      }
      case PT_octet:
      {
         localName = "Octet";
         m_typecode->kind = DDS::tk_octet;
         m_typecode->id = "DDS::Octet";
         m_elemSize = 1;
         m_elemAlign = 1;
         m_swapCall = "";
         break;
      }
      case PT_any:
      {
         localName = "Any";
         m_typecode->kind = DDS::tk_any;
         m_typecode->id = "DDS::Any";
         m_elemSize = 0;
         m_elemAlign = 0;
         m_swapCall = "";
         m_isReturnedByVar = pbtrue;
         break;
      }
      case PT_void:
      {
         localName = "void";
         enclosingScope = "";
         m_typecode->kind = DDS::tk_void;
         m_typecode->id = "void";
         m_elemSize = 0;
         m_elemAlign = 0;
         m_swapCall = "";
         break;
      }
      case PT_object:
      {
         localName = "Object";
         m_typecode->kind = DDS::tk_objref;
         m_typecode->id = "IDL:omg.org/DDS/Object:1.0";
         m_elemSize = 0;
         m_elemAlign = 0;
         m_swapCall = "";
         m_isReturnedByVar = pbtrue;
         break;
      };
      case PT_local_object:
      {
         localName = "LocalObject";
         m_typecode->kind = DDS::tk_local_interface;
         m_typecode->id = "IDL:omg.org/DDS/LocalObject:1.0";
         m_elemSize = 0;
         m_elemAlign = 0;
         m_swapCall = "";
         m_isReturnedByVar = pbtrue;
         break;
      };
      case PT_pseudo:
      {
         //
         // this should never occur
         // YO don't know what to do here...
         //
         localName = local_name()->get_string();
         m_typecode->kind = (DDS::TCKind)0;
         m_typecode->id = "DDS_PseudoObject";
         m_elemSize = 0;
         m_elemAlign = 0;
         m_swapCall = "";
         break;
      }
      case PT_typecode:
      {
         localName = local_name()->get_string();
         m_typecode->kind = DDS::tk_TypeCode;
         m_typecode->id = "DDS::TypeCode";
         m_elemSize = 0;
         m_elemAlign = 0;
         m_swapCall = "";
         break;
      }
      default:
      {
         localName = local_name()->get_string();
         break;
      }
   }

   m_any_op_id = m_typecode->id;

   InitializeTypeMap (this);
}

void be_predefined_type::InitializeTypeMap (be_Type * t)
{
   idlType = t;

   if (t)
   {
      DDS_StdString inName;
      DDS_StdString inoutName;
      DDS_StdString outName;
      DDS_StdString retName;
      PredefinedType pt = PT_void;
      be_typedef * t_typedef = 0;
      be_predefined_type * t_predef = 0;

      if ((t_typedef = (be_typedef*)t->narrow((long) & be_typedef::type_id)))
      {
         AST_Type * t_ast = (AST_Type*)t->narrow((long) & AST_Type::type_id);
         AST_Type * basetype = t_typedef->base_type();

         assert(t_ast);
         assert(basetype);

         while ((t_typedef = (be_typedef*)basetype->narrow((long) & be_typedef::type_id)))
         {
            basetype = t_typedef->base_type();
            assert(basetype);
         }

         t_predef = be_predefined_type::_narrow(basetype);
         assert(t_predef);

         pt = t_predef->pt();
         /**
          * @internal
          * @bug Hacked in a fix
          */
         if (CorbaTypesMap::TypeName(pt))
         {
            t->TypeName(CorbaTypesMap::TypeName(pt));
         }
         else
         {
            t->TypeName(NameToString(t_ast->name()));
         }
         t->TypeCodeTypeName(BE_Globals::TCPrefix + t->LocalName());
         t->MetaTypeTypeName(BE_Globals::MTPrefix + NameToString(t_ast->name(), "_"));
      }
      else if ((t_predef = (be_predefined_type*)t->narrow((long) & be_predefined_type::type_id)))

      {
         pt = t_predef->pt();
         t->TypeName(CorbaTypesMap::TypeName(pt));
         t->TypeCodeTypeName(CorbaTypesMap::TypeCodeName(pt));
      }


      if (pt == AST_PredefinedType::PT_object ||
          pt == AST_PredefinedType::PT_local_object)
      {
         t_predef->m_isInterfaceType = TRUE;
      }
      else
      {
         t_predef->m_isInterfaceType = FALSE;
      }

      if (pt < AST_PredefinedType::PT_any)
      {
         t->InTypeName(t->TypeName());
         t->InOutTypeName(t->TypeName() + "&");
         t->OutTypeName(t->TypeName() + "_out");
         t->ReturnTypeName(t->TypeName());
         t->DMFAdtMemberTypeName(t->TypeName());
         t->StructMemberTypeName(t->TypeName());
         t->UnionMemberTypeName(t->TypeName());
         t->SequenceMemberTypeName(t->TypeName());
         t->StreamOpTypeName(t->TypeName());
         t->IStreamOpTypeName(t->TypeName() + "&");

         t->VarSignature( VT_InParam,
                          t->TypeName(),
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
         t->VarSignature( VT_InOutParam,
                          t->TypeName(),
                          VT_NonConst,
                          VT_Var,
                          VT_Reference);
         t->VarSignature( VT_OutParam,
                          t->TypeName(),
                          VT_NonConst,
                          VT_Var,
                          VT_Reference);
         t->VarSignature( VT_Return,
                          t->TypeName(),
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
      }
      else if (pt == AST_PredefinedType::PT_void)
      {
         t->InTypeName(t->TypeName());
         t->InOutTypeName(t->TypeName());
         t->OutTypeName(t->TypeName());
         t->ReturnTypeName(t->TypeName());
         t->DMFAdtMemberTypeName(t->TypeName());
         t->StructMemberTypeName(t->TypeName());
         t->UnionMemberTypeName(t->TypeName());
         t->SequenceMemberTypeName(t->TypeName());
         t->StreamOpTypeName(t->TypeName());
         t->IStreamOpTypeName(t->TypeName() + "&");

         t->VarSignature( VT_InParam,
                          t->TypeName(),
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
         t->VarSignature(VT_InOutParam,
                         t->TypeName(),
                         VT_NonConst,
                         VT_Var,
                         VT_NonReference);
         t->VarSignature(VT_OutParam,
                         t->TypeName(),
                         VT_NonConst,
                         VT_Var,
                         VT_NonReference);
         t->VarSignature(VT_Return,
                         t->TypeName(),
                         VT_NonConst,
                         VT_Var,
                         VT_NonReference);
      }
      else if (pt == AST_PredefinedType::PT_any)
      {
         outName = t->TypeName() + DDSOutExtension;

         t->InTypeName((DDS_StdString)"const " + t->TypeName() + "&");
         t->InOutTypeName(t->TypeName() + "&");
         t->OutTypeName(outName);
         t->ReturnTypeName(t->TypeName() + DDSPtrExtension);
         t->DMFAdtMemberTypeName(t->TypeName() + DDSPtrExtension);
         t->StructMemberTypeName(t->TypeName());
         t->UnionMemberTypeName(t->TypeName());
         t->SequenceMemberTypeName(t->TypeName());
         t->StreamOpTypeName(t->TypeName());
         t->IStreamOpTypeName(t->TypeName() + "&");

         t->VarSignature( VT_InParam,
                          t->TypeName(),
                          VT_Const,
                          VT_Var,
                          VT_Reference);
         t->VarSignature( VT_InOutParam,
                          t->TypeName(),
                          VT_NonConst,
                          VT_Var,
                          VT_Reference);
         t->VarSignature( VT_OutParam,
                          t->OutTypeName(),
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
         t->VarSignature( VT_Return,
                          t->TypeName(),
                          VT_NonConst,
                          VT_Pointer,
                          VT_NonReference);
      }
      else if (pt == AST_PredefinedType::PT_object ||
               pt == AST_PredefinedType::PT_local_object)
      {
         DDS_StdString ptrName = t->TypeName() + DDSPtrExtension;

         inName = ptrName;
         inoutName = t->TypeName() + "_ptr&";
         outName = t->TypeName() + DDSOutExtension;
         retName = ptrName;

         t->InTypeName(inName);
         t->InOutTypeName(inoutName);
         t->OutTypeName(outName);
         t->ReturnTypeName(retName);
         t->DMFAdtMemberTypeName(ptrName);
         t->StructMemberTypeName(t->TypeName() + DDSVarExtension);
         t->UnionMemberTypeName(t->TypeName() + DDSPtrExtension);
         t->SequenceMemberTypeName(ptrName);
         t->StreamOpTypeName(ptrName);
         t->IStreamOpTypeName(ptrName + "&");

         t->VarSignature( VT_InParam,
                          inName,
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
         t->VarSignature( VT_InOutParam,
                          inoutName,
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
         t->VarSignature( VT_OutParam,
                          outName,
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
         t->VarSignature( VT_Return,
                          retName,
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
      }
      else // PT_typecode or PT_pseudo
      {
         assert(pt == AST_PredefinedType::PT_typecode
                || pt == AST_PredefinedType::PT_pseudo);
         DDS_StdString ptrName = t->TypeName() + DDSPtrExtension;

         inName = ptrName;
         inoutName = t->TypeName() + "_ptr&";
         outName = t->TypeName() + DDSOutExtension;
         retName = ptrName;

         t->InTypeName(inName);
         t->InOutTypeName(inoutName);
         t->OutTypeName(outName);
         t->ReturnTypeName(retName);
         t->DMFAdtMemberTypeName(ptrName);
         t->StructMemberTypeName(t->TypeName() + DDSVarExtension);
         t->UnionMemberTypeName(t->TypeName() + DDSPtrExtension);
         t->SequenceMemberTypeName(ptrName);
         t->StreamOpTypeName(ptrName);
         t->IStreamOpTypeName(ptrName + "&");

         t->VarSignature( VT_InParam,
                          inName,
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
         t->VarSignature( VT_InOutParam,
                          inoutName,
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
         t->VarSignature( VT_OutParam,
                          outName,
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
         t->VarSignature( VT_Return,
                          retName,
                          VT_NonConst,
                          VT_Var,
                          VT_NonReference);
      }
   }
   else
   {
      assert(0);
   }
}

DDS::Boolean
be_predefined_type::IsFixedLength() const
{
   be_predefined_type * my = (be_predefined_type*)this;
   DDS::Boolean ret = TRUE;

   if ( (my->pt() == AST_PredefinedType::PT_any) ||
        (my->pt() == AST_PredefinedType::PT_object) ||
        (my->pt() == AST_PredefinedType::PT_typecode) ||
        (my->pt() == AST_PredefinedType::PT_local_object) )
   {
      ret = FALSE;
   }

   return ret;
}

/* This function is added to counter the problem found with Union when changing the
   switches in IsFixedLength to include wchar boolean and long double.
   Problem not known but prevents code compilation!
*/

DDS::Boolean
be_predefined_type::IsFixedLengthPrimitiveType() const
{
   be_predefined_type * my = (be_predefined_type*)this;
   DDS::Boolean ret = TRUE;

   if ( (my->pt() == AST_PredefinedType::PT_boolean) ||
         (my->pt() == AST_PredefinedType::PT_longdouble) ||
         (my->pt() == AST_PredefinedType::PT_wchar))
   {
      ret = FALSE;
   }

   return ret;
}

DDS::Boolean
be_predefined_type::IsStructuredType() const
{
   be_predefined_type * my = (be_predefined_type*)this;
   DDS::Boolean ret = FALSE;

   if (my->pt() == AST_PredefinedType::PT_any)
   {
      ret = TRUE;
   }

   return ret;
}

void
be_predefined_type::GenerateTypedefs(
   const DDS_StdString &scope,
   const be_typedef& alias,
   be_ClientHeader& source)
{
   be_predefined_type * my = (be_predefined_type*)this;
   ostream & os = source.Stream();
   be_Tab tab(source);

   switch (my->pt())
   {
      case AST_PredefinedType::PT_object:
      {
         os << tab << "typedef " << typeName << DDSStubExtension
            << " " << alias.LocalName() << DDSStubExtension << ";" << nl;
         // fall through
      }

      case AST_PredefinedType::PT_local_object:

      case AST_PredefinedType::PT_typecode:
      {
         os << tab << "typedef " << typeName
            << " " << alias.LocalName() << ";" << nl;
         os << tab << "typedef " << typeName << DDSPtrExtension
            << " " << alias.LocalName() << DDSPtrExtension << ";" << nl;
         os << tab << "typedef " << typeName << DDSVarExtension
            << " " << alias.LocalName() << DDSVarExtension << ";" << nl;
         os << tab << "typedef " << typeName << DDSOutExtension
            << " " << alias.LocalName() << DDSOutExtension << ";" << nl;
         os << tab << "typedef " << typeName << DDSMgrExtension
            << " " << alias.LocalName() << DDSMgrExtension << ";" << nl;
      }

      break;

      case AST_PredefinedType::PT_any:
      {
         os << tab << "typedef " << typeName
            << " " << alias.LocalName() << ";" << nl;
         os << tab << "typedef " << typeName << DDSVarExtension
            << " " << alias.LocalName() << DDSVarExtension << ";" << nl;
         os << tab << "typedef " << typeName << DDSOutExtension
            << " " << alias.LocalName() << DDSOutExtension << ";" << nl;
      }

      break;

      default:
      {
         os << tab << "typedef " << typeName
            << " " << alias.LocalName() << ";" << nl;
      }

      break;
   }
}

DDS_StdString be_predefined_type::Allocater (const DDS_StdString & arg) const
{
   return (DDS_StdString)typeName + " " + arg + ";";
}

DDS_StdString be_predefined_type::Initializer
(
   const DDS_StdString& arg,
   VarType vt
) const
{
   DDS_StdString ret;

   AST_PredefinedType::PredefinedType pdt = ((be_predefined_type*)this)->pt();

   if
   (
      (pdt == AST_PredefinedType::PT_pseudo) ||
      (pdt == AST_PredefinedType::PT_object) ||
      (pdt == AST_PredefinedType::PT_typecode) ||
      (pdt == AST_PredefinedType::PT_local_object) ||
      (
         (pdt == AST_PredefinedType::PT_any) &&
         (vt != VT_InOutParam) &&
         (vt != VT_Attribute)
      )
   )
   {
      ret = arg + " = 0;";
   }
   else
   {
      ret = arg + ";";
   }

   return ret;
}

DDS_StdString
be_predefined_type::InRequestArgumentDeclaration(
   be_Type& btype,
   const DDS_StdString& arg,
   VarType vt)
{
   DDS_StdString ret = btype.TypeName();

   AST_PredefinedType::PredefinedType pdt = ((be_predefined_type*)this)->pt();

   if
   (
      (pdt == AST_PredefinedType::PT_object) ||
      (pdt == AST_PredefinedType::PT_typecode) ||
      (pdt == AST_PredefinedType::PT_local_object)
   )
   {
      ret += "_var " + arg + ";";
   }
   else if (!IsFixedLength() && (vt == VT_Return || vt == VT_OutParam))
   {
      if (pdt == AST_PredefinedType::PT_any)
      {
         ret += "_var " + arg + ";";
      }
      else
      {
         ret += "_mgr " + arg + "(TRUE);";
      }
   }
   else
   {
      ret += " " + arg + ";";
   }

   return ret;
}

DDS_StdString be_predefined_type::Releaser (const DDS_StdString & arg) const
{
   AST_PredefinedType::PredefinedType pdt = ((be_predefined_type*)this)->pt();
   DDS_StdString ret;  // primitives, voids any don't get heap allocated

   if
   (
      pdt == AST_PredefinedType::PT_pseudo ||
      pdt == AST_PredefinedType::PT_typecode ||
      pdt == AST_PredefinedType::PT_object ||
      pdt == AST_PredefinedType::PT_local_object
   )
   {
      ret = (DDS_StdString)"DDS::release (" + arg + ");";
   }
   if (pdt == AST_PredefinedType::PT_any)
   {
      ret = (DDS_StdString)"delete " + arg + ";";
   }

   return ret;
}

DDS_StdString
be_predefined_type::Assigner(const DDS_StdString&, const DDS_StdString&) const
{
   // YO BEN probably need code here for Object, and maybe also some
   // other predefined types
   assert(pbfalse);
   return (DDS_StdString)"";
}

DDS_StdString be_predefined_type::Duplicater
(
   const DDS_StdString & arg,
   const DDS_StdString & val,
   const DDS_StdString &,
   const pbbool
) const
{
   AST_PredefinedType::PredefinedType pdt = ((be_predefined_type*)this)->pt();

   if (pdt == AST_PredefinedType::PT_object)
   {
      DDS_StdString ret = arg + " = DDS::Object::_duplicate(" + val + ");";
      return ret;
   }
   else if (pdt == AST_PredefinedType::PT_local_object)
   {
      DDS_StdString ret = arg + " = DDS::Object::_duplicate(" + val + ");";
      return ret;
   }
   else if (pdt == AST_PredefinedType::PT_typecode)
   {
      DDS_StdString ret = arg + " = DDS::TypeCode::_duplicate (" + val + ");";
      return ret;
   }
   else if (pdt == AST_PredefinedType::PT_longdouble)
   {
      DDS_StdString ret = arg + " = DDS::LongDouble::_duplicate (" + val + ");";
      return ret;
   }
   else
   {
      // YO BEN do we need code for other predefined types?
      assert(pbfalse);
      return (DDS_StdString)"";
   }
}

DDS_StdString
be_predefined_type::NullReturnArg()
{
   DDS_StdString ret = (DDS_StdString)"(" + returnTypeName + ")0";

   return ret;
}

be_predefined_type *
be_predefined_type::_narrow(AST_Type * atype)
{
   be_predefined_type * ret = 0;

   if (atype)
   {
      ret = (be_predefined_type*)atype->narrow((long) & be_predefined_type::type_id);
   }

   return ret;
}

void
be_predefined_type::GenerateTypeCodeInit(be_ClientHeader &source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << tab << "// DDS::TypeCode_ptr "
   << (const char*)Scope(TypeCodeTypeName()) << ";" << nl;

   if ( (pt() == AST_PredefinedType::PT_object) ||
        (pt() == AST_PredefinedType::PT_typecode) ||
        (pt() == AST_PredefinedType::PT_local_object) )
   {
      os << "DDS_METATYPE_OBJECT(" << (const char*)TypedefName() << ")" << nl;
   }
   else
   {
      os << "DDS_METATYPE_SIMPLE(" << (const char*)TypedefName() << ")" << nl;
   }
}

DDS_StdString be_predefined_type::kind_string ()
{
   DDS_StdString ret ("");

   switch (m_typecode->kind)
   {
      case DDS::tk_null:
      ret = "DDS::tk_null";
      break;

      case DDS::tk_void:
      ret = "DDS::tk_void";
      break;

      case DDS::tk_short:
      ret = "DDS::tk_short";
      break;

      case DDS::tk_long:
      ret = "DDS::tk_long";
      break;

      case DDS::tk_ushort:
      ret = "DDS::tk_ushort";
      break;

      case DDS::tk_ulong:
      ret = "DDS::tk_ulong";
      break;

      case DDS::tk_float:
      ret = "DDS::tk_float";
      break;

      case DDS::tk_double:
      ret = "DDS::tk_double";
      break;

      case DDS::tk_boolean:
      ret = "DDS::tk_boolean";
      break;

      case DDS::tk_char:
      ret = "DDS::tk_char";
      break;

      case DDS::tk_octet:
      ret = "DDS::tk_octet";
      break;

      case DDS::tk_any:
      ret = "DDS::tk_any";
      break;

      case DDS::tk_TypeCode:
      ret = "DDS::tk_TypeCode";
      break;

      case DDS::tk_Principal:
      ret = "DDS::tk_Principal";
      break;

      case DDS::tk_objref:
      ret = "DDS::tk_objref";
      break;

      case DDS::tk_local_interface:
      ret = "DDS::tk_local_interface";
      break;

      case DDS::tk_string:
      ret = "DDS::tk_string";
      break;

      case DDS::tk_sequence:
      ret = "DDS::tk_sequence";
      break;

      case DDS::tk_array:
      ret = "DDS::tk_array";
      break;

      case DDS::tk_alias:
      ret = "DDS::tk_alias";
      break;

      case DDS::tk_except:
      ret = "DDS::tk_except";
      break;

      case DDS::tk_longlong:
      ret = "DDS::tk_longlong";
      break;

      case DDS::tk_ulonglong:
      ret = "DDS::tk_ulonglong";
      break;

      case DDS::tk_longdouble:
      ret = "DDS::tk_longdouble";
      break;

      case DDS::tk_wchar:
      ret = "DDS::tk_wchar";
      break;

      case DDS::tk_wstring:
      ret = "DDS::tk_wstring";
      break;

      case DDS::tk_fixed:
      ret = "DDS::tk_fixed";
      break;

      default: assert (0);
   }

   return ret;
}

DDS::ULong
be_predefined_type::get_elem_size()
{
   return m_elemSize;
}

DDS::ULong
be_predefined_type::get_elem_alignment()
{
   return m_elemAlign;
}

DDS::ULong
be_predefined_type::get_OS_elem_alignment()
{
   DDS::ULong alignment = 0;
   switch (pt())
   {
      case PT_double:
      case PT_longlong:
      {
         alignment = 4;
      }
      break;

      default:
      {
         alignment = m_elemAlign;
      }
      break;
   }
   return alignment;
}

DDS::Boolean be_predefined_type::declare_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & stubScope,
   VarType vt
)
{
   DDS::Boolean ret = FALSE;

   switch (pt ())
   {
      case AST_PredefinedType::PT_local_object:
      {
         break;
      }

      case AST_PredefinedType::PT_object:
      {
         if (vt == VT_OutParam)
         {
            os << tab << arg << " = new DDS::Object ();" << nl;
            ret = TRUE;
         }
         else if (vt == VT_Return)
         {
            os << tab << ScopedName () << "_var _ret_ = "
               << "new DDS::Object ();" << nl;
            ret = TRUE;
         }
         break;
      }

      case AST_PredefinedType::PT_typecode:
      {
         if (vt == VT_Return)
         {
            os << tab << TypeName() << "_ptr " << arg << ";" << nl;
            ret = TRUE;
         }
         break;
      }

      case AST_PredefinedType::PT_any:
      {
         ret = TRUE;

         switch (vt)
         {
            case VT_InParam:
            case VT_InOutParam:
            break;

            case VT_OutParam:
            {
               os << tab << arg << " = new DDS::Any;" << nl;
               break;
            }

            case VT_Return:
            {
               os << tab << "DDS_var <DDS::Any> _ret_ (new DDS::Any);"
                  << nl;
               break;
            }

            default: break;
         }
         break;
      }

      default:
      {
         if (vt == VT_Return)
         {
            os << tab << TypeName() << " " << arg << ";" << nl;
         }
         break;
      }
   }

   return ret;
}

DDS::Boolean
be_predefined_type::declare_for_struct_put(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   switch (pt())
   {

         case AST_PredefinedType::PT_local_object:
         case AST_PredefinedType::PT_object:
      {}

         break;

         case AST_PredefinedType::PT_any:
      {}

         break;

         default:
         // do nothing...and like it!
         break;
   }

   return FALSE;
}

DDS::Boolean
be_predefined_type::declare_for_struct_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   DDS::Boolean ret = FALSE;

   switch (pt())
   {

         case AST_PredefinedType::PT_object:
         {
            os << tab << "DDS::Object::_assert_stub((DDS::Object*&)("
            << sptr << "->" << fld << "));"
            << nl;

            os << tab << "IOP::IOR::release (((DDS::Object*)(" << sptr << "->"
            << fld << "))->get_ior());" << nl;

            ret = TRUE;
         }
         break;

         case AST_PredefinedType::PT_typecode:
         {
            os << tab << "DDS::release ((DDS::TypeCode*)("
               << sptr << "->" << fld << "));" << nl;

            ret = TRUE;
         }
         break;

         case AST_PredefinedType::PT_any:
         {}

         break;

         default:
         // do nothing...and like it!
         break;
   }

   return ret;
}

DDS::Boolean
be_predefined_type::declare_for_union_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   DDS::Boolean ret = FALSE;

   switch (pt())
   {

         case AST_PredefinedType::PT_object:
         {
            os << tab << "DDS::Object::_assert_stub((DDS::Object*&)("
            << sptr << "->" << fld << "));"
            << nl;

            os << tab << "IOP::IOR::release (((DDS::Object*)(" << sptr << "->"
            << fld << "))->get_ior());" << nl;

            ret = TRUE;
         }

         break;

         case AST_PredefinedType::PT_typecode:
         {
            os << tab << "DDS::release ((DDS::TypeCode*)("
               << sptr << "->" << fld << "));" << nl;

            ret = TRUE;
         }
         break;

         case AST_PredefinedType::PT_any:
         {
            os << tab << sptr << "->" << fld << " = new DDS::Any;" << nl;
         }

         break;

         default:
         // do nothing...and like it!
         break;
   }

   return ret;
}

DDS::Boolean be_predefined_type::make_get_param_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   VarType vt
) const
{
   DDS::Boolean ret = FALSE;
   DDS_StdString dir = "DDS::PARAM_OUT";

   if (vt == VT_InOutParam)
   {
      dir = "DDS::PARAM_INOUT";
   }

   switch (pt ())
   {
      case AST_PredefinedType::PT_object:
      {
         if ( vt == VT_InOutParam || vt == VT_OutParam || vt == VT_Return)
         {
            os << tab;
            os << "{ " << Scope (TypeCodeTypeName()) << ", &";
            os << arg << "->get_ior (), " << dir << " ";
            os << "}";
            ret = TRUE;
         }
      }
      break;

      case AST_PredefinedType::PT_any:
      {
         if ( vt == VT_InOutParam || vt == VT_OutParam || vt == VT_Return)
         {
            os << tab << "{ " << Scope (TypeCodeTypeName ()) << ", ";

            switch (vt)
            {
               case VT_InOutParam:
               os << "&" << arg << ", " << dir << " ";
               break;

               case VT_OutParam:
               os << arg << ".out(), " << dir << " ";
               break;

               case VT_Return:
               os << "_ret_, " << dir << " ";
               break;

               default: assert (0);
            }

            os << "}";
            ret = TRUE;
         }
      }
      break;

      case AST_PredefinedType::PT_typecode:
      {
         if (vt == VT_InOutParam || vt == VT_Return)
         {
            os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";
            os << "&" << arg << ", " << dir << " ";
            os << "}";
            ret = TRUE;
         }
         else if (vt == VT_OutParam)
         {
            os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";
            os << "&" << arg << ".m_ptr, " << dir << " ";
            os << "}";
            ret = TRUE;
         }
      }
      break;

      default:
      {
         if (vt == VT_InOutParam || vt == VT_OutParam || vt == VT_Return)
         {
            os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";
            os << "&" << arg << ", " << dir << " ";
            os << "}";
            ret = TRUE;
         }
      }
      break;
   }

   return ret;
}

DDS::Boolean be_predefined_type::make_put_param_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   VarType vt
) const
{
   DDS::Boolean ret = FALSE;

   switch (pt())
   {

         case AST_PredefinedType::PT_object:
         {
            if (vt == VT_InParam || vt == VT_InOutParam)
            {
               os << tab;
               os << "{ " << Scope(TypeCodeTypeName()) << ", ";
               os << "(" << arg << ") ? " << arg << "->get_ior() : 0, ";
               os << "DDS::PARAM_IN ";
               os << "}";
               ret = TRUE;
            }
         }

         break;

         case AST_PredefinedType::PT_any:
         {
            if (vt == VT_InParam || vt == VT_InOutParam)
            {
               os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";

               switch (vt)
               {
                  case VT_InParam:
                     os << "(DDS::Any*)&" << arg << ", DDS::PARAM_IN ";
                     break;

                  case VT_InOutParam:
                     os << "&" << arg << ", DDS::PARAM_IN ";
                     break;

                  default: assert (0);
               }

               os << "}";
               ret = TRUE;
            }
         }

         break;

         case AST_PredefinedType::PT_typecode:
         {
            if (vt == VT_InParam || vt == VT_InOutParam)
            {
               os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";

               switch (vt)
               {
                  case VT_InParam:
                     os << arg << ", DDS::PARAM_IN ";
                     break;

                  case VT_InOutParam:
                     os << arg << ", DDS::PARAM_INOUT ";
                     break;

                  default: assert (0);
               }

               os << "}";
               ret = TRUE;
            }
         }

         break;

         default:
         {
            if (vt == VT_InParam || vt == VT_InOutParam)
            {
               os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";

               switch (vt)
               {
                  case VT_InParam:
                     os << "&" << arg << ", DDS::PARAM_IN ";
                     break;

                  case VT_InOutParam:
                     os << "&" << arg << ", DDS::PARAM_INOUT ";
                     break;

                  default: assert (0);
               }

               os << "}";
               ret = TRUE;
            }
         }
   }

   return ret;
}

DDS::Boolean be_predefined_type::make_put_param_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << Scope (TypeCodeTypeName ()) << ", ";

   switch (pt())
   {
      case AST_PredefinedType::PT_object:
      {
         os << "((DDS::Object*)(" << sptr << "->" << fld
            << "))->get_ior(), DDS::PARAM_IN ";
      }
      break;

      case AST_PredefinedType::PT_any:
      {
         os << "(void*)&" << sptr << "->" << fld << ", DDS::PARAM_IN ";
      }
      break;

      case AST_PredefinedType::PT_typecode:
      {
         os << "(void *) " << sptr << "->" << fld << ", DDS::PARAM_IN";
      }
      break;

      default:
      {
         os << "(void*)&" << sptr << "->" << fld << ", DDS::PARAM_IN ";
      }
   }

   os << "}";

   return TRUE;
}

DDS::Boolean be_predefined_type::make_get_param_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << Scope (TypeCodeTypeName ()) << ", ";

   switch (pt ())
   {
      case AST_PredefinedType::PT_object:
      os << "&((DDS::Object*)(" << sptr << "->" << fld
      << "))->get_ior(), DDS::PARAM_OUT ";
      break;

      case AST_PredefinedType::PT_any:
      os << "&" << sptr << "->" << fld << ", DDS::PARAM_OUT ";
      break;

      default:
      os << "&" << sptr << "->" << fld << ", DDS::PARAM_OUT ";
      break;
   }

   os << "}";

   return TRUE;
}

DDS::Boolean be_predefined_type::make_put_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << Scope (TypeCodeTypeName ()) << ", ";

   switch (pt())
   {
      case AST_PredefinedType::PT_object:
      {
         os << "((DDS::Object*)(" << sptr << "->" << fld
            << "))->get_ior(), DDS::PARAM_IN ";
      }
      break;

      case AST_PredefinedType::PT_any:
      {
         os << sptr << "->" << fld << ", DDS::PARAM_IN ";
      }
      break;

      case AST_PredefinedType::PT_typecode:
      {
         os << "(void *) " << sptr << "->" << fld << ", DDS::PARAM_IN";
      }
      break;

      default:
      {
         os << "(void*)&" << sptr << "->" << fld << ", DDS::PARAM_IN ";
      }
   }

   os << "}";

   return TRUE;
}

DDS::Boolean be_predefined_type::make_get_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << Scope (TypeCodeTypeName ()) << ", ";

   switch (pt ())
   {
      case AST_PredefinedType::PT_object:
      os << "&((DDS::Object*)(" << sptr << "->" << fld
      << "))->get_ior(), DDS::PARAM_OUT ";
      break;

      case AST_PredefinedType::PT_any:
      os << sptr << "->" << fld << ", DDS::PARAM_OUT ";
      break;

      default:
      os << "&" << sptr << "->" << fld << ", DDS::PARAM_OUT ";
      break;
   }

   os << "}";

   return TRUE;
}

ostream & be_predefined_type::put_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   tab.indent ();

   switch (pt ())
   {

      case AST_PredefinedType::PT_object:
      {
         os << tab << "os.cdr_put(" << sptr << "->" << fld
            << "->get_ior ()" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }

      break;

      case AST_PredefinedType::PT_any:
      {
         os << tab << sptr << "->" << fld << ".put (os"
            << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }

      break;

      default:
      {
         os << tab << "os.cdr_put(" << sptr
            << "->" << fld << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }
   }

   tab.outdent ();

   return os;
}

ostream & be_predefined_type::get_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   tab.indent();

   switch (pt())
   {

      case AST_PredefinedType::PT_object:
      {
         os << tab << "DDS::Object::_assert_stub("
            << sptr << "->" << fld << ".m_ptr);" << nl;
         os << tab << "is.cdr_get(" << sptr << "->" << fld
            << "->get_ior ()" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }

      break;

      case AST_PredefinedType::PT_any:
      {
         os << tab << sptr << "->" << fld << ".get (is"
            << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }

      break;


      default:
      {
         os << tab << "is.cdr_get (" << sptr << "->"
            << fld << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }
   }

   tab.outdent();

   return os;
}

ostream & be_predefined_type::put_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   switch (pt())
   {
      case AST_PredefinedType::PT_object:
      {
         os << tab << "os.cdr_put (((DDS::Object*)"
            << sptr << "->" << fld << ")->get_ior());" << nl;
         break;
      }

      case AST_PredefinedType::PT_any:
      {
         os << tab << "((DDS::Any*)" << sptr << "->" << fld
            << ")->put (os" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
         break;
      }

      default:
      {
         put_for_struct (os, tab, sptr, fld, uid);
         break;
      }
   }

   return os;
}

ostream & be_predefined_type::get_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   switch (pt())
   {
      case AST_PredefinedType::PT_object:
      {
         os << tab << "DDS::Object::_assert_stub("
            << sptr << "->" << fld << ".m_ptr);" << nl;
         os << tab << "is.cdr_get(((DDS::Object*)"
            << sptr << "->" << fld << ")->get_ior() "
            << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
         break;
      }

      case AST_PredefinedType::PT_any:
      {
         os << tab << "((DDS::Any*)" << sptr << "->" << fld
            << ")->get (os" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
         break;
      }

      default:
      {
         get_for_struct (os, tab, sptr, fld, uid);
         break;
      }
   }

   return os;
}

ostream & be_predefined_type::put_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   switch (pt())
   {
      case AST_PredefinedType::PT_object:
      {
         os << tab << "DDS::Codec::Param putArg = "
            << "{ " << Scope(TypeCodeTypeName()) << ", "
            << arg << "[" << index << "]->get_ior (), DDS::PARAM_IN };"
            << nl;
         os << tab << "os.put (&putArg, 1" << XBE_Ev::arg (XBE_ENV_VARN)
            << ");" << nl;
      }

      break;

      case AST_PredefinedType::PT_any:
      {
         os << tab << arg << "[" << index << "].put (os"
            << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }

      break;

      case AST_PredefinedType::PT_typecode:
      {
         os << tab << "DDS::Codec::Param putArg = "
            << "{ " << Scope (TypeCodeTypeName ()) << ", "
            << arg << "[" << index << "]" << ", DDS::PARAM_IN };"
            << nl;
         os << tab << "os.put (&putArg, 1"
            << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }
      break;

      default:
      {
         os << tab << "DDS::Codec::Param putArg = { "
            << Scope(TypeCodeTypeName())
            << ", &" << arg << "[" << index << "]"
            << ", DDS::PARAM_IN };" << nl;
         os << tab << "os.put (&putArg, 1"
            << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }
   }

   return os;
}

ostream & be_predefined_type::get_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   switch (pt ())
   {
      case AST_PredefinedType::PT_object:
      {
         os << tab << "DDS::Object::_assert_stub("
            << arg << "[" << index << "]);" << nl;
         os << tab << "IOP::IOR::release("
            << arg << "[" << index << "]->get_ior());" << nl;
         os << tab << "DDS::Codec::Param getArg = "
            << "{ " << Scope(TypeCodeTypeName()) << ", "
            << "&" << arg << "[" << index << "]->get_ior(), DDS::PARAM_IN };"
            << nl;
         os << tab << "is.get(&getArg, 1"
            << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }
      break;

      case AST_PredefinedType::PT_any:
      {
         os << tab << arg << "[" << index << "].get (is"
            << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }
      break;

      default:
      {
         if (pt () ==  AST_PredefinedType::PT_typecode)
         {
            os << tab << "DDS::release (" << arg << "[" << index << "]);" << nl;
         }

         os << tab << "DDS::Codec::Param getArg = {" << Scope (TypeCodeTypeName ())
            << ", &" << arg << "[" << index << "]" << ", DDS::PARAM_OUT };" << nl;
         os << tab << "is.get (&getArg, 1"
            << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      }
   }

   return os;
}

ostream &
be_predefined_type::put_for_array(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   return put_for_sequence(os, tab, arg, index, uid);
}

ostream &
be_predefined_type::get_for_array(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   return get_for_sequence(os, tab, arg, index, uid);
}

be_DispatchableType::en_HowStoredInDispatcher
be_predefined_type::HowStoredInDispatcher(
   const be_ArgumentDirection& direction) const
{
   switch (pt())
   {

         case AST_PredefinedType::PT_object:
         // marshals an object reference as a member, accessible by
         // .in(), .out(), and .inout(), of a class
         return STORED_IN_IOR_VAR;

         case AST_PredefinedType::PT_any:

         switch (direction)
         {

               case VT_InParam:

               case VT_InOutParam:
               return STORED_AS_STACK_VARIABLE;

               case VT_OutParam:

               case VT_Return:
               return STORED_IN_VAR;

               default:
               assert(0);

         }

         break;

         case PT_typecode:
         // when marshaling, sends a typecode as a descendant of a
         // base TypeCode class; the resulting pointer must be recast to the
         // proper TypeCode subclass when calling the servant method from
         // a dispatcher
         return STORED_IN_DESCENDANT_VAR;

         default: break;
   }
   return STORED_AS_STACK_VARIABLE;
}

bool
be_predefined_type::IsPointerTypeWhenSequenceMember() const
{
   switch (pt())
   {

         case AST_PredefinedType::PT_local_object:

         case AST_PredefinedType::PT_object:

         case AST_PredefinedType::PT_typecode:

         case AST_PredefinedType::PT_pseudo:
         return true;

         default:
         return false;
   }
}

be_CppType
be_predefined_type::CppTypeWhenSequenceMember() const
{
   if (IsPointerTypeWhenSequenceMember())
   {
      return be_CppType(TypeName() + DDSPtrExtension);
   }
   else
   {
      return be_CppType(typeName);
   }
}

void be_predefined_type::make_put_param_field
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg
)
{
   switch (pt ())
   {
      case AST_PredefinedType::PT_object:
      case AST_PredefinedType::PT_any:
      break;

      default:
      {
         os << tab << "{ " << Scope(TypeCodeTypeName()) << ", &" << arg << ", DDS::PARAM_IN }";
      }
   }
}

void be_predefined_type::make_get_param_field
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg
)
{
   switch (pt ())
   {
      case AST_PredefinedType::PT_local_object:
      case AST_PredefinedType::PT_object:
      case AST_PredefinedType::PT_any:
      break;

      default:
      {
         os << tab << "{ " << Scope(TypeCodeTypeName()) << ", &" << arg << ", DDS::PARAM_OUT }";
      }
   }
}

DDS::Boolean be_predefined_type::is_core_marshaled ()
{
   return TRUE;
}

void
be_predefined_type::generate_tc_ctor_val(
   be_Source & source)
{}


void be_predefined_type::generate_tc_dtor_val
(
   be_Source & source,
   pbbool isCounted
)
{}


void
be_predefined_type::generate_tc_put_val(
   be_Source & source)
{}


void
be_predefined_type::generate_tc_get_val(
   be_Source & source)
{}


void
be_predefined_type::generate_tc_assign_val(
   be_Source & source)
{}


DDS_StdString
be_predefined_type::swap_call(const DDS_StdString& arg)
{
   DDS_StdString ret = m_swapCall + "(*";

   if (pt() == PT_float)
   {
      ret += "((DDS::ULong*)";
   }

   ret += arg + ")";

   if (pt() == PT_float)
   {
      ret += ")";
   }

   return ret;
}
