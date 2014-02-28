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
#include "xbe_typedef.h"
#include "xbe_root.h"
#include "xbe_union.h"
#include "xbe_predefined.h"
#include "xbe_enum.h"
#include "xbe_interface.h"
#include "xbe_string.h"
#include "xbe_scopestack.h"
#include "xbe_genlist.h"
#include "cppgen_iostream.h"

// -------------------------------------------------
//  be_array implementation
// -------------------------------------------------

String_map be_array::generatedArrays (idlc_hash_str);

static const char * DDSSliceExtension = "_slice";

IMPL_NARROW_METHODS4(be_array, AST_Array, be_CodeGenerator,
                     be_DispatchableType, be_Type)
IMPL_NARROW_FROM_DECL(be_array)

be_array::be_array ()
{
   isAtModuleScope (pbfalse);
}

be_array::be_array (UTL_ScopedName *n, unsigned long ndims, UTL_ExprList *_dims)
:
   AST_Decl (AST_Decl::NT_array, n),
   AST_Array (n, ndims, _dims),
   initialized (pbfalse),
   anonymous (pbtrue),
   arraySize (0),
   matrixSize (0),
   isPrimitiveArray (pbfalse),
   baseType (0),
   m_cppScope (g_feScopeStack.Top ())
{
   isAtModuleScope (pbfalse);
   g_generatorList.Add (this);

   //
   // Note: the baseType is not known at construction time
   //
   // NOTE: Array type name is created when needed by initialize
   // NOTE: Enclosing scope is nil unless typedef'd

   arraySize = ExprToULong(dims()[0]);

   // DETERMINE MATRIX SIZE
   matrixSize = arraySize;

   for (unsigned int i = 1; i < n_dims(); i++)
   {
      matrixSize *= ExprToULong(dims()[i]);
   }
}

/*
 * SetUpTypeCode()  --  Creates and initializes ProtoTypeCode
 *
 * Recursive: calls itself for each subdimension of a multidimensional
 * array.  See paragraph on representation of arrays near end of
 * section 10.7.1 in the DDS Architecture spec.
 */

void
be_array::SetUpTypeCode(
   const unsigned int dim,   // which dimension: 0 is top (leftmost) dim

   ProtoTypeCode *proto)  // for leftmost dim: 'm_typecode' member of
//  be_array object;
// for other dims: first (and only) element
//  of 'members' of parent ProtoTypeCode
{
   proto->kind = DDS::tk_array;
   // one through loop for each sub-array that composes a
   // multidimensional array, except the top level

   if (dim == n_dims() - 1) // if bottom (rightmost) dimension
   {          // then content_type is type of element
      proto->members.push_back(baseType->m_typecode);
   }
   else        // else content_type is a sub-array: recurse
   {
      ProtoTypeCode *subproto = new ProtoTypeCode;
      SetUpTypeCode(dim + 1, subproto);
      proto->members.push_back(subproto);
   }

   proto->bounds = ExprToULong(dims()[dim]);
   proto->id = (DDS_StdString)"ARR_";
   proto->id += baseType->m_typecode->id;
   proto->id += dim;
   proto->id += (DDS_StdString)"_";
   proto->id += m_typecode->bounds;
}

void be_array::Initialize ()
{
   assert (base_type());

   if (base_type())
   {
      baseType = be_typedef::_beBase(base_type());

      assert(baseType);

      if (baseType)
      {
         DDS_StdString anonName;
         initialized = pbtrue;

         // Construct anonymous local name

         if (!localName.length ())
         {
            char* mainFilename = idl_global->main_filename()->get_string();
            DDS_StdString BaseFilename;

            BaseFilename = StripExtension(FindFilename(mainFilename));
            BaseFilename = BaseName(BaseFilename);

            localName = (DDS_StdString)"_a_";
            localName += BaseFilename;
            localName += (DDS_StdString)"_";
            localName += baseType->TypeName ();
            localName += (DDS_StdString)"_";
            localName += UniqueString::unique_string();
            ColonToBar ((char *)localName);

            for (unsigned int d = 0; d < n_dims(); d++)
            {
               localName += (DDS_StdString)"_";
               localName += BE_Globals::ulong_to_string(ExprToULong(dims()[d]));
            }
         }

         forAnyName = LocalName() + "_forany";

         arrayType = LocalName();
         sliceTypeName = arrayType + DDSSliceExtension;
         baseTypeName = baseType->StructMemberTypeName();
         sliceType = sliceTypeName;

         allocater = ArrayType() + "_alloc";
         releaser = ArrayType() + "_free";
         assigner = ArrayType() + "_copy";
         copier = ArrayType() + "_dup";

         if (BE_Globals::isocpp_new_types)
         {
            // Don't put the dimensions onto the typename
         }
         else
         {
            for (unsigned int i = 0; i < n_dims(); i++)
            {
                DDS_StdString dim = DimExpr(i);

                arrayType += dim;

                if (i > 0)
                {
                  sliceType += dim;
                }
            }
         }

         isPrimitiveArray =
         (
            baseType->IsPrimitiveType ()
            && baseType->IsFixedLength () // eliminates ANY, etc..
            && !baseType->IsEnumeratedType ()
            && baseType->IsFixedLengthPrimitiveType ()
         );

         SetUpTypeCode (0, m_typecode);

         InitializeTypeMap(this);

         anonName = LocalName ();

         m_tc_ctor_val = (DDS_StdString) anonName + "_ctor";
         m_tc_dtor_val = (DDS_StdString) anonName + "_dtor";
         m_tc_assign_val = (DDS_StdString) anonName + "_copy";
         if ((this->local () == I_TRUE) || isPrimitiveArray)
         {
            m_tc_put_val = (DDS_StdString) "0";
            m_tc_get_val = (DDS_StdString) "0";
         }
         else
         {
            m_tc_put_val = (DDS_StdString) anonName + "_put";
            m_tc_get_val = (DDS_StdString) anonName + "_get";
         }
         m_any_op_id = anonName;
      }
   }
}

void
be_array::SetAccess(be_ClientHeader& source, const DDS_StdString& access)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   source.Outdent();
   os << tab << access << nl;
   source.Indent();
}

DDS::Boolean be_array::SetName
(
   const DDS_StdString & _scope,
   const DDS_StdString & _name
)
{
   DDS::Boolean ret = FALSE;

   if (anonymous)
   {
      enclosingScope = _scope;
      localName = _name;
      anonymous = FALSE;

      Initialize();

      ret = TRUE;
   }

   return ret;
}

DDS_StdString
be_array::DimExpr(int i)
{
   ostringstream os;

   os << "[";
   dims()[i]->dump(os);
   os << "]";
   os << ends;

   return (DDS_StdString) os.str().c_str();
}

void be_array::GenerateAuxTypes (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab(source);

   DDS_StdString varArrTemplate;
   DDS_StdString varMatTemplate;

   if (IsFixedLength ())
   {
      varArrTemplate = "DDS_DCPS_FArray_var";
      varMatTemplate = "DDS_DCPS_MArray_var";
   }
   else
   {
      varArrTemplate = "DDS_DCPS_VArray_var";
      varMatTemplate = "DDS_DCPS_MArray_var";
   }

   if (n_dims () < 3)  // do not support >2 dim arrays
   {
      // Make template unique

      os << tab << "struct " << LocalName() << "_uniq_ {};" << nl;

      // var type

      os << tab << "typedef "
         << ((n_dims() == 1) ? varArrTemplate : varMatTemplate)
         << "< " << LocalName() << ", " << LocalName() << "_slice, struct "
         << LocalName() << "_uniq_> " << LocalName() << "_var;" << nl;

      // forany type

      os << tab << "typedef "
         << ((n_dims() == 1) ? "DDS_DCPS_Array_forany" : "DDS_DCPS_MArray_forany")
         << "< " << LocalName() << ", " << LocalName() << "_slice, struct "
         << LocalName() << "_uniq_> " << LocalName() << "_forany;" << nl;

      // out type

      if (!IsFixedLength())
      {
         os << tab << "typedef DDS_DCPS_VLArray_out"
            << "< " << LocalName() << ", " << LocalName() << "_slice, "
            << LocalName() << "_var, " << LocalName() << "_uniq_> "
            << LocalName() << "_out;" << nl;
      }
   }
}

void be_array::Generate (be_ClientHeader & source)
{
   DDS_StdString scopedName = Scope (typeName);
   ostream & os = source.Stream ();
   be_Tab tab (source);

   if (Generated () || imported ())
   {
      return;
   }

   // don't generate until we're in the right C++ scope

   if (!g_cppScopeStack.IsCurrentScope (m_cppScope))
   {
      return;
   }

   if (generatedArrays.find (scopedName) == generatedArrays.end ())
   {
      DDS_StdString varClassname;
      DDS_StdString fileScope;
      DDS_StdString varName;
      DDS_StdString outName;

      Generated (pbtrue);
      generatedArrays[scopedName] = scopedName;

      Initialize();

      varName = LocalName() + "_var";
      outName = LocalName() + "_out";
      fileScope = "";

      // Generate array

      if (BE_Globals::isocpp_new_types)
      {
          os << tab << "typedef ";
          /* For each dimension we nest the array template */
          for (unsigned int i = 0; i < n_dims(); i++)
          {
              /* Do not 'tidy' that trailing space */
              os << "::dds::core::array< ";
          }
          /* At the inner most level, next the type */
          os <<  BaseTypeName () << " ";
          /* We write the dimensions backwards. This is not
           * a mistake. */
          for (int i = n_dims() - 1; i >= 0; --i)
          {
              os << ", ";
              dims()[i]->dump(os);
              os << ">";
          }
          os << " " << ArrayType () << ";" << nl;
          /* Our work here is done */
          return;
      }

      os << tab << "typedef " << BaseTypeName () << " "
         << SliceType () << ";" << nl;

      os << tab << "typedef " << BaseTypeName () << " "
         << ArrayType () << ";" << nl;

      if (IsFixedLength())
      {
         os << tab << "typedef " << LocalName () << " "
            << outName << ";" << nl;
      }

      // GENERATE T_ALLOC() AND T_FREE() AND T_ASSIGN()

      if (isAtModuleScope ())
      {
         fileScope = BE_Globals::UserDLL +
                     BE_Globals::DLLExtension + "extern ";
      }
      else if (m_cppScope.ContainsAtLeastOneName ())
      {
         fileScope = "static ";
      }

      os << tab << fileScope << SliceTypeName () << " * "
         << allocater << " ();" << nl;
      os << tab << fileScope << "void " << releaser << " ("
         << SliceTypeName () << " *);" << nl;
      os << tab << fileScope << "void " << assigner
         << " (" << SliceTypeName () << "* to, const "
         << SliceTypeName () << "* from);" << nl;
      os << tab << fileScope << SliceTypeName () << " *"
         << copier << " (const " << SliceTypeName () << "* from);" << nl;

      os << nl;

      GenerateAuxTypes (source);

      if (!isPrimitiveArray)
      {
         be_root::AddPutGetOps (*this);
      }

      be_root::AddAnyOps (*this);
      be_root::AddStreamOps (*this);
      be_root::AddImplementations (*this);
      be_root::AddTypedef (*this);
      be_root::AddTypecode (*this);
   }
}

// is type stored as a pointer when it's an array element?
static bool
s_IsPointerWhenArrayElement(const be_Type& type)
{
   //
   // objects, interfaces, strings, and typecodes are stored as pointers;
   // all other types are stored as-is when they're array elements
   //

   be_interface* pInterface =
      (be_interface*)type.narrow((long) & be_interface::type_id);

   if (pInterface != 0)
   {
      return true;
   }

   be_predefined_type* pPredefined =
      (be_predefined_type*)type.narrow((long) & be_predefined_type::type_id);

   if (pPredefined != 0
         && pPredefined->pt() == AST_PredefinedType::PT_object)
   {
      return true;
   }

   if (pPredefined != 0
         && pPredefined->pt() == AST_PredefinedType::PT_local_object)
   {
      return true;
   }

   be_string* pString =
      (be_string*)type.narrow((long) & be_string::type_id);

   if (pString != 0)
   {
      return true;
   }

   if (pPredefined != 0
         && pPredefined->pt() == AST_PredefinedType::PT_typecode)
   {
      return true;
   }

   be_typedef* pTypedef =
      (be_typedef*)type.narrow((long) & be_typedef::type_id);

   if (pTypedef != 0)
   {
      return s_IsPointerWhenArrayElement(*pTypedef->get_base_be_type());
   }

   return false;
}

void be_array::isAtModuleScope (bool is_at_module)
{
   be_CodeGenerator *generator;

   be_CodeGenerator::isAtModuleScope (is_at_module);

   if (baseType)
   {
      generator = (be_CodeGenerator*)
         baseType->narrow((long) & be_CodeGenerator::type_id);

      if (generator)
      {
         generator->isAtModuleScope (is_at_module);
      }
   }
}

bool be_array::isAtModuleScope () const
{
   return be_CodeGenerator::isAtModuleScope ();
}

DDS_StdString be_array::CppScoped (const DDS_StdString & ident) const
{
   DDS_StdString scoped;

   if (m_cppScope.ContainsAtLeastOneName ())
   {
      scoped += m_cppScope.ToString ();
      scoped += "::";
      scoped += ident;
   }
   else
   {
      scoped = ident;
   }

   return scoped;
}

void be_array::GenerateAuxTypes (be_ClientImplementation & source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   DDS_StdString name = CppScoped(LocalName());

   DDS_StdString varArrTemplate;
   DDS_StdString varMatTemplate;

   os << nl << "#if DDS_USE_EXPLICIT_TEMPLATES" << nl;

   if (IsFixedLength ())
   {
      varArrTemplate = "DDS_DCPS_FArray_var";
      varMatTemplate = "DDS_DCPS_MArray_var";
   }
   else
   {
      varArrTemplate = "DDS_DCPS_VArray_var";
      varMatTemplate = "DDS_DCPS_MArray_var";
   }

   if (n_dims () < 3)  // do not support >2 dim arrays
   {
      // VAR TYPE

      os << "template class "
         << ((n_dims() == 1) ? varArrTemplate : varMatTemplate)
         << "< " << name << ", " << name << "_slice, struct "
         << name << "_uniq_>;" << nl;

      // FORANY TYPE

      os << "template class "
         << ((n_dims() == 1) ? "DDS_DCPS_Array_forany" : "DDS_DCPS_MArray_forany")
         << "< " << name << ", " << name << "_slice, struct "
         << name << "_uniq_>;" << nl;

      // OUT TYPE

      if (!IsFixedLength())
      {
         os << "template class DDS_DCPS_VLArray_out < " << name << ", " << name
            << "_slice, " << name << "_var, " << name << "_uniq_>;" << nl;
      }
   }

   os << "#endif" << nl;

   os << nl;
   os << "template <>" << nl;
   os << name << "_slice* DDS_DCPS_ArrayHelper < " << name << ", " << name
      << "_slice, " << name << "_uniq_>::alloc ()" << nl;
   os << "{" << nl;
   tab.indent ();
   os << tab << "return " << name << "_alloc ();" << nl;
   tab.outdent ();
   os << "}" << nl << nl;

   os << "template <>" << nl;
   os << "void DDS_DCPS_ArrayHelper < " << name << ", " << name << "_slice, "
      << name << "_uniq_>::copy (" << name << "_slice *to, const "
      << name << "_slice* from)" << nl;
   os << "{" << nl;
   tab.indent ();
   os << tab << name << "_copy (to, from);" << nl;
   tab.outdent ();
   os << "}" << nl << nl;

   os << "template <>" << nl;
   os << "void DDS_DCPS_ArrayHelper < " << name << ", " << name << "_slice, "
      << name << "_uniq_>::free (" << name << "_slice *ptr)" << nl;
   os << "{" << nl;
   tab.indent ();
   os << tab << name << "_free (ptr);" << nl;
   tab.outdent ();
   os << "}" << nl << nl;
}

void be_array::Generate (be_ClientImplementation& source)
{
   AST_Type * astType = be_typedef::_astBase(base_type());
   be_Type * btype = be_typedef::_beBase(base_type());
   DDS_StdString corbaULong = BE_Globals::CorbaScope("ULong");
   ostream & os = source.Stream();
   be_Tab tab(source);

   // Generate auxiliary implementations first

   GenerateAuxTypes (source);

   // allocater

   os << tab << CppScoped (SliceTypeName ()) << " * " << CppScoped (allocater) << " ()" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   os << tab << BaseTypeName () << " * ret = (" << BaseTypeName () << "*) new "
      << BaseTypeName () << " [" <<  MatrixSize () << "];" << nl;

   if (s_IsPointerWhenArrayElement (*btype))
   {
      os << tab << "for (DDS::ULong i = 0; i < " << MatrixSize ()
         << "; i++)" << nl;
      os << tab << "{" << nl;
      tab.indent ();
      if (btype->IsStringType ())
      {
         os << tab << "ret[i] = (" << btype->TypeName () << ") 0;" << nl;
      }
      else
      {
         os << tab << "ret[i] = 0;" << nl;
      }
      tab.outdent ();
      os << tab << "}" << nl;
   }

   os << tab << "return (" << SliceTypeName () << " *) ret;" << endl;
   tab.outdent ();

   os << tab << "}" << nl << nl;

   // releaser

   os << tab << "void " << CppScoped (releaser) << " (" << SliceTypeName () << " * s)" << nl;
   os << tab << "{" << nl;
   tab.indent ();

   if (s_IsPointerWhenArrayElement (*btype))
   {
      os << tab << "if (s)" << nl
         << tab << "{" << nl;
      tab.indent ();
      os << tab << BaseTypeName () << " * base = (" << BaseTypeName ()
         << "*) s;" << nl;
      os << tab << "for (DDS::ULong i = 0; i < " << MatrixSize () << "; i++)"
         << nl << tab << "{" << nl;
      tab.indent ();
      if (btype->IsStringType ())
      {
         os << tab << "base[i] = (" << btype->TypeName () << ") 0;" << nl;
      }
      else
      {
         os << tab << "base[i] = 0;" << nl;
      }
      tab.outdent ();
      os << tab << "}" << nl;
      tab.outdent ();
      os << tab << "}" << nl;
   }

   os << tab << "delete [] s;" << nl;
   tab.outdent ();
   os << tab << "}" << nl << nl;

   // assigner

   os << tab << "void " << CppScoped (assigner) << nl;
   os << tab << "(" << nl;
   tab.indent ();
   os << tab << SliceTypeName () << " * to," << nl;
   os << tab << "const " << SliceTypeName () << " * from" << nl;
   tab.outdent ();
   os << tab << ")" << nl;
   os << tab << "{" << nl;
   tab.indent ();

   // cast to vector

   os << tab << "const " << baseTypeName << "* sv = ( const " << baseTypeName << "*) from;" << nl;
   os << tab << baseTypeName << "* tv = (" << baseTypeName << "*) to;" << nl;

   // assign as vector

   os << tab << "for (" << corbaULong << " i = 0; i < " << MatrixSize() << "; i++) ";

   if (btype->IsArrayType())
   {
      be_array * ba = be_array::_narrow (astType);

      if (ba)
      {
         os << ba->Assigner("tv[i]", "sv[i]") << nl;
      }
      else
      {
         assert(pbfalse);
      }
   }
   else if (btype->IsStringType())
   {
      be_string* sbt = be_string::_narrow(astType);

      if (sbt->IsWide())
         os << "tv[i] = DDS::wstring_dup (sv[i]);" << nl;
      else
         os << "tv[i] = DDS::string_dup (sv[i]);" << nl;
   }
   else if (btype->IsInterfaceType())
   {
      os << "tv[i] = " << btype->TypeName() << "::_duplicate (("
         << SliceTypeName () << ")sv[i]);" << nl;
   }
   else
   {
      os << "tv[i] = sv[i];" << nl;
   }

   tab.outdent ();
   os << tab << "}" << nl << nl;

   // copy

   os << tab << CppScoped (SliceTypeName()) << " * " << CppScoped (copier) << nl;
   tab.indent ();
   os << tab << "(const " << SliceTypeName() << " * from)" << nl;
   tab.outdent ();
   os << tab << "{" << nl;
   tab.indent ();

   // cast to vector

   os << tab << SliceTypeName () << " * to = " << allocater << " ();" << nl;
   os << tab << assigner << " (to, from);" << nl;
   os << tab << "return to;" << nl;
   tab.outdent ();
   os << tab << "}" << nl << nl;
}

void be_array::GenerateGlobal (be_ClientHeader& source)
{
   /* With new types we have no truck with this */
   if (BE_Globals::isocpp_new_types)
   {
      return;
   }

   /* Can't see why we need this for imported types */
   if (imported())
   {
      return;
   }

   ostream & os = source.Stream ();
   DDS_StdString name = CppScoped(LocalName());

   os << "template <>" << nl;
   os << name << "_slice* DDS_DCPS_ArrayHelper < " << name << ", " << name
      << "_slice, " << name << "_uniq_>::alloc ();" << nl;

   os << "template <>" << nl;
   os << "void DDS_DCPS_ArrayHelper < " << name << ", " << name << "_slice, "
      << name << "_uniq_>::copy (" << name << "_slice *to, const "
      << name << "_slice* from);" << nl;

   os << "template <>" << nl;
   os << "void DDS_DCPS_ArrayHelper < " << name << ", " << name << "_slice, "
      << name << "_uniq_>::free (" << name << "_slice *ptr);" << nl;
}

void be_array::InitializeTypeMap (be_Type* t)
{
   idlType = t;

   DDS_StdString retName;

   t->TypeName(t->Scope(t->LocalName()));
   t->InTypeName(t->TypeName());
   t->InOutTypeName(t->TypeName());
   t->StructMemberTypeName(t->TypeName());
   t->SequenceMemberTypeName(t->TypeName());

   be_array * t_array = 0;

   if ((t_array = (be_array*)t->narrow((long) & be_array::type_id)))
   {
      if (BE_Globals::isocpp_new_types)
      {
          t->UnionMemberTypeName(t_array->ArrayType());
      }
      else
      {
          t->UnionMemberTypeName(t_array->SliceTypeName() + "*");
      }
      t->DMFAdtMemberTypeName(t_array->SliceTypeName() + "*");
      t->ReturnTypeName(t_array->SliceTypeName() + "*");
      retName = t->Scope(t_array->SliceTypeName());
   }
   else if (t->narrow((long)&be_typedef::type_id))
   {
      if (BE_Globals::isocpp_new_types)
      {
          t->UnionMemberTypeName(t->Scope(t->LocalName()));
      }
      else
      {
          t->UnionMemberTypeName(t->Scope(t->LocalName()) + DDSSliceExtension + "*");
      }
      t->DMFAdtMemberTypeName(t->Scope(t->LocalName()) + DDSSliceExtension + "*");
      t->ReturnTypeName(t->Scope(t->LocalName()) + DDSSliceExtension + "*");
      retName = t->Scope(t->LocalName()) + DDSSliceExtension;
   }
   else
   {
      assert(0);
   }

   t->VarSignature(VT_InParam, t->TypeName(), VT_Const, VT_Var, VT_NonReference);
   t->VarSignature(VT_InOutParam, t->TypeName(), VT_NonConst, VT_Var, VT_NonReference);
   t->VarSignature(VT_Return, retName, VT_NonConst, VT_Pointer, VT_NonReference);

   if (t->IsFixedLength())
   {
      t->OutTypeName(t->TypeName() + DDSOutExtension);
      t->VarSignature(VT_OutParam, t->OutTypeName(), VT_NonConst, VT_Var, VT_NonReference);
   }
   else
   {
      t->OutTypeName(t->TypeName() + DDSOutExtension);
      t->VarSignature(VT_OutParam, t->OutTypeName(), VT_NonConst, VT_Var, VT_NonReference);
   }

   t->TypeCodeTypeName(BE_Globals::TCPrefix + t->LocalName());
   t->TypeCodeBaseName(BE_Globals::TCBasePrefix + t->LocalName());
   t->TypeCodeRepName(BE_Globals::TCRepPrefix + t->LocalName());
   t->MetaTypeTypeName(BE_Globals::MTPrefix + t->TypedefName());
}

pbbool be_array::IsFixedLength () const
{
   be_array * my = (be_array*) this;
   pbbool ret = pbtrue;
   be_Type * bbase = be_typedef::_beBase (my->base_type ());

   if (bbase)
   {
      ret = bbase->IsFixedLength ();
   }
   else
   {
      assert (pbfalse);
   }

   return ret;
}

pbbool be_array::IsFixedLengthPrimitiveType () const
{
   return FALSE;
}

void be_array::GenerateTypedefs
(
   const DDS_StdString &scope,
   const be_typedef& alias,
   be_ClientHeader& source
)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   DDS_StdString fileScope = "";

   DDS_StdString aliasReleaser = alias.LocalName() + "_free";
   DDS_StdString aliasAllocater = alias.LocalName() + "_alloc";
   DDS_StdString aliasAssigner = alias.LocalName() + "_copy";
   DDS_StdString aliasDuplicator = alias.LocalName() + "_dup";
   DDS_StdString aliasSliceName = alias.LocalName() + DDSSliceExtension;

   if (!Generated())
   {
      Generate(source);
   }
   else
   {
      DDS_StdString relTypeName = BE_Globals::RelativeScope (scope, typeName);
      os << tab << "typedef " << relTypeName << " "
      << alias.LocalName() << ";" << nl;

      os << tab << "typedef " << relTypeName << DDSSliceExtension << " "
      << alias.LocalName() << DDSSliceExtension << ";" << nl;

      if (n_dims() < 3)
      {
         os << tab << "typedef " << relTypeName << "_forany" << " "
         << alias.LocalName() << "_forany" << ";" << nl;
         os << tab << "typedef " << relTypeName << DDSVarExtension << " "
         << alias.LocalName() << DDSVarExtension << ";" << nl;
      }

      if (!IsFixedLength())
      {
         os << tab << "typedef " << relTypeName << DDSOutExtension << " "
         << alias.LocalName() << DDSOutExtension << ";" << nl;
      }

      // GENERATE T_ALLOC() AND T_FREE() AND T_ASSIGN()
      if (alias.isAtModuleScope ())  // Declare file-scope help functions
      {
         fileScope = BE_Globals::UserDLL + BE_Globals::DLLExtension + "inline ";
      }
      else if (alias.EnclosingScope ().length () == 0)
      {
         fileScope = "static ";
      }

      //
      // allocater
      //
      os << tab << fileScope << aliasSliceName << " * " << aliasAllocater;

      os << "()" << nl
         << tab << "{" << nl
         << tab << tab << "return (" << aliasSliceName << "*)" << allocater
         << "();" << nl
         << tab << "}" << nl;

      //
      // releaser
      //
      os << tab << fileScope << "void " << aliasReleaser << "("
         << aliasSliceName << " *_slice_)"
         << tab << "{ " << nl
         << tab << tab << releaser << "(_slice_);" << nl
         << tab << "}" << nl;

      //
      // assigner
      //
      os << tab << fileScope << "void " << aliasAssigner
         << "(" << aliasSliceName << "* _to_, const "
         << aliasSliceName << "* _from_)" << nl;
      os << tab << "{" << nl
         << tab << tab << assigner << "(_to_, _from_);" << nl
         << tab << "}" << nl;

      //
      // dup
      //
      os << tab << fileScope << aliasSliceName << " * " << aliasDuplicator
         << "(" << aliasSliceName << "* _from_)" << nl;
      os << tab << "{" << nl
         << tab << tab << aliasSliceName << "* to = " << aliasAllocater
         << " ();" << nl
         << tab << tab << assigner << "(to, _from_);" << nl
         << tab << tab << "return to;" << nl
         << tab << "}" << nl;
   }
}

DDS_StdString
be_array::NullReturnArg()
{
   DDS_StdString ret = (DDS_StdString)"(" + Scope(SliceTypeName()) + "*)0";

   return ret;
}

DDS_StdString
be_array::Allocater(const DDS_StdString& arg) const
{
   return arg + " = " + Scope(allocater) + "();";
}

DDS_StdString
be_array::Initializer(const DDS_StdString& arg, VarType vt) const
{
   DDS_StdString ret = arg;

   if (vt == VT_Return || (!IsFixedLength() && (vt == VT_OutParam || vt == VT_Return)))
   {
      ret = arg + " = 0";
   }

   return ret + ";";
}

DDS_StdString
be_array::InRequestArgumentDeclaration(be_Type& btype, const DDS_StdString& arg, VarType vt)
{
   DDS_StdString ret = btype.TypeName();

   if (vt == VT_Return || (!IsFixedLength() && vt == VT_OutParam))
   {
      ret += "_var " + arg + ";";
   }
   else if (!IsFixedLength() && vt == VT_InOutParam)
   {
      ret = Allocater(ret + "_var");
   }
   else
   {
      ret += " " + arg + ";";
   }

   return ret;
}

DDS_StdString
be_array::Releaser(const DDS_StdString& arg) const
{
   if (BE_Globals::isocpp_new_types)
   {
      DDS_StdString str("delete ");
      return str + arg + ";";
   }
   return Scope(releaser) + "(" + arg + ");";
}

DDS_StdString
be_array::Assigner(const DDS_StdString& arg1, const DDS_StdString& arg2) const
{
   DDS_StdString ret;
   if (BE_Globals::isocpp_new_types)
   {
    ret = arg1 + " = " + arg2 + ";";
   }
   else
   {
   // casts prevent warnings in union set functions
    ret = CppScoped(assigner) + "((" + CppScoped(SliceTypeName()) + "*) " + arg1
      + ", (const " + CppScoped(SliceTypeName()) + " *) " + arg2 + ");";
   }
   return ret;
}

DDS_StdString be_array::Duplicater
(
   const DDS_StdString & arg,
   const DDS_StdString & val,
   const DDS_StdString & scope,
   const pbbool isConst
) const
{
   DDS_StdString ret;
   if (BE_Globals::isocpp_new_types)
   {
      ret = arg + " = new " + ArrayType() + "(" + val + ");";
   }
   else
   {
      ret = Allocater (arg) + " " + Assigner(arg, val);
   }
   return ret;
}

DDS_StdString
be_array::SyncStreamOut(const DDS_StdString& arg, const DDS_StdString &out, VarType /*vt*/) const
{
   DDS_StdString ret;

   if (baseType->IsPrimitiveType())
   {
      const char * noderef = out;

      ret = (DDS_StdString)"DDS_PUT_ARRAY(" +
            &noderef[1] + "," + arg + "," +
            DDS_StdString(BE_Globals::ulong_to_string(MatrixSize())) +
            "," + BaseTypeName() + ");";
   }
   else
   {
      DDS_StdString cast = Scope(SliceTypeName()) + "* _t = (" +
                           Scope(SliceTypeName()) + "*)" + arg +
                           ";" + BaseTypeName() + "* _t_ = (" +
                           BaseTypeName() + "*)_t; ";

      if (baseType->IsOpaqueType())
      {
         ret = (DDS_StdString)"{ " +
               cast + "for(DDS::ULong i=0;i < " +
               DDS_StdString(BE_Globals::ulong_to_string(MatrixSize())) +
               ";i++) " + out + " << (_t_[i]).ptr_; }";
      }
      else
      {
         ret = (DDS_StdString)"{ " +
               cast + "for(DDS::ULong i=0;i < " +
               DDS_StdString(BE_Globals::ulong_to_string(MatrixSize())) +
               ";i++) " + out + " << _t_[i]; }";
      }
   }

   return ret;
}


DDS_StdString be_array::SyncStreamIn
(
   const DDS_StdString& arg,
   const DDS_StdString &in,
   VarType /*vt*/
) const
{
   DDS_StdString ret;

   if (baseType->IsPrimitiveType())
   {
      const char * noderef = in;

      ret = (DDS_StdString)"DDS_GET_ARRAY(" + &noderef[1] + "," +
            arg + "," +
            DDS_StdString(BE_Globals::ulong_to_string(MatrixSize())) +
            "," + BaseTypeName() + ");";
   }
   else
   {
      DDS_StdString cast = Scope(SliceTypeName()) + "* _t = " + arg +
                           ";" + BaseTypeName() + "*& _t_ = (" +
                           BaseTypeName() + "*&)_t; ";

      if (baseType->IsOpaqueType())
      {
         ret = (DDS_StdString)"{ " +
               cast + "for(DDS::ULong i=0;i < " +
               DDS_StdString(BE_Globals::ulong_to_string(MatrixSize())) +
               ";i++) " + in + " >> (_t_[i]).ptr_; }";
      }
      else
      {
         ret = (DDS_StdString)"{ " +
               cast + "for(DDS::ULong i=0;i < " +
               DDS_StdString(BE_Globals::ulong_to_string(MatrixSize()))
               + ";i++) " + in + " >> _t_[i]; }";
      }
   }

   return ret;
}


DDS_StdString
be_array::StructStreamIn(const DDS_StdString& arg, const DDS_StdString &in) const
{
   DDS_StdString ret;

   DDS_StdString cast = Scope(SliceTypeName()) + "* _t = " + arg +
                        ";" + BaseTypeName() + "*& _t_ = (" + BaseTypeName() + "*&)_t; ";

   ret = (DDS_StdString)"{ " +
         cast + "for(DDS::ULong i=0;i < " +
         DDS_StdString(BE_Globals::ulong_to_string(MatrixSize())) +
         ";i++) " + in + " >> _t_[i]; }";
   return ret;
}

DDS_StdString
be_array::StructStreamOut(const DDS_StdString& arg, const DDS_StdString &out) const
{
   DDS_StdString ret;

   DDS_StdString cast = Scope(SliceTypeName()) + "* _t = (" +
                        Scope(SliceTypeName()) + "*)" + arg +
                        ";" + BaseTypeName() + "* _t_ = (" + BaseTypeName() + "*)_t; ";

   ret = (DDS_StdString)"{ " +
         cast + "for(DDS::ULong i=0;i < " +
         DDS_StdString(BE_Globals::ulong_to_string(MatrixSize())) +
         ";i++) " + out + " << _t_[i]; }";

   return ret;
}

be_array *
be_array::_narrow(AST_Type * atype)
{
   be_array * ret = 0;

   if (atype)
   {
      ret = (be_array*)atype->narrow((long) & be_array::type_id);
   }

   return ret;
}

be_array *
be_array::_narrow_from_alias(AST_Type * atype)
{
   AST_Type * baseType = be_typedef::_astBase(atype);

   return be_array::_narrow(baseType);
}

DDS::Boolean be_array::is_core_marshaled ()
{
   DDS::Boolean ret = FALSE;

   if (isPrimitiveArray)
   {
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_array::declare_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & stubScope,
   VarType vt
)
{
   DDS::Boolean ret = TRUE;
   DDS_StdString relTypeName = BE_Globals::RelativeScope(
                                  stubScope,
                                  typeName);
   DDS_StdString relAllocater = BE_Globals::RelativeScope(
                                   stubScope,
                                   Scope(allocater));

   switch (vt)
   {
      case VT_OutParam:
      {
         if (!IsFixedLength())
         {
            DDS_StdString dim = DimExpr(n_dims() - 1);

            os << tab << arg << " = " << relAllocater << " ();" << nl;
         }
         break;
      }

      case VT_Return:
      {
         DDS_StdString dim = DimExpr(n_dims() - 1);
         DDS_StdString relSliceName = BE_Globals::RelativeScope(
                                         stubScope,
                                         Scope(sliceTypeName));

         os << tab << relTypeName << "_var " << arg
            << " = " << relAllocater << " ();" << nl;
         break;
      }

      case VT_InParam:
      case VT_InOutParam:
      default:
      {
         break;
      }
   }

   return ret;
}

DDS::Boolean
be_array::declare_for_struct_put(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return FALSE;
}


DDS::Boolean
be_array::declare_for_struct_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return FALSE;
}


DDS::Boolean
be_array::declare_for_union_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   os << tab << sptr << "->" << fld << " = " << CppScoped(allocater) << "();" << nl;

   return TRUE;
}


DDS::Boolean be_array::make_get_param_for_stub
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
      os << tab << "{ " << Scope (TypeCodeTypeName ()) << ", ";

      switch (vt)
      {
         case VT_InOutParam:
         {
            os << arg << ", DDS::PARAM_OUT ";
            break;
         }

         case VT_OutParam:
         {
            os << arg << ", DDS::PARAM_OUT ";
            break;
         }

         case VT_Return:
         {
            os << arg << ", DDS::PARAM_OUT ";
            break;
         }

         default:
         {
            break;
         }
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_array::make_put_param_for_stub
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
      os << tab << "{ " << Scope (TypeCodeTypeName ()) << ", ";

      switch (vt)
      {
         case VT_InParam:
            os << "(" << TypeName () << "*)";
         // NO break here !

         case VT_InOutParam:
            os << arg << ", DDS::PARAM_IN ";
         default:
            break;
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_array::make_put_param_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << CppScoped(TypeCodeTypeName()) << ", ";
   os << sptr << "->" << fld << ", DDS::PARAM_IN ";
   os << "}";

   return TRUE;
}


DDS::Boolean be_array::make_get_param_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << CppScoped(TypeCodeTypeName()) << ", ";
   os << sptr << "->" << fld << ", DDS::PARAM_OUT ";
   os << "}";

   return TRUE;
}

DDS::Boolean be_array::make_put_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << CppScoped(TypeCodeTypeName()) << ", ";
   os << sptr << "->" << fld << ", DDS::PARAM_IN ";
   os << "}";

   return TRUE;
}


DDS::Boolean be_array::make_get_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << CppScoped(TypeCodeTypeName()) << ", ";
   os << sptr << "->" << fld << ", DDS::PARAM_OUT ";
   os << "}";

   return TRUE;
}


ostream & be_array::put_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   DDS_StdString arrptr = sptr;

   arrptr += "->" + fld;
   putter (os, tab, arrptr, uid++);

   return os;
}


ostream &
be_array::get_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   DDS_StdString arrptr = sptr;

   arrptr += "->" + fld;
   getter(os, tab, arrptr, uid++);

   return os;
}


ostream &
be_array::put_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   DDS_StdString arrptr = fld;

   arrptr += uid;

   os << tab << sliceTypeName << " * ___" << arrptr
   << " = " << "(" << sliceTypeName << "*)" << sptr
   << "->" << fld << ";" << nl;

   putter(os, tab, arrptr, uid++);

   return os;
}


ostream &
be_array::get_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   DDS_StdString arrptr = fld;

   arrptr += uid;

   os << tab << sliceTypeName << " * ___" << arrptr
   << " = " << "(" << sliceTypeName << "*)" << sptr
   << "->" << fld << ";" << nl;

   getter(os, tab, arrptr, uid++);

   return os;
}


ostream &be_array::put_for_sequence
(
   ostream &os,
   be_Tab &tab,
   const DDS_StdString &arg,
   const DDS_StdString &index,
   unsigned long uid
)
{
   DDS_StdString put_val = BE_Globals::ScopeOf(ScopedName()) + "::"
      + TypeCodeTypeName() + "->put_val";

   os << tab << put_val << "(os, (void*)&" << arg << "[" << index << "],"
      << " DDS::PARAM_IN" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream &be_array::get_for_sequence
(
   ostream &os,
   be_Tab &tab,
   const DDS_StdString &arg,
   const DDS_StdString &index,
   unsigned long uid
)
{
   DDS_StdString get_val = BE_Globals::ScopeOf(ScopedName()) + "::"
      + TypeCodeTypeName() + "->get_val";

   os << tab << get_val << "(is, (void*)&" << arg << "[" << index << "], "
      << "DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream &be_array::put_for_array
(
   ostream &os,
   be_Tab &tab,
   const DDS_StdString &arg,
   const DDS_StdString &index,
   unsigned long uid
)
{
   DDS_StdString put_val = BE_Globals::ScopeOf(ScopedName()) + "::"
      + TypeCodeTypeName() + "->put_val";

   os << tab << put_val << "(os, (void*)&p[" << index << "],"
      << " DDS::PARAM_IN" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream &be_array::get_for_array
(
   ostream &os,
   be_Tab &tab,
   const DDS_StdString &arg,
   const DDS_StdString &index,
   unsigned long uid
)
{
   DDS_StdString get_val = BE_Globals::ScopeOf(ScopedName()) + "::"
      + TypeCodeTypeName() + "->get_val";

   os << tab << get_val << "(is, (void*)&p[" << index << "], "
      << "DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

void be_array::generate_tc_ctor_val (be_Source & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   // declare ctor body

   os << "#ifndef _eorb" << m_tc_ctor_val << "_" << nl;
   os << "#define _eorb" << m_tc_ctor_val << "_" << nl;
   os << "static void * " << m_tc_ctor_val << " ()" << nl;
   os << "{" << nl;

   // just allocate and return

   tab.indent ();
   os << tab << "return " << CppScoped (allocater) << " ();" << nl;
   tab.outdent ();
   os << "}" << nl;
   os << "#endif" << nl << nl;
}

void be_array::generate_tc_dtor_val
(
   be_Source & source,
   pbbool isCounted
)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   // declare dtor body

   os << "#ifndef _eorb" << m_tc_dtor_val << "_" << nl;
   os << "#define _eorb" << m_tc_dtor_val << "_" << nl;
   os << "static void " << m_tc_dtor_val << " (void * arg)" << nl;
   os << "{" << nl;
   tab.indent ();
   os << tab << CppScoped (releaser) << " ((" << CppScoped (sliceTypeName) << "*) arg);" << nl;
   tab.outdent();
   os << "}" << nl;
   os << "#endif" << nl << nl;
}

void be_array::putter
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arrptr,
   unsigned long uid
)
{

   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString length = BE_Globals::ulong_to_string(MatrixSize());
   DDS_StdString size = BE_Globals::ulong_to_string(MatrixSize() *
                        baseType->get_elem_size());
   DDS_StdString index = (DDS_StdString)"i" + uids;

   os << tab << "for (DDS::ULong " << index << " = 0; " << index
      << " < " << length << "; " << index << "++)" << nl;
   os << tab << "{" << nl;
   tab.indent();
   baseType->put_for_array(os, tab, arrptr, index, uid);
   tab.outdent();
   os << tab << "}" << nl;

/*
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString length = BE_Globals::ulong_to_string(MatrixSize());
   DDS_StdString size = BE_Globals::ulong_to_string(MatrixSize() *
                        baseType->get_elem_size());
   DDS_StdString index = (DDS_StdString)"i" + uids;

   os << tab << "for (DDS::ULong " << index << " = 0; " << index
      << " < " << length << "; " << index << "++)" << nl;
   os << tab << "{" << nl;
   tab.indent();
   baseType->put_for_array(os, tab, arrptr, index, uid);
   tab.outdent();
   os << tab << "}" << nl;
*/
}


void be_array::generate_tc_put_val (be_Source & source)
{
   // put and get for array of primitive
   // is handled in the core

   if (isPrimitiveArray)
   {
      return;
   }

   ostream & os = source.Stream ();
   be_Tab tab (source);

   //
   // declare writer body
   //
   os << "#ifndef _eorb" << m_tc_put_val << "_" << nl;
   os << "#define _eorb" << m_tc_put_val << "_" << nl;
   os << "static void " << m_tc_put_val << nl;
   os << "(" << nl;
   tab.indent ();
   os << tab << "DDS::Codec::OutStream & os," << nl;
   os << tab << "const void * arg," << nl;
   os << tab << "DDS::ParameterMode mode" << nl;
   if (XBE_Ev::generate ())
   {
      os << tab << XBE_Ev::arg (XBE_ENV_ARGN, false) << nl;
   }
   tab.outdent ();
   os << ")" << nl;
   os << "{" << nl;
   tab.indent ();

   // first, cast that pesky void *
   //
   // Pointer is to type of the elements of the lowest dimension of
   // the array.  This enables putter() to treat the array as if it
   // is one-dimensional regardless of how many dimensions it has.
   //
   os << tab << baseTypeName << " * p = (" << baseTypeName << "*) arg;" << nl << nl;

   // now put the data

   putter (os, tab, "p", 0);
   tab.outdent ();

   os << "}" << nl;
   os << "#endif" << nl << nl;
}

void be_array::getter
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arrptr,
   unsigned long uid
)
{
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString length = BE_Globals::ulong_to_string(MatrixSize());
   DDS_StdString size = BE_Globals::ulong_to_string(MatrixSize() *
                        baseType->get_elem_size());
   DDS_StdString index = (DDS_StdString)"i" + uids;
   DDS_StdString swapCall;
   DDS_StdString swapArg;
   DDS_StdString incData;

   os << tab << "for (DDS::ULong " << index << " = 0; "
      << index << " < " << length << "; " << index << "++)" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   baseType->get_for_array (os, tab, arrptr, index, uid);
   tab.outdent ();
   os << tab << "}" << nl;
}

void be_array::generate_tc_get_val (be_Source & source)
{
   // put and get for array of primitive
   // is handled in the core

   if (isPrimitiveArray)
   {
      return;
   }

   ostream & os = source.Stream ();
   be_Tab tab (source);

   // declare reader body

   os << "#ifndef _eorb" << m_tc_get_val << "_" << nl;
   os << "#define _eorb" << m_tc_get_val << "_" << nl;
   os << "static void " << m_tc_get_val << nl;
   os << "(" << nl;
   tab.indent ();
   os << tab << "DDS::Codec::InStream & is," << nl;
   os << tab << "void * arg," << nl;
   os << tab << "DDS::ParameterMode mode" << nl;
   if (XBE_Ev::generate ())
   {
      os << tab << XBE_Ev::arg (XBE_ENV_ARGN, false) << nl;
   }
   tab.outdent ();
   os << ")" << nl;
   os << "{" << nl;
   tab.indent ();

   // first, cast that pesky void *
   //
   // Pointer is to the type of the elements of the lowest dimension of
   // the array.  This enables getter() to treat the array as if it
   // were one-dimensional regardless of how many dimensions it has.

   os << tab << baseTypeName << " * p = (" << baseTypeName << "*) arg;" << nl << nl;

   // now, get the data

   getter (os, tab, "p", 0);
   tab.outdent ();

   os << "}" << nl;
   os << "#endif" << nl << nl;
}

void be_array::generate_tc_assign_val (be_Source & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   // declare assign body

   os << "#ifndef _eorb" << m_tc_assign_val << "_" << nl;
   os << "#define _eorb" << m_tc_assign_val << "_" << nl;
   os << "static void " <<  m_tc_assign_val << " (void * dest, void * src)" << nl;
   os << "{" << nl;
   tab.indent ();

   // just call array's assign

   os << tab << CppScoped (assigner) << " ((" << CppScoped (sliceTypeName)
      << " *) dest, (const " << CppScoped (sliceTypeName) << " *) src);" << nl;
   tab.outdent ();
   os << "}" << nl;
   os << "#endif" << nl << nl;
}

be_DispatchableType::en_HowStoredInDispatcher
be_array::HowStoredInDispatcher(
   const be_ArgumentDirection& direction) const
{
   if (IsFixedLength ())
   {
      switch (direction)
      {
         case VT_InParam:
         case VT_InOutParam:
         case VT_OutParam:
            return STORED_AS_STACK_VARIABLE;

         case VT_Return:
            return STORED_IN_VAR;
         default:
            assert (0);
      }
   }
   else // variable-length array
   {
      switch (direction)
      {
         case VT_InParam:
            return STORED_AS_STACK_VARIABLE;

         case VT_OutParam:
         case VT_Return:
            return STORED_IN_VAR;

         case VT_InOutParam:
            return STORED_IN_ALLOCED_VAR;
         default:
            assert (0);
      }
   }

   return STORED_AS_STACK_VARIABLE; // should never execute this line
}

void
be_array::NilOutArray(
   ostream& os,
   be_Tab& tab,
   const be_CppName& argName) const
{
   be_CppName index = (DDSString)"__i" + argName;

   os << tab << "for (DDS::ULong " << index << " = 0; "
   << index << " < " << MatrixSize() << "; " << index << "++)" << nl;
   os << tab << "{" << nl;
   tab.indent();
   os << tab << argName << "[" << index << "] = 0;" << nl;
   tab.outdent();
   os << tab << "}" << nl;
}

void
be_array::InitializeInDispatcher(
   ostream& os,
   be_Tab& tab,
   const be_CppName& argName,
   const be_ArgumentDirection& direction) const
{
   switch (direction)
   {

         case VT_InParam:

         case VT_InOutParam:

         if (baseType->IsInterfaceType())
         {
            // We need to null out an array of interfaces when In or InOut,
            // because when get args puts the array from the client into the
            // dispatcher's array, the assignment function will try to free
            // any existing (non-null) pointer in the array.
            NilOutArray(os, tab, argName);
         }

         break;

         case VT_OutParam:

         case VT_Return:
         break;

         default:
         assert(0);
   }
}

DDS_StdString
be_array::kind_string()
{
   return "DDS::tk_array";
}

DDS::ULong
be_array::get_elem_size()
{
   return baseType->get_elem_size(); // eas
   // return 0;
}

DDS::ULong
be_array::get_elem_alignment()
{
   return baseType->get_elem_alignment();
}

DDS::Boolean
be_array::is_primitive_array()
{
   return isPrimitiveArray;
}
