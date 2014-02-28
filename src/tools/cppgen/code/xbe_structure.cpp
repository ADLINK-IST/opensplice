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
#include "xbe_structure.h"
#include "xbe_field.h"
#include "xbe_typedef.h"
#include "xbe_array.h"
#include "xbe_utils.h"
#include "xbe_root.h"
#include "xbe_sequence.h"
#include "xbe_cppfwd.h"
#include "xbe_scopestack.h"
#include "xbe_genlist.h"

// -------------------------------------------------
//  BE_STRUCTURE IMPLEMENTATION
// -------------------------------------------------

IMPL_NARROW_METHODS4(be_structure, AST_Structure, be_CodeGenerator,
                     be_DispatchableType, be_Type)
IMPL_NARROW_FROM_DECL(be_structure)
IMPL_NARROW_FROM_SCOPE(be_structure)

be_structure::be_structure ()
:  m_canOptimize(pbfalse),
   m_lastFieldSize(0),
   m_interface_dependant(pbfalse)
{
   isAtModuleScope(pbfalse);
}

be_structure::be_structure(UTL_ScopedName *n, const UTL_Pragmas &p)
   :
   AST_Decl (AST_Decl::NT_struct, n, p),
   UTL_Scope (AST_Decl::NT_struct, n, p),
   m_isFixedLength (pbtrue),
   m_elemAlignment (0),
   m_maxElemAlignment (1),
   m_elemSize (0),
   m_canOptimize (pbfalse),
   m_lastFieldSize (0),
   m_cppScope (g_feScopeStack.Top()),
   m_cppType (g_feScopeStack.Top(), *n),
   m_interface_dependant (pbfalse)
{
   DDS_StdString barScopedName = NameToString(name(), "_");
   isAtModuleScope(pbfalse);

   localName = local_name()->get_string();
   enclosingScope = be_Type::EnclosingScopeString(this);

   m_tc_ctor_val = (DDS_StdString) barScopedName + "_ctor";
   m_tc_dtor_val = (DDS_StdString) barScopedName + "_dtor";
   m_tc_put_val = (DDS_StdString) barScopedName + "_put";
   m_tc_get_val = (DDS_StdString) barScopedName + "_get";
   m_tc_assign_val = (DDS_StdString) barScopedName + "_copy";

   m_any_op_id = barScopedName;
   m_nullArg = (DDS_StdString)"*(new " + ScopedName() + ")";
   m_typecode->kind = DDS::tk_struct;
   m_typecode->id = get_decl_pragmas().get_repositoryID()->get_string();
   m_typecode->name_of_type = localName;
   m_marshalInCore = FALSE;

   InitializeTypeMap(this);
   if (!imported())
      be_CppFwdDecl::Add(be_CppFwdDecl::STRUCT, this, m_cppScope);
}

DDS::Boolean
be_structure::IsFixedLength() const
{
   return m_isFixedLength;
}

DDS::Boolean
be_structure::IsFixedLengthPrimitiveType() const
{
   return FALSE;
}

pbbool be_structure::IsOptimizable ()
{
   return m_canOptimize && IsFixedLength ();
}

pbbool be_structure::IsInterfaceDependant () const
{
   return m_interface_dependant;
}

void
be_structure::InitializeTypeMap(be_Type* t)
{
   idlType = t;

   if (t)
   {
      AST_Type * t_ast = (AST_Type*)t->narrow((long) & AST_Type::type_id);

      assert(t_ast);

      t->TypeName(NameToString(t_ast->name()));
      t->InTypeName("const " + t->TypeName() + "&");
      t->InOutTypeName(t->TypeName() + "&");

      t->VarSignature(VT_InParam, t->TypeName(), VT_Const, VT_Var, VT_Reference);
      t->VarSignature(VT_InOutParam, t->TypeName(), VT_NonConst, VT_Var, VT_Reference);

      if (t->IsFixedLength())
      {
         t->OutTypeName(t->TypeName() + "&");
         t->ReturnTypeName(t->TypeName());
         t->VarSignature(VT_OutParam, t->TypeName(), VT_NonConst, VT_Var, VT_Reference);
         t->VarSignature(VT_Return, t->TypeName(), VT_NonConst, VT_Var, VT_NonReference);
      }
      else
      {
         t->OutTypeName(t->TypeName() + DDSOutExtension);
         t->ReturnTypeName(t->TypeName() + "*");

         t->VarSignature(VT_OutParam, t->OutTypeName(), VT_NonConst, VT_Var, VT_NonReference);
         t->VarSignature(VT_Return, t->TypeName(), VT_NonConst, VT_Pointer, VT_NonReference);
      }

      t->DMFAdtMemberTypeName(t->TypeName() + "*");
      t->StructMemberTypeName(t->TypeName());
      t->UnionMemberTypeName(t->TypeName());
      t->SequenceMemberTypeName(t->TypeName());

      DDS_StdString basename = NameToString(t_ast->name(), "_");
      t->TypeCodeTypeName(BE_Globals::TCPrefix + t->LocalName());
      t->MetaTypeTypeName(BE_Globals::MTPrefix + basename);
      t->TypeCodeBaseName(BE_Globals::TCBasePrefix + basename);
      t->TypeCodeRepName(BE_Globals::TCRepPrefix + basename);
   }
}

void
be_structure::GenerateTypedefs( const DDS_StdString &scope,
                                const be_typedef& alias,
                                be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   DDS_StdString relTypeName = BE_Globals::RelativeScope(scope, typeName);

   os << tab << "typedef " << relTypeName << " "
   << alias.LocalName() << ";" << nl;

   if (BE_Globals::isocpp_new_types)
     return;

   os << tab << "typedef " << relTypeName << DDSVarExtension << " "
   << alias.LocalName() << DDSVarExtension << ";" << nl;

   if (!m_isFixedLength)
   {
      os << tab << "typedef " << relTypeName << DDSOutExtension << " "
      << alias.LocalName() << DDSOutExtension << ";" << nl;
   }
}

DDS_StdString
be_structure::NullReturnArg()
{
   DDS_StdString ret = m_nullArg;

   if (!m_isFixedLength)
   {
      ret = (DDS_StdString)"(" + ScopedName() + "*)0";
   }

   return ret;
}

DDS_StdString
be_structure::Allocater(const DDS_StdString& arg) const
{
   DDS_StdString ret = arg + " = new " + typeName + ";";

   return ret;
}

DDS_StdString
be_structure::Initializer(const DDS_StdString& arg, VarType vt) const
{
   DDS_StdString ret = arg ;

   if (!m_isFixedLength && (vt == VT_OutParam || vt == VT_Return))
   {
      ret = arg + " = 0";
   }

   return ret + ";";
}

DDS_StdString be_structure::Releaser (const DDS_StdString & arg) const
{
   return (DDS_StdString)"if (" + arg + ") delete " + arg + ";";
}

DDS_StdString
be_structure::Assigner(const DDS_StdString& arg, const DDS_StdString& val) const
{
   return arg + " = " + val + ";";
}

DDS_StdString be_structure::Duplicater
(
   const DDS_StdString& arg,
   const DDS_StdString& val,
   const DDS_StdString& currentScope,
   const pbbool isConst
) const
{
   DDS_StdString relativeName = BE_Globals::RelativeScope(currentScope, typeName);
   DDS_StdString ret = arg + " = new "
                       + relativeName + "(" + val + ");";
   return ret;
}

DDS_StdString
be_structure::SyncStreamOut(const DDS_StdString& arg, const DDS_StdString& out, VarType vt) const
{
   DDS_StdString ret = out;

   if (!m_isFixedLength && (vt == VT_Return || vt == VT_OutParam))
   {
      ret += " << (" + ScopedName() + " *)" + arg + ";";
   }
   else
   {
      ret += " << " + arg + ";";
   }

   return ret;
}

AST_Field *
be_structure::add_field(AST_Field *af)
{
   if (AST_Structure::add_field(af))
   {
      be_field * field;

      if ((field = be_field::_narrow(af)))
      {
         field->initialize();

         //
         // re-initialize typemap
         //

         if (m_isFixedLength && !field->IsFixedLength())
         {
            is_fixed_length(FALSE);
            InitializeTypeMap(this);
         }

         //
         // determine if marshaling should be done in the core
         //
         if (field->is_core_marshaled())
         {
            m_marshalInCore = TRUE;
         }

         //
         // update type stats
         //
         if (!m_fields.size())
         {
            m_elemAlignment = field->get_elem_alignment();
         }

         //
         // don't forget to add the padding!!!
         //
         DDS::ULong alignment = field->get_elem_alignment();
         DDS::ULong osAlignment = field->get_OS_elem_alignment();
         if ((alignment == osAlignment) || ( m_elemSize % alignment == 0))
         {
            DDS::ULong fieldPad = (alignment) ?
               (alignment - m_elemSize % alignment) % alignment :
               4;
            DDS::ULong maxAlign = alignment;
            be_structure * structField;
            if ((structField = be_structure::_narrow( af->field_type() )))
            {
               maxAlign = structField->get_max_elem_alignment();
            }

            m_maxElemAlignment = ( m_maxElemAlignment > maxAlign )
               ? m_maxElemAlignment
               : maxAlign;


            be_array * t_array = 0;

            be_Type* type = field->get_be_type();

            m_interface_dependant |= type->IsInterfaceDependant ();

            be_typedef* pTypedef;

            //
            // dealias typedefs
            //
            do
            {
               pTypedef = (be_typedef*) (type->narrow((long) & be_typedef::type_id));

               if (pTypedef != 0)
               {
                  type = pTypedef->get_base_be_type();
               }
            }
            while (pTypedef != 0);

            DDS::ULong fieldSize = field->get_elem_size();
            if (fieldSize == 0)
            {
               m_elemSize = 0;
            }
            else if (m_elemSize > 0 || (m_elemSize == 0 && !m_fields.size()))
            {
               m_elemSize += fieldPad;

               //
               // check for array
               //
               if ((t_array = (be_array*)(type->narrow((long) & be_array::type_id))))
               {
                  m_elemSize += fieldSize * t_array->MatrixSize();
               }
               else
               {
                  m_elemSize += fieldSize;
               }
            }

            //
            // check to see if we can optimize streaming
            //
            if (!m_fields.size())
            {
               // first time through, set to true
               m_canOptimize = TRUE;
            }

            if (!type->IsOptimizable())
            {
               m_canOptimize = FALSE;
            }
         }
         else
         {
            m_elemSize = 0;
         }

         // set m_lastFieldSize to new value;
         m_lastFieldSize = field->get_elem_size();

         //
         // add to our own field list
         //
         m_fields.push_back(field);

         //
         // add to the TypeCode
         //
         m_typecode->members.push_back(field->get_be_type()->m_typecode);

         m_typecode->member_names.push_back(field->get_local_name());
      }

      return af;
   }

   return 0;
}

void be_structure::GenerateAuxTypes (be_ClientHeader& source)
{
   if (BE_Globals::isocpp_new_types)
     return;

   DDS_StdString varName = LocalName () + "_var";
   DDS_StdString outName = LocalName () + "_out";
   ostream & os = source.Stream ();
   be_Tab tab (source);

   if (m_isFixedLength)
   {
      os << tab << "typedef DDS_DCPSStruct_var < "
      << LocalName() << "> " << varName << ";" << nl;
      os << tab << "typedef " << LocalName ()
         << "&" << outName << ";" << nl;
   }
   else
   {
      os << tab << "typedef DDS_DCPSStruct_var < "
      << LocalName() << "> " << varName << ";" << nl;
      os << tab << "typedef DDS_DCPSStruct_out < "
      << LocalName() << "> " << outName << ";" << nl;
   }
}

void
be_structure::GenerateDefaultConstructor(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << tab << LocalName() << "()" << nl;
   os << tab << "{" << nl << tab << "}" << nl;
}

void
be_structure::GenerateMemcpyCopyConstructor(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << tab << LocalName() << "(const "
      << LocalName() << "& that)" << nl;
   os << tab << "{" << nl << tab.indent()
      << tab << "memcpy(this, &that, sizeof(" << LocalName() << "));"
      << nl << tab.outdent()
      << tab << "}" << nl;
}

void be_structure::Generate (be_ClientImplementation & source)
{
   UTL_ScopeActiveIterator * i;
   AST_Decl * d;
   UTL_Scope * s = (UTL_Scope*)narrow((long) & UTL_Scope::type_id);

   if (BE_Globals::ignore_interfaces && IsInterfaceDependant ())
   {
      return;
   }

   if (s)
   {
      i = new UTL_ScopeActiveIterator(s, UTL_Scope::IK_localtypes);

      while (!(i->is_done()))
      {
         be_CodeGenerator * cg;

         d = i->item();

         if (!d->imported() &&
               (cg = (be_CodeGenerator*)d->narrow((long) & be_CodeGenerator::type_id)))
         {
            cg->Generate (source);
         }

         i->next();
      }

      delete i;
   }
   else
   {
      assert(pbfalse);
   }
}

void be_structure::Generate (be_ClientHeader& source)
{
   if (BE_Globals::ignore_interfaces && IsInterfaceDependant ())
   {
      return;
   }

   if (!Generated())
   {
      be_root::AddAnyOps(*this);
      be_root::AddPutGetOps(*this);
      be_root::AddStreamOps(*this);
      be_root::AddTypedef(*this);
      be_root::AddTypecode(*this);

      ostream & os = source.Stream();
      DDS_StdString scopedName = ScopedName();
      DDS_StdString relativeName;
      be_Tab tab(source);
      TList<be_field *>::iterator mit;
      be_field * field;

      Generated (pbtrue);

      be_root::GenerateDependants(source, SequenceMemberTypeName(),
                                  EnclosingScope());

      g_cppScopeStack.Push(m_cppType);

      // struct definition

      os << nl;
      os << tab << (BE_Globals::isocpp_new_types ? "class " : "struct ") << DLLMACRO << LocalName ()
                <<  (BE_Globals::isocpp_new_types ? " OSPL_DDS_FINAL" : "") << nl;
      os << tab << "{" << nl;
      if (BE_Globals::isocpp_new_types)
        os << tab << "public:" << nl;

      source.Indent();

      // declare nested types

      source.Indent();
      be_CodeGenerator::Generate(source);

      // member accessor functions for isocpp new types
      if (BE_Globals::isocpp_new_types)
      {
        /** @internal
         * @todo OSPL-3369 Repetition; the sort of code people go to hell
         * for (rightfully); is_sequency is already evideantally a stupid name; &c... */
        // Constructors
        if (m_fields.size() > 0)
        {
          os << tab << LocalName() << "() {}" << nl;
          os << tab << "explicit " << LocalName() << "(" << nl;
          for (mit = m_fields.begin(); mit != m_fields.end(); mit++)
          {
            field = *mit;
            bool is_sequency = !(field->get_be_type()->IsPrimitiveType()
                                  || field->get_be_type()->IsEnumeratedType());
            relativeName = BE_Globals::RelativeScope
                (scopedName, field->StructMemberTypeName ());
            TList<be_field *>::iterator final_field = m_fields.end();
            --final_field;
            os << tab << tab << (is_sequency ? "const " : "") << relativeName
                << (is_sequency ? "& " : " ") << field->get_local_name();
            if (mit == final_field)
              os << ")";
            else
              os <<"," << nl;
          }
          os << tab << ":" << nl;
          for (mit = m_fields.begin(); mit != m_fields.end(); mit++)
          {
            field = *mit;
            TList<be_field *>::iterator final_field = m_fields.end();
            --final_field;
            os << tab << tab << tab << field->get_local_name() << "_(" << field->get_local_name() << ")" << (mit == final_field ? " {}" : ",") << nl;
          }
        }
        // C++ 11 move constructor, copy consructor, and assignement ops
        source.Outdent();
        os << "#ifdef OSPL_DDS_CXX11" << nl;
        os << "#  ifdef OSPL_CXX11_NO_FUNCTION_DEFAULTS" << nl;
        source.Indent();
        os << tab << LocalName() << "(const " << LocalName() << "& _other)" << nl;
        if (m_fields.size() > 0)
        {
          os << tab << ":" << nl;
          for (mit = m_fields.begin(); mit != m_fields.end(); mit++)
          {
            field = *mit;
            TList<be_field *>::iterator final_field = m_fields.end();
            --final_field;
            os << tab << tab << tab << field->get_local_name() << "_(_other." << field->get_local_name() << "_)" << (mit == final_field ? "" : ",") << nl;
          }
        }
        os << tab << "{}" << nl;
        os << tab << LocalName() << "(" << LocalName() << "&& _other)" << nl;
        if (m_fields.size() > 0)
        {
          os << tab << ":" << nl;
          for (mit = m_fields.begin(); mit != m_fields.end(); mit++)
          {
            field = *mit;
            TList<be_field *>::iterator final_field = m_fields.end();
            --final_field;
            os << tab << tab << tab << field->get_local_name() << "_(::std::move(_other." << field->get_local_name() << "_))" << (mit == final_field ? "" : ",") << nl;
          }
        }
        os << tab << "{}" << nl;
        os << tab <<  LocalName() << "& operator=(" << LocalName() << "&& _other)" << nl;
        os << tab << "{" << nl ;
        if (m_fields.size() > 0)
        {
          os << tab << tab << "if (this != &_other)" << nl;
          os << tab << tab << "{" << nl;
          for (mit = m_fields.begin(); mit != m_fields.end(); mit++)
          {
            field = *mit;
            TList<be_field *>::iterator final_field = m_fields.end();
            --final_field;
            os << tab << tab << tab << field->get_local_name() << "_ = ::std::move(_other." << field->get_local_name() << "_);" << nl;
          }
          os << tab << tab << "}" << nl;
        }
        os << tab << tab << "return *this;" << nl;
        os << tab << "}" << nl;
        os << tab <<  LocalName() << "& operator=(const "  << LocalName() << "& _other)" << nl;
        os << tab << "{" << nl ;
        if (m_fields.size() > 0)
        {
          os << tab << tab << "if (this != &_other)" << nl;
          os << tab << tab << "{" << nl;
          for (mit = m_fields.begin(); mit != m_fields.end(); mit++)
          {
            field = *mit;
            TList<be_field *>::iterator final_field = m_fields.end();
            --final_field;
            os << tab << tab << tab << field->get_local_name() << "_ = _other." << field->get_local_name() << "_;" << nl;
          }
          os << tab << tab << "}" << nl;
        }
        os << tab << tab << "return *this;" << nl;
        os << tab << "}" << nl;
        source.Outdent();
        os << "#  else" << nl;
        source.Indent();
        os << tab << LocalName() << "(const " << LocalName() << "& _other) = default;" << nl;
        os << tab << LocalName() << "(" << LocalName() << "&& _other) = default;" << nl;
        os << tab <<  LocalName() << "& operator=(" << LocalName() << "&& _other) = default;" << nl;
        os << tab <<  LocalName() << "& operator=(const "  << LocalName() << "& _other) = default;" << nl;
        source.Outdent();
        os << "#  endif" << nl;
        os << "#endif" << nl;
        source.Indent();
        for (mit = m_fields.begin(); mit != m_fields.end(); mit++)
        {
          field = *mit;
          bool is_sequency = !(field->get_be_type()->IsPrimitiveType()
                                || field->get_be_type()->IsEnumeratedType());
          relativeName = BE_Globals::RelativeScope
              (scopedName, field->StructMemberTypeName ());
          // const get accessor
          os << tab << (is_sequency ? "const " : "") << relativeName << (is_sequency ? "& " : " ")
              << field->get_local_name() << "() const { return this->" << field->get_local_name() << "_; }" << nl;
          // reference get accessor
          os << tab << relativeName << "& "
              << field->get_local_name() << "() { return this->" << field->get_local_name() << "_; }" << nl;
          // const set accessor
          os << tab << "void "
              << field->get_local_name() << (is_sequency ? "(const " : "(") << relativeName
              << (is_sequency ? "&" : "") << " _val_) { this->" << field->get_local_name() << "_ = _val_; }" << nl;
          source.Outdent();
          os << "#ifdef OSPL_DDS_CXX11" << nl;
          source.Indent();
          // C++ 11 move assignement op
          os << tab << "void "
              << field->get_local_name() <<  "(" << relativeName << "&& _val_) { this->" << field->get_local_name() << "_ = _val_; }" << nl;
          source.Outdent();
          os << "#endif" << nl;
          source.Indent();
        }
      }

      if (BE_Globals::gen_equality)
      {
          os << tab << "bool operator==(const " << LocalName() << "& _other) const" <<
            nl << tab << "{" << nl << tab << tab << "return ";
          DDS_StdString relName;
          for(mit = m_fields.begin(); mit != m_fields.end(); mit++)
          {
            field = *mit;
            relName = BE_Globals::RelativeScope
            (scopedName, field->get_local_name());
            TList<be_field *>::iterator final_field = m_fields.end();
            --final_field;

            if(mit != m_fields.begin())
                os << tab << tab;

            os  << relName << (BE_Globals::isocpp_new_types ? "_" : "")
                << " == _other."
                << relName << (BE_Globals::isocpp_new_types ? "_" : "")
                << (mit != final_field ? " &&" : ";") << nl;

          }
          os << tab << "}" << nl;

          os << tab << "bool operator!=(const " << LocalName() << "& other) const" <<
                nl << tab << "{" << nl << tab << tab << "return !(*this == other);"
                << nl << tab << "}" << nl;

      }

      // member declarations
      for (mit = m_fields.begin(); mit != m_fields.end(); mit++)
      {
         field = *mit;
         relativeName = BE_Globals::RelativeScope
            (scopedName, field->StructMemberTypeName ());
         os << tab << relativeName << " "
            << field->get_local_name() << (BE_Globals::isocpp_new_types ? "_" : "") << ";" << nl;
      }

      source.Outdent();
      source.Outdent();

      source.Stream() << tab << "};" << nl << nl;
      g_cppScopeStack.Pop();



      if (BE_Globals::isocpp_test_methods)
      {
          //Get base filename and append _testmethod.h
          DDS_StdString BaseFilename;
          BaseFilename = StripExtension(source.Filename());
          BaseFilename += "_testmethod.h";

          //Open or append to file
          be_Source testsource;
          ostream & ts = testsource.Stream();
          if(!testsource.Open(BaseFilename))
              cerr << "Cannot open: " << BaseFilename << endl;

          testsource.Indent();
          ts <<  "namespace {" << nl
                              <<  "template <>" << nl <<  "::std::vector< ::"
                              << ScopedName() <<  " > generate_test_values< ::"
                              << ScopedName() << " >()"  << nl
                              << "{" << nl;

          ts << tab << "::std::vector< ::" << ScopedName() << " > values;" << nl
                              << tab << "::" << ScopedName() << " next;" << nl
                              << tab << "::std::size_t biggest = 0;" << nl;

          for (mit = m_fields.begin(); mit != m_fields.end(); mit++)
          {
             field = *mit;
             relativeName = field->StructMemberTypeName ();

             ts << tab <<"::std::vector< " << field->StructMemberTypeName() << " > " << field->get_local_name() << "_ = generate_test_values< "
                                 << field->StructMemberTypeName() << " >();" << nl;
             ts << tab << "if(" << field->get_local_name() << "_.size() > biggest)"
                                 << nl << tab << tab << "biggest = " << field->get_local_name() << "_.size();"
                                 << nl;
          }
          ts << tab << "for(::std::size_t i = 0; i < biggest; ++i)" << nl << tab << "{" << nl;
          for (mit = m_fields.begin(); mit != m_fields.end(); mit++)
          {
             field = *mit;
             relativeName = BE_Globals::RelativeScope(scopedName, field->StructMemberTypeName ());

             ts << tab << tab << "next." << field->get_local_name() << "_ = " << field->get_local_name()
                << "_[i < " << field->get_local_name() << "_.size()? i : " << field->get_local_name()
                << "_.size() -1];" << nl;
          }
          ts << tab << tab << "values.push_back(next);" << nl
             << tab << "}" << nl
             << tab << "return values;" << nl;
          ts << "} }" << nl << nl;
          testsource.Outdent();
          testsource.Close();
      }

      GenerateAuxTypes (source);

      be_root::GenerateDependants(source, SequenceMemberTypeName(),
                                  EnclosingScope());
   }
}

be_structure *
be_structure::_narrow(AST_Type * atype)
{
   be_structure * ret = 0;

   if (atype)
   {
      ret = (be_structure*)atype->narrow((long) & be_structure::type_id);
   }

   return ret;
}

void be_structure::put_core_fields
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   FieldList & coreFields,
   unsigned long uid
)
{
   DDS::Boolean wroteOne = FALSE;
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString _in_ = (DDS_StdString)"_in_" + uids;
   FieldList::iterator it;

   // first, let's declare the core args for putting

   for (it = coreFields.begin(); it != coreFields.end(); it++)
   {
      (*it)->declare_for_struct_put (os, tab, sptr, uid);
   }

   //
   // now, let's initilize our put args
   //
   os << tab << "DDS::Codec::Param " << _in_ << "[" << coreFields.size() << "] =" << nl;
   os << tab << "{" << nl;

   tab.indent ();
   for (it = coreFields.begin(); it != coreFields.end(); it++)
   {
      if (wroteOne)
      {
         os << "," << nl;
      }

      (*it)->make_put_param (os, tab, sptr, uid);
      wroteOne = TRUE;
   }
   tab.outdent ();

   os << nl << tab << "};" << nl;

   // and finally, let's put 'em

   os << tab << "os.put (" << _in_ << ", " << coreFields.size ()
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
}

void be_structure::get_core_fields
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   FieldList & coreFields,
   unsigned long uid
)
{
   DDS::Boolean wroteOne = FALSE;
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString _out_ = (DDS_StdString)"_out_" + uids;
   FieldList::iterator it;

   // first, let's declare the core args for getting

   for (it = coreFields.begin(); it != coreFields.end(); it++)
   {
      (*it)->declare_for_struct_get (os, tab, sptr, uid);
   }

   // now, let's initilize our get args

   os << tab << "DDS::Codec::Param " << _out_ << "[" << coreFields.size() << "] =" << nl;
   os << tab << "{" << nl;

   tab.indent ();
   for (it = coreFields.begin(); it != coreFields.end(); it++)
   {
      if (wroteOne)
      {
         os << "," << nl;
      }

      (*it)->make_get_param (os, tab, sptr, uid);
      wroteOne = TRUE;
   }
   tab.outdent ();

   os << nl << tab << "};" << nl;

   // and finally, let's get 'em

   os << tab << "is.get (" << _out_ << ", " << coreFields.size ()
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
}

void be_structure::putter
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   unsigned long uid
)
{
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);

   if (m_marshalInCore)
   {
      put_core_fields (os, tab, sptr, m_fields, uid);
   }
   else
   {
      FieldList coreFields;
      FieldList::iterator it;

      for (it = m_fields.begin(); it != m_fields.end(); it++)
      {
         if ((*it)->is_core_marshaled())
         {
            coreFields.push_back(*it);
         }
         else
         {
            if (coreFields.size())
            {
               put_core_fields (os, tab, sptr, coreFields, uid++);
               coreFields.erase ();
            }

            (*it)->put_for_struct(os, tab, sptr, uid++);
         }
      }

      if (coreFields.size())
      {
         put_core_fields(os, tab, sptr, coreFields, uid++);
         coreFields.erase();
      }
   }
}


void be_structure::getter
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   unsigned long uid
)
{
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);

   if (m_marshalInCore)
   {
      get_core_fields(os, tab, sptr, m_fields, uid);
   }
   else
   {
      FieldList coreFields;
      FieldList::iterator it;

      for (it = m_fields.begin(); it != m_fields.end(); it++)
      {
         if ((*it)->is_core_marshaled())
         {
            coreFields.push_back(*it);
         }
         else
         {
            if (coreFields.size())
            {
               get_core_fields(os, tab, sptr, coreFields, uid++);
               coreFields.erase();
            }

            (*it)->get_for_struct(os, tab, sptr, uid++);
         }
      }

      if (coreFields.size())
      {
         get_core_fields(os, tab, sptr, coreFields, uid++);
         coreFields.erase();
      }
   }
}


void
be_structure::generate_tc_ctor_val(
   be_Source & source)
{
   be_Type::generate_tc_ctor_val(source);
}


void be_structure::generate_tc_dtor_val
(
   be_Source & source,
   pbbool isCounted
)
{
   be_Type::generate_tc_dtor_val (source, FALSE);
}


void be_structure::generate_tc_put_val (be_Source & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   unsigned long uid = 0;

   // declare writer body

   os << "void " << m_tc_put_val << nl;
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

   os << tab << ScopedName () << " * p = (" << ScopedName () << "*) arg;" << nl;

   // now, let's put our fields

   putter (os, tab, "p", uid);

   tab.outdent ();
   os << tab << "}" << nl << nl;
}

void be_structure::generate_tc_get_val (be_Source & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   unsigned long uid = 0;

   // declare reader body

   os << "void " << m_tc_get_val << nl;
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

   os << tab << ScopedName () << " * p = (" << ScopedName () << "*) arg;" << nl;

   // now, let's get our fields

   getter (os, tab, "p", uid);

   tab.outdent ();
   os << tab << "}" << nl << nl;
}

void be_structure::generate_tc_assign_val (be_Source & source)
{
   be_Type::generate_tc_assign_val (source);
}

DDS_StdString
be_structure::UnionStreamIn(const DDS_StdString& arg, const DDS_StdString& in) const
{
   if (m_isFixedLength)
   {
      return be_Type::UnionStreamIn(arg, in);
   }
   else
   {
      DDS_StdString ret = in + " >> (" + localName + "*&) " + arg + ";";
      return ret;
   }
}

DDS_StdString
be_structure::UnionStreamOut(const DDS_StdString& arg, const DDS_StdString& out) const
{
   if (m_isFixedLength)
   {
      return be_Type::UnionStreamOut(arg, out);
   }
   else
   {
      DDS_StdString ret = out + " << (" + localName + "*) " + arg + ";";
      return ret;
   }
}

DDS_StdString
be_structure::kind_string()
{
   return "DDS::tk_struct";
}

void
be_structure::is_fixed_length(DDS::Boolean val)
{
   m_isFixedLength = val;
}

DDS::ULong
be_structure::get_elem_size()
{
   //
   // If the size of the struct is not divisible by the size of the largest
   // primitive in the struct, there will be more padding in the in-memory copy
   // than that required by GIOP. Therefore, the size returned is 0 so that
   // the fields will be marshalled individually rather than memcpy'd.
   //

   return ( m_elemSize % m_maxElemAlignment ? 0 : m_elemSize );
}

DDS::ULong
be_structure::get_elem_alignment()
{
   //
   // return the alignment of the first member
   //
   return m_elemAlignment;
}

DDS::ULong
be_structure::get_max_elem_alignment()
{
   //
   // return the largest alignment of the members.
   //
   return m_maxElemAlignment;
}

DDS::Boolean be_structure::declare_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & stubScope,
   VarType vt
)
{
   DDS::Boolean ret = FALSE;
   DDS_StdString relTypeName = BE_Globals::RelativeScope (stubScope, typeName);

   if (m_isFixedLength)
   {
      if (vt == VT_Return)
      {
         os << tab << relTypeName << " " << arg << ";" << nl;
      }
   }
   else
   {
      switch (vt)
      {
         case VT_InParam:
         case VT_InOutParam:
         break;

         case VT_OutParam:
         {
            os << tab << arg << " = new " << relTypeName << ";" << nl;
         }
         break;

         case VT_Return:
         {
            os << tab << "DDS_DCPSStruct_var < " << relTypeName << "> " << arg
               << " (new " << relTypeName << ");" << nl;
         }
         break;

         default:   // VT_OutParam
         ret = FALSE;
      }
   }

   return ret;
}

DDS::Boolean
be_structure::declare_for_struct_put(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return FALSE;
}

DDS::Boolean
be_structure::declare_for_struct_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return FALSE;
}

DDS::Boolean
be_structure::declare_for_union_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   if (!m_isFixedLength)
   {
      os << tab << sptr << "->" << fld << " = (void*) new "
      << ScopedName() << ";" << nl;
   }

   return FALSE;
}

DDS::Boolean be_structure::make_get_param_for_stub
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
      os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";

      if (m_isFixedLength)
      {
         switch (vt)
         {
            case VT_Return:
            case VT_InOutParam:
            case VT_OutParam:
            os << "&" << arg << ", DDS::PARAM_OUT ";
            default:
            break;
         }
      }
      else
      {
         switch (vt)
         {
            case VT_InOutParam:
            os << "&" << arg << ", DDS::PARAM_OUT ";
            break;

            case VT_OutParam:
            os << arg << ".out(), DDS::PARAM_OUT ";
            break;

            case VT_Return:
            os << arg << ", DDS::PARAM_OUT ";
            break;

            default:
               break;
         }
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_structure::make_put_param_for_stub
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
      os << tab << "{ " << Scope (TypeCodeTypeName()) << ", ";

      switch (vt)
      {
         case VT_InParam:
         os << "(" << TypeName() << "*)" << "&" << arg << ", DDS::PARAM_IN ";
         break;

         case VT_InOutParam:
         os << "&" << arg << ", DDS::PARAM_IN ";
         break;

         default:
            break;
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_structure::make_put_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   if (m_isFixedLength)
   {
      os << tab << "{ " << Scope(TypeCodeTypeName())
      << ", &" << sptr << "->" << fld << ", DDS::PARAM_IN }";
   }
   else
   {
      os << tab << "{ " << Scope(TypeCodeTypeName())
      << ", " << sptr << "->" << fld << ", DDS::PARAM_IN }";
   }

   return TRUE;
}

DDS::Boolean be_structure::make_get_param_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   if (m_isFixedLength)
   {
      os << tab << "{ " << Scope(TypeCodeTypeName())
      << ", &" << sptr << "->" << fld << ", DDS::PARAM_OUT }";
   }
   else
   {
      os << tab << "{ " << Scope(TypeCodeTypeName())
      << ", " << sptr << "->" << fld << ", DDS::PARAM_OUT }";
   }

   return TRUE;
}

DDS::Boolean be_structure::make_put_param_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
#if defined(OPTIMIZE)
   os << tab << m_tc_put_val << "(os, (void*)&__" << fld
   << ");" << nl;
#else

   os << tab << "{ " << Scope(TypeCodeTypeName())
   << ", &" << sptr << "->" << fld << ", DDS::PARAM_IN }";
#endif

   return TRUE;
}

DDS::Boolean be_structure::make_get_param_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
#if defined(OPTIMIZE)
   os << tab << m_tc_get_val << "(is, (void*&)__" << fld
   << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
#else

   os << tab << "{ " << Scope(TypeCodeTypeName())
   << ", &" << sptr << "->" << fld << ", DDS::PARAM_OUT }";
#endif

   return TRUE;
}

ostream & be_structure::put_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << m_tc_put_val << "(os, (void*)&"
      << sptr << "->" << fld << ", DDS::PARAM_IN"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_structure::get_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   //
   // first declare the field
   //
   os << tab << ScopedName() << " * ___" << fld << " = &("
   << sptr << "->" << fld << ");" << nl;

   //
   // now get it
   //
   tab.indent();
   os << tab << m_tc_get_val << "(is, (void*&)___"
   << fld << ", DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent();

   return os;
}

ostream &
be_structure::put_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   if (m_isFixedLength)
   {
      //
      // just put it
      //
      os << tab << m_tc_put_val << "(os, (void*)&"
         << sptr << "->" << fld << ", DDS::PARAM_IN"
         << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   }
   else
   {
      //
      // first declare the field
      //
      os << tab << ScopedName() << " * ___" << fld
      << " = (" << ScopedName() << "*) &(" << sptr << "->" << fld << ");"
      << nl << nl;

      //
      // now put it
      //
      os << tab << m_tc_put_val << "(os, (void*)&___"
         << fld << ", DDS::PARAM_IN"
         << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   }

   return os;
}

ostream &
be_structure::get_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   //
   // first declare the field
   //

   os << tab << ScopedName() << " * ___" << fld
   << " = (" << ScopedName() << "*) &(" << sptr << "->" << fld << ");"
   << nl << nl;
   //
   // now get it
   //
   return os << tab << m_tc_get_val << "(is, (void*&)___" << fld
          << ", DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
}

ostream & be_structure::put_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString sptr = (DDS_StdString)"sptr" + uids;

   os << tab << ScopedName() << " * "
      << sptr << " = &" << arg << "[" << index
      << "];" << nl;

   os << tab << m_tc_put_val << " (os, " << sptr << ", DDS::PARAM_IN"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream &
be_structure::get_for_sequence(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString sptr = (DDS_StdString)"sptr" + uids;

   os << tab << ScopedName() << " * "
      << sptr << " = &" << arg << "[" << index
      << "];" << nl;

   os << tab << m_tc_get_val << " (is, (void*&)" << sptr
      << ", DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream &
be_structure::put_for_array(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString sptr = (DDS_StdString)"sptr" + uids;

   os << tab << ScopedName() << " * "
      << sptr << " = (" << ScopedName() << "*)&"
      << arg << "[" << index << "];" << nl;

   os << tab << m_tc_put_val << " (os, " << sptr << ", DDS::PARAM_IN"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_structure::get_for_array
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString sptr = (DDS_StdString)"sptr" + uids;

   os << tab << ScopedName() << " * "
      << sptr << " = (" << ScopedName() << "*)&"
      << arg << "[" << index << "];" << nl;

   os << tab << m_tc_get_val << " (is, (void*&)" << sptr
      << ", DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

DDS::Boolean be_structure::is_core_marshaled ()
{
   return FALSE;
}

be_DispatchableType::en_HowStoredInDispatcher be_structure::HowStoredInDispatcher
   (const be_ArgumentDirection& direction) const
{
   if (! IsFixedLength ())
   {
      // variable-length structure

      switch (direction)
      {
         case VT_InParam:
         case VT_InOutParam:
            return STORED_AS_STACK_VARIABLE;

         case VT_OutParam:
         case VT_Return:
            return STORED_IN_VAR;

         default: assert (0);
      }
   }
   return STORED_AS_STACK_VARIABLE;
}
