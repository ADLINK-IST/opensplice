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
#include "xbe_exception.h"
#include "xbe_field.h"
#include "xbe_root.h"
#include "xbe_array.h"
#include "xbe_typedef.h"
#include "xbe_utils.h"

#if defined(_WIN32)

#pragma warning( disable : 4101 )

#endif

// -------------------------------------------------
//  BE_EXCEPTION IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS3(be_exception, AST_Exception, be_ClassGenerator, be_Type)
IMPL_NARROW_FROM_DECL(be_exception)
IMPL_NARROW_FROM_SCOPE(be_exception)

be_exception::be_exception ()
{
   isAtModuleScope (pbfalse);
}

be_exception::be_exception (UTL_ScopedName *n, const UTL_Pragmas &p)
:
   AST_Decl (AST_Decl::NT_except, n, p),
   UTL_Scope (AST_Decl::NT_except, n, p),
   AST_Structure (AST_Decl::NT_except, n, p),
   m_isFixedLength (TRUE),
   m_elemAlignment (0),
   m_elemSize (0),
   m_marshalInCore (false)
{
   isAtModuleScope (pbfalse);
   const DDS_StdString userException = "DDS::UserException";
   DDS_StdString barScopedName = NameToString(name(), "_");

   localName = local_name()->get_string();
   SetName(localName);
   enclosingScope = be_Type::EnclosingScopeString(this);

   m_tc_ctor_val = (DDS_StdString) barScopedName + "_ctor";
   m_tc_dtor_val = (DDS_StdString) barScopedName + "_dtor";
   m_tc_put_val = "(DDS::TypeCode::PutFunc) 0";
   m_tc_get_val = "(DDS::TypeCode::GetFunc) 0";
   m_tc_assign_val = (DDS_StdString) barScopedName +
      "_copy";

   SetScopedClassName(ScopedName());
   AddParent (new be_ClassParent (userException, userException, true, false));

   m_typecode->kind = DDS::tk_except;
   m_typecode->id = get_decl_pragmas().get_repositoryID()->get_string();
   m_typecode->name_of_type = localName;

   InitializeTypeMap(this);
}

DDS::Boolean
be_exception::IsFixedLength() const
{
   return m_isFixedLength;
}

DDS::Boolean
be_exception::IsFixedLengthPrimitiveType() const
{
   return false;
}


void
be_exception::InitializeTypeMap(be_Type* t)
{
   idlType = t;

   if (t)
   {
      AST_Type * t_ast = (AST_Type*)t->narrow((long) & AST_Type::type_id);

      assert(t_ast);

      t->TypeName(NameToString(t_ast->name()));
      t->InTypeName("const " + t->TypeName() + "&");
      t->InOutTypeName(t->TypeName() + "&");

      t->VarSignature(VT_InParam,
                      t->TypeName(),
                      VT_Const,
                      VT_Var,
                      VT_Reference);
      t->VarSignature(VT_InOutParam,
                      t->TypeName(),
                      VT_NonConst,
                      VT_Var,
                      VT_Reference);

      if (t->IsFixedLength())
      {
         t->OutTypeName(t->TypeName() + "&");
         t->ReturnTypeName(t->TypeName());

         VarSignature( VT_OutParam,
                       t->TypeName(),
                       VT_NonConst,
                       VT_Var,
                       VT_Reference);
         VarSignature(VT_Return,
                      t->TypeName(),
                      VT_NonConst,
                      VT_Var,
                      VT_NonReference);
      }
      else
      {
         t->OutTypeName(t->TypeName() + DDSOutExtension);
         t->ReturnTypeName(t->TypeName() + "*");

         t->VarSignature(VT_OutParam,
                         t->OutTypeName(),
                         VT_NonConst,
                         VT_Var,
                         VT_NonReference);
         t->VarSignature(VT_Return,
                         t->TypeName(),
                         VT_NonConst,
                         VT_Pointer,
                         VT_NonReference);
      }

      t->DMFAdtMemberTypeName(t->TypeName() + "*");
      t->StructMemberTypeName(t->TypeName());
      t->UnionMemberTypeName(t->TypeName());
      t->SequenceMemberTypeName(t->TypeName());

      t->TypeCodeTypeName(BE_Globals::TCPrefix + t->LocalName());

      DDS_StdString scopebar = NameToString(t_ast->name(), "_");
      t->TypeCodeBaseName(BE_Globals::TCBasePrefix + scopebar);
      t->TypeCodeRepName (BE_Globals::TCRepPrefix + scopebar);
      t->MetaTypeTypeName(BE_Globals::MTPrefix + scopebar);
   }
   else
   {
      assert(0);
   }
}

void
be_exception::GenerateMembers(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   pbbool first = pbtrue;
   be_Tab tab(source);

   UTL_Scope * s = (UTL_Scope*)narrow((long) & UTL_Scope::type_id);
   assert(s);

   // ITERATE THROUGH DECLS
   UTL_ScopeActiveIterator *it;

   for ( it = new UTL_ScopeActiveIterator(s, UTL_Scope::IK_decls);
         !it->is_done();
         it->next())
   {
      AST_Decl * adecl = it->item();

      assert(adecl);

      if (first)
      {
         os << nl << tab << "public: " << nl << nl;
         first = pbfalse;
      }

      be_field * bfield = be_field::_narrow(adecl);

      if (bfield)
      {
         os << tab;
         os << BE_Globals::RelativeScope(
            ScopedName(),
            bfield->StructMemberTypeName());
         os << " " << bfield->get_local_name() << ";" << nl;
      }
   }

   delete it;
}

void be_exception::GenerateConvenienceConstructor (be_ClientHeader& source)
{
   ostream & os = source.Stream ();
   const char * argPrefix = "_";
   pbbool first = pbtrue;
   be_Tab tab (source);

   os << tab << LocalName () << " (";

   UTL_Scope * s = (UTL_Scope*)narrow ((long) & UTL_Scope::type_id);
   assert (s);

   // Iterate through decls

   UTL_ScopeActiveIterator *it;

   for
   (
      it = new UTL_ScopeActiveIterator (s, UTL_Scope::IK_decls);
      !it->is_done ();
      it->next ()
   )
   {
      AST_Decl * adecl = it->item();
      assert (adecl);
      be_field * bfield = be_field::_narrow (adecl);
      be_Type * btype;

      if (bfield)
      {
         btype = bfield->get_be_type ();

         if (!first)
         {
            os << ", ";
         }

         first = pbfalse;

         if (btype && btype->IsStringType ())
         {
            // Strings are special case

            os << (char*) BE_Globals::RelativeScope (ScopedName (), bfield->InTypeName ());
         }
         else
         {
            os << (char*) BE_Globals::RelativeScope (ScopedName (), bfield->StructMemberTypeName ());
         }

         os << " " << argPrefix << (char*) bfield->get_local_name ();
      }
   }

   delete it;

   os << ");" << nl;
}

void be_exception::GenerateConvenienceConstructor (be_ClientImplementation & source)
{
   ostream & os = source.Stream ();
   const char * argPrefix = "_";
   pbbool first = pbtrue;
   be_Tab tab (source);
   be_Type * btype;

   os << ScopedName () << "::" << LocalName () << " (";

   UTL_Scope * s = (UTL_Scope*)narrow ((long) & UTL_Scope::type_id);
   assert (s);

   // Iterate through decls

   UTL_ScopeActiveIterator * it;

   for
   (
      it = new UTL_ScopeActiveIterator (s, UTL_Scope::IK_decls);
      !it->is_done ();
      it->next ()
   )
   {
      AST_Decl * adecl = it->item ();
      assert (adecl);

      be_field *bfield = (be_field *)adecl->narrow ((long) & be_field::type_id);

      if (bfield)
      {
         btype = bfield->get_be_type ();

         if (!first)
         {
            os << ", ";
         }

         first = pbfalse;

         if (btype && btype->IsStringType ())
         {
            // Strings are special case

            os << (char*) BE_Globals::RelativeScope (ScopedName (), bfield->InTypeName ());
         }
         else
         {
            os << (char*) BE_Globals::RelativeScope (ScopedName (), bfield->StructMemberTypeName ());
         }

         os << " " << argPrefix << (char*) bfield->get_local_name ();
      }
   }
   delete it;

   os << ")" << nl;

   // Iterate thru members initializing

   if (nmembers ())
   {
      source.Indent ();

      first = pbtrue;

      for
      (
         it = new UTL_ScopeActiveIterator (s, UTL_Scope::IK_decls);
         !it->is_done ();
         it->next ()
      )
      {
         AST_Decl * adecl = it->item ();
         assert (adecl);

         be_field *bfield = (be_field *)adecl->narrow ((long) & be_field::type_id);

         if (bfield)
         {
            btype = bfield->get_be_type ();

            // Array members are assigned in body of method

            if (btype && ! btype->IsArrayType ())
            {
               if (first)
               {
                  first = pbfalse;
                  os << "   : " << nl;
               }
               else
               {
                  os << "," << nl;
               }

               os << "   " << (char*) bfield->get_local_name () << "(" << argPrefix
               << (char*) bfield->get_local_name () << ")";
            }
         }
      }
      delete it;

      source.Outdent ();
   }

   os << nl;
   os << tab << "{" << nl;

   for 
   (
      it = new UTL_ScopeActiveIterator (s, UTL_Scope::IK_decls);
      ! it->is_done ();
      it->next ()
   )
   {
      AST_Decl * adecl = it->item ();
      assert (adecl);

      be_field *bfield = (be_field *) adecl->narrow ((long) & be_field::type_id);

      if (bfield)
      {
         btype = bfield->get_be_type ();
         if (btype && btype->IsArrayType ())
         {
            // Need to copy array elements

            os << "   "
               << (char*) BE_Globals::RelativeScope (ScopedName (), bfield->StructMemberTypeName ())
               << "_copy (" << argPrefix << bfield->get_local_name () 
               << ", " << bfield->get_local_name () << ");" << nl;
         }
      }
   }
   delete it;

   os << "}" << nl << nl;
}

void be_exception::GenerateCopyConstructor (be_ClientImplementation & source)
{
   ostream & os = source.Stream ();

   os << ScopedName () << "::" << LocalName ()
      << " (const " << LocalName () << " & that)" << nl
      << "   : DDS::UserException (that)" << nl
      << "{" << nl
      << "   *this = that;" << nl
      << "}" << nl << nl;
}

void be_exception::GenerateAssignmentOperator (be_ClientImplementation& source)
{
   ostream & os = source.Stream ();
   DDS_StdString that ("");
   be_Type * btype;

   if (nmembers ())
   {
      that = " that";
   }

   os << ScopedName () << " & "
      << ScopedName () << "::operator = (const "
      << LocalName () << " &" << that << ")" << nl;
   os << "{" << nl;

   UTL_Scope * s = (UTL_Scope*)narrow((long) & UTL_Scope::type_id);
   assert (s);

   UTL_ScopeActiveIterator *it;

   // Iterate through decls

   for 
   (
      it = new UTL_ScopeActiveIterator (s, UTL_Scope::IK_decls);
      ! it->is_done ();
      it->next ()
   )
   {
      AST_Decl * adecl = it->item ();
      assert (adecl);

      be_field *bfield = (be_field *) adecl->narrow ((long) & be_field::type_id);

      if (bfield)
      {
         btype = bfield->get_be_type ();
         if (btype && btype->IsArrayType ())
         {
            // Need to copy array elements

            os << "   "
               << (char*) BE_Globals::RelativeScope (ScopedName (), bfield->StructMemberTypeName ())
               << "_copy (" << bfield->get_local_name () 
               << ", that." << bfield->get_local_name () << ");" << nl;
         }
         else
         {
            os << "   " << bfield->get_local_name () << " = that."
               << bfield->get_local_name() << ";" << nl;
         }
      }
   }

   delete it;

   os << "   return *this;" << nl;
   os << "}" << nl << nl;
}

void
be_exception::Generate(be_ServerHeader&)
{}

void
be_exception::Generate(be_ServerImplementation&)
{}

AST_Field *
be_exception::add_field(AST_Field *af)
{
   if (AST_Structure::add_field(af))
   {
      be_field * field = be_field::_narrow (af);

      if (field)
      {
         field->initialize();

         if (IsFixedLength() && !field->IsFixedLength())
         {
            is_fixed_length (false);
            InitializeTypeMap(this);
         }

         //
         // note the alignment of the first field
         //
         if (!m_fields.size())
         {
            m_elemAlignment = field->get_elem_alignment();
         }

         //
         // don't forget to add the padding!!!
         //
         DDS::ULong fieldPad = (field->get_elem_size()) ?
                                 m_elemSize % field->get_elem_size() :
                                 4;

         //
         // now update the element size
         //
         m_elemSize += fieldPad + field->get_elem_size();

         //
         // determine if marshaling should be done in the core
         //
         if (field->is_core_marshaled())
         {
            m_marshalInCore = TRUE;
         }

         //
         // add field to our list for marshaling
         //
         m_fields.push_back(field);
      }

      //
      // add to typecode
      //
      m_typecode->members.push_back(field->get_be_type()->m_typecode);

      m_typecode->member_names.push_back(field->get_local_name());

      return af;
   }

   return 0;
}

void be_exception::Generate (be_ClientHeader & source)
{
   if (!Generated ())
   {
      ostream & os = source.Stream ();
      be_Tab tab (source);
      DDS_StdString lname = LocalName ();

      Generated (pbtrue);

      be_root::AddGlobalDeclarations (this);
      be_root::AddAnyOps (*this);
      be_root::AddStreamOps (*this);
      be_root::AddTypedef (*this);
      be_root::AddTypecode (*this);

      GenerateOpenClassDefinition (source);
      GenerateClassDeclarations (source);
      SetAccess (source, CA_PUBLIC);

      // now define nested types

      be_CodeGenerator::Generate (source);

      // generate _downcast

      os << tab << "static " << lname << "* _downcast ("
         << "DDS::Exception *);" << nl;
      os << tab << "static const " << lname << "* _downcast ("
         << "const DDS::Exception *);" << nl;

      // generate factory and builder

      os << tab << "static DDS::Exception * factory ();" << nl;
      os << tab << "static DDS::ExceptionInitializer m_initializer;" << nl << nl;

      // generate inline default constructor

      os << tab << lname << " () {};" << nl;

      // generate convenience constructor

      if (nmembers())
      {
         GenerateConvenienceConstructor (source);
      }

      // generate copy constructor

      os << tab << lname << " (const " << lname << " &);" << nl;

      // generate assignment operator

      os << tab << lname << "& operator = " << "(const " << lname << " &);" << nl;

      // generate duplicate

      os << tab << "virtual DDS::Exception * _clone () const;" << nl;

      // generate raise

      os << tab << "virtual void _raise (" << XBE_Ev::arg (XBE_ENV_ARG1)
         << ") const;" << nl;

      // generate name

      os << tab << "virtual const char * _name () const { return m_name; };" << nl;

      // generate repository id

      os << tab << "virtual const char * _rep_id () const { return m_id; };" << nl;

      // generate virtual destructor

      os << tab << "virtual ~" << lname << " () {}" << nl;

      GenerateMembers (source);
      SetAccess (source, CA_PRIVATE);

      os << tab << "static const char * m_name;" << nl;
      os << tab << "static const char * m_id;" << nl;

      GenerateCloseClassDefinition (source);

      be_root::GenerateDependants
      (
         source,
         SequenceMemberTypeName (),
         EnclosingScope ()
      );
   }
}

void be_exception::Generate (be_ClientImplementation& source)
{
   ostream & os = source.Stream();
   be_Tab tab (source);
   UTL_String * repID = get_decl_pragmas().get_repositoryID();
   const DDS_StdString corbaException = "DDS::Exception";

   // convenience constructor

   if (nmembers())
   {
      GenerateConvenienceConstructor(source);
   }

   // copy constructor

   GenerateCopyConstructor (source);

   // assignment operator

   GenerateAssignmentOperator (source);

   be_CodeGenerator::Generate (source);

   // name and id

   assert (repID);
   os << "const char * " << ScopedName () << "::m_name = \"" 
      << LocalName () << "\";" << nl << nl;
   os << "const char * " << ScopedName () << "::m_id = \"" 
      << repID->get_string () << "\";" << nl << nl;

   // downcast

   os << tab << ScopedName () << " * ";
   os << ScopedName () << "::";
   os << "_downcast (DDS::Exception * e)" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   os << tab << "if (e && (e->_rep_id () == m_id))" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   os << tab << "return static_cast < " << LocalName () << "*> (e);" << nl;
   tab.outdent ();
   os << tab << "}" << nl;
   os << tab << "return 0;" << nl;
   tab.outdent ();
   os << tab << "}" << nl << nl;

   // const downcast

   os << tab << "const " << ScopedName() << " * ";
   os << ScopedName () << "::";
   os << "_downcast (const DDS::Exception * e)" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   os << tab << "if (e && (e->_rep_id () == m_id))" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   os << tab << "return static_cast < " << LocalName () 
      << "*> ((" << LocalName () << "*) e);" << nl;
   tab.outdent ();
   os << tab << "}" << nl;
   os << tab << "return 0;" << nl;
   tab.outdent ();
   os << tab << "}" << nl << nl;

   // _clone

   os << tab << corbaException << " * " << ScopedName ();
   os << "::_clone () const" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   os << tab << "return (" << corbaException << "*) new ";
   os << ScopedName () << " (*this);" << nl;
   tab.outdent ();
   os << tab << "}" << nl << nl;

   // raise

   os << tab << "void " << ScopedName () << "::_raise ("
      << XBE_Ev::arg (XBE_ENV_ARG1) << ") const" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   os << tab;
   XBE_Ev::throwex (os, "*this");
   os << nl;
   tab.outdent ();
   os << tab << "}" << nl << nl;

   // factory

   os << tab << "DDS::Exception * " << ScopedName() << "::factory ()" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   os << tab << "return new " << ScopedName () << ";" << nl;
   tab.outdent ();
   os << tab << "}" << nl << nl;

   // initilaizer

   os << "DDS::ExceptionInitializer " << ScopedName() << "::m_initializer (";
   os << "\"" << repID->get_string () << "\"";
   os << ", " << ScopedName () << "::factory);" << nl;
}

void
be_exception::GenerateTypedefs( const DDS_StdString & scope,
                                const be_typedef & alias,
                                be_ClientHeader & source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   DDS_StdString relTypeName = BE_Globals::RelativeScope(scope, typeName);

   os << tab << "typedef " << (char*) relTypeName << " "
   << alias.LocalName() << ";" << nl;
   os << tab << "typedef " << (char*) relTypeName << " "
   << alias.LocalName() << ";" << nl;
}

void be_exception::GenerateGlobalDecls (be_ClientHeader& source)
{}

DDS::Boolean
be_exception::is_core_marshaled()
{
   return false;
}

void be_exception::put_core_fields
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   FieldList & coreFields,
   unsigned long uid
)
{
   DDS::Boolean wroteOne = false;
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString _in_ = (DDS_StdString)"_in_" + uids;
   FieldList::iterator it;

   // first, let's declare the core args for putting

   for (it = coreFields.begin(); it != coreFields.end(); it++)
   {
      (*it)->declare_for_struct_put(os, tab, sptr, uid);
   }

   // now, let's initilize our put args

   os << tab << "DDS::Codec::Param " << _in_ << "[" << coreFields.size () << "] =" << nl;
   os << tab << "{" << nl;

   tab.indent ();
   for (it = coreFields.begin(); it != coreFields.end(); it++)
   {
      if (wroteOne)
      {
         os << "," << nl;
      }

      (*it)->make_put_param(os, tab, sptr, uid);
      wroteOne = TRUE;
   }
   tab.outdent ();

   os << nl << tab << "};" << nl;

   // and finally, let's put 'em

   os << tab << "os.put (" << _in_ << "," << coreFields.size ();
   os << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
}

void be_exception::get_core_fields
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   FieldList & coreFields,
   unsigned long uid
)
{
   DDS::Boolean wroteOne = false;
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString _out_ = (DDS_StdString)"_out_" + uids;
   FieldList::iterator it;

   // first, let's declare the core args for getting

   for (it = coreFields.begin(); it != coreFields.end(); it++)
   {
      (*it)->declare_for_struct_get(os, tab, sptr, uid);
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

      (*it)->make_get_param(os, tab, sptr, uid);
      wroteOne = TRUE;
   }
   tab.outdent ();

   os << nl << tab << "};" << nl;

   // and finally, let's get 'em

   os << tab << "is.get (" << _out_ << "," << coreFields.size ();
   os << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
}

void be_exception::generate_tc_ctor_val (be_Source & source)
{
   be_Type::generate_tc_ctor_val (source);
}

void be_exception::generate_tc_dtor_val
(
   be_Source & source,
   pbbool isCounted
)
{
   be_Type::generate_tc_dtor_val (source, false);
}


void be_exception::putter
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
      put_core_fields(os, tab, sptr, m_fields, uid);
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
               put_core_fields(os, tab, sptr, coreFields, uid++);
               coreFields.erase();
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

void
be_exception::generate_tc_put_val(be_Source & source)
{}


void be_exception::getter
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
be_exception::generate_tc_get_val(be_Source & source)
{}


void
be_exception::generate_tc_assign_val(
   be_Source & source)
{
   be_Type::generate_tc_assign_val(source);
}

DDS::Boolean be_exception::declare_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & stubScope,
   VarType vt
)
{
   assert (FALSE); // not supported in IDL
   return FALSE;
}

DDS::Boolean
be_exception::declare_for_struct_put(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return FALSE;
}

DDS::Boolean
be_exception::declare_for_struct_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return FALSE;
}

DDS::Boolean
be_exception::make_get_param_for_stub(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & argname,
   VarType vt) const
{
   assert(FALSE); // not supported in IDL
   return FALSE;
}

DDS::Boolean
be_exception::make_put_param_for_stub(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & argname,
   VarType vt) const
{
   assert(FALSE); // not supported in IDL
   return FALSE;
}

DDS::Boolean
be_exception::make_put_param_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return FALSE;
}

DDS::Boolean
be_exception::make_get_param_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return FALSE;
}

DDS::Boolean
be_exception::make_put_param_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return FALSE;
}

DDS::Boolean
be_exception::make_get_param_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return FALSE;
}

ostream &
be_exception::put_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return os;
}

ostream &
be_exception::get_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return os;
}

ostream &
be_exception::put_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return os;
}

ostream &
be_exception::get_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return os;
}

ostream &
be_exception::put_for_sequence(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return os;
}

ostream &
be_exception::get_for_sequence(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return os;
}

ostream &
be_exception::put_for_array(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return os;
}

ostream &
be_exception::get_for_array(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   assert(FALSE); // not supported in IDL
   return os;
}

DDS_StdString
be_exception::kind_string()
{
   return "DDS::tk_except";
}

DDS::ULong
be_exception::get_elem_size()
{
   return 0;
}

DDS::ULong
be_exception::get_elem_alignment()
{
   return m_elemAlignment;
}
