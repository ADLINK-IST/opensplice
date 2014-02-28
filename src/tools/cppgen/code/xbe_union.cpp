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
#include "xbe_enum.h"
#include "xbe_utils.h"
#include "xbe_globals.h"
#include "xbe_literals.h"
#include "xbe_union.h"
#include "xbe_root.h"
#include "xbe_typedef.h"
#include "xbe_array.h"
#include "xbe_interface.h"
#include "xbe_predefined.h"
#include "xbe_string.h"
#include "xbe_utils.h"
#include "xbe_scopestack.h"
#include "xbe_cppfwd.h"
#include "xbe_genlist.h"

#include "cppgen_iostream.h"

const long be_union::NO_DEFAULT_INDEX = 0xffffffff;
// -------------------------------------------------
//  BE_UNION IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS4(be_union, AST_Union, be_CodeGenerator,
                     be_DispatchableType, be_Type)
IMPL_NARROW_FROM_DECL(be_union)
IMPL_NARROW_FROM_SCOPE(be_union)

be_union::be_union()
{
   isAtModuleScope(pbfalse);
}

be_union::be_union( AST_ConcreteType *dt,
                    UTL_ScopedName *n,
                    const UTL_Pragmas &p)
:
   AST_Decl (AST_Decl::NT_union, n, p),
   UTL_Scope (AST_Decl::NT_union, n, p),
   AST_Structure (AST_Decl::NT_union, n, p),
   AST_Union (dt, n, p),
   m_isFixedLength (TRUE),
   defaultBranch (0),
   discType (0),
   defaultValue (0),
   m_cppScope (g_feScopeStack.Top ()),
   m_cppType (g_feScopeStack.Top(), *n),
   defaultIndex (be_union::NO_DEFAULT_INDEX),
   m_interface_dependant(pbfalse)
{
   DDS_StdString barScopedName = NameToString(name(), "_");
   isAtModuleScope(pbfalse);
   // YO JOEY this is a lame way to assert

   if ( !disc_type() ||
         !(discType = be_Type::_narrow(disc_type())) )
   {
      assert(pbfalse);
   }

   localName = local_name()->get_string();
   enclosingScope = be_Type::EnclosingScopeString(this);


   m_tc_ctor_val = (DDS_StdString) barScopedName + "_ctor";
   m_tc_dtor_val = (DDS_StdString) barScopedName + "_dtor";
   m_tc_put_val = (DDS_StdString) barScopedName + "_put";
   m_tc_get_val = (DDS_StdString) barScopedName + "_get";
   m_tc_assign_val = (DDS_StdString) barScopedName +
      "_copy";

   m_any_op_id = barScopedName;
   // YO JOEY
   //  m_nullArg        = (DDS_StdString)"*(new " + ScopedName() + ")";
   m_typecode->kind = DDS::tk_union;
   m_typecode->id = get_decl_pragmas().get_repositoryID()->get_string();
   m_typecode->name_of_type = localName;
   m_typecode->disc_type = discType->m_typecode;
   m_typecode->has_default = defaultIndex;

   InitializeTypeMap(this);
   if (!imported())
      be_CppFwdDecl::Add(be_CppFwdDecl::UNION, this, m_cppScope);
}

void
be_union::InitializeTypeMap(be_Type* t)
{
   idlType = t;

   if (t)
   {
      DDS_StdString outName;
      AST_Type* t_ast = (AST_Type*)t->narrow((long) & AST_Type::type_id);

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

         t->VarSignature(VT_OutParam,
                         t->TypeName(),
                         VT_NonConst,
                         VT_Var,
                         VT_Reference);
         t->VarSignature(VT_Return,
                         t->TypeName(),
                         VT_NonConst,
                         VT_Var,
                         VT_NonReference);
      }
      else
      {
         outName = t->TypeName() + DDSOutExtension;

         t->OutTypeName(outName);
         t->ReturnTypeName(t->TypeName() + "*");

         t->VarSignature(VT_OutParam,
                         outName,
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

      if (BE_Globals::nesting_bug == pbtrue)
      {
         const char * streamoptypename = 0;

         t->StreamOpTypeName(NameToString(t_ast->name()));
         streamoptypename = t->StreamOpTypeName();
         ColonToBar((char *)streamoptypename);
      }

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

void
be_union::GenerateTypedefs(const DDS_StdString &scope,
                           const be_typedef& alias,
                           be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   DDS_StdString relTypeName = BE_Globals::RelativeScope(scope,
                               typeName);

   os << tab << "typedef " << relTypeName << " "
   << alias.LocalName() << ";" << nl;

   if (BE_Globals::isocpp_new_types)
     return;

   os << tab << "typedef " << relTypeName
   << DDSVarExtension << " "
   << alias.LocalName() << DDSVarExtension
   << ";" << nl;

   if (!IsFixedLength())
   {
      os << tab << "typedef " << relTypeName
      << DDSOutExtension << " "
      << alias.LocalName()
      << DDSOutExtension << ";" << nl;
   }
}

AST_UnionBranch*
be_union::add_union_branch(AST_UnionBranch *ub)
{
   if (AST_Union::add_union_branch(ub))
   {
      be_union_branch* branch;

      branch = (be_union_branch*)ub->narrow((long) & be_union_branch::type_id);
      if (branch)
      {
         m_interface_dependant |= branch->Type()->IsInterfaceDependant ();

         long branchValue = branch->ast_Value();
         DDS_StdString valStr = "(" + // use full scoping for NT and HP -- gks
            DiscTypeName() + ")";

         // INITIALIZE DEFAULT BRANCH LABEL

         if (branch->IsDefault())
         {
            valStr += BE_Globals::int_to_string(defaultValue);
            branch->be_Value(defaultValue);

            branchValue = defaultValue;
            defaultBranch = branch;
            defaultBranch->Label(valStr);
            branches.push_back(branch);
            defaultIndex = branches.size() - 1;
            m_typecode->has_default = defaultIndex;
         }
         else
         {
            valStr += BE_Globals::int_to_string((int)branchValue);
            branch->be_Value(branchValue);
            branch->Label(valStr);
            Append(*branch);

            //
            // MAINTAIN DEFAULT BRANCH LABEL
            // Note that default value may be any value not already assigned.
            // This crazy logic ensures that we don't violate this.
            //

            if (defaultValue == (unsigned long)branchValue)
            {
               DDS::Boolean notDone = TRUE;

               while (notDone)
               {
                  notDone = pbfalse;
                  defaultValue++;
                  valStr = "(" +
                           BE_Globals::RelativeScope(enclosingScope,
                                                     DiscTypeName()) +
                           ")" ;
                  valStr += BE_Globals::int_to_string(defaultValue);

                  UTL_Scope * s = (UTL_Scope*)narrow((long) & UTL_Scope::type_id);
                  assert(s);
                  UTL_ScopeActiveIterator* i = 0;
                  i = new UTL_ScopeActiveIterator(s, UTL_Scope::IK_decls);

                  for ( ; !(i->is_done()); i->next())
                  {
                     AST_Decl *adecl = i->item();

                     assert(adecl);

                     be_union_branch *branch;
                     branch = (be_union_branch *)adecl->narrow((long) & be_union_branch::type_id);

                     if (branch)
                     {
                        if (branch->be_Value() == defaultValue)
                        {
                           delete i;
                           notDone = pbtrue;
                           break;
                        }
                     }
                  }
               }

               if (defaultBranch)
               {
                  defaultBranch->Label(valStr);
                  defaultBranch->be_Value(defaultValue);
               }
            }

         }

         //
         // maintain our own fixed-length status
         //
         if (IsFixedLength() && !branch->IsFixedLength())
         {
            IsFixedLength(pbfalse);
            InitializeTypeMap(this);
         }

         //
         // add to the TypeCode
         //
         m_typecode->members.push_back(branch->Type()->m_typecode);

         m_typecode->member_names.push_back(strdup(branch->LocalName()));

         switch (m_typecode->disc_type->kind)
         {

               case DDS::tk_char:

               case DDS::tk_octet:

               case DDS::tk_boolean:
               m_typecode->disclist.push_back(new char((char)branchValue));
               break;

               case DDS::tk_wchar:

               case DDS::tk_short:

               case DDS::tk_ushort:
               m_typecode->disclist.push_back(new short((short)branchValue));
               break;

               case DDS::tk_enum:

               case DDS::tk_long:

               case DDS::tk_ulong:
               m_typecode->disclist.push_back(new long(branchValue));
               break;

               case DDS::tk_longlong:

               case DDS::tk_ulonglong:
               m_typecode->disclist.push_back(new DDS::LongLong(branchValue));
               break;

               default :
               assert(0);
               break;
         }
      }

      return ub;
   }

   return 0;
}

void
be_union::Append(be_union_branch& branch)
{
   // INSERT BEFORE THE DEFAULT BRANCH

   if (defaultBranch)
   {
      branches.remove(defaultBranch);

      branches.push_back(&branch);
      branches.push_back(defaultBranch);
   }
   else
   {
      branches.push_back(&branch);
   }
}

void be_union::GenerateOpenClassDefinition (be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << tab << "class " << DLLMACRO << LocalName() << nl;
   os << tab << "{" << nl;
   os << tab << "public:" << nl;
}

void
be_union::GenerateMembers(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   be_union_branch * branch;
   TList<be_union_branch *>::iterator bit;
   String_map done_branches(idlc_hash_str);

   os << tab << BE_Globals::RelativeScope(enclosingScope, DiscTypeName())
   << " " << DDSDiscMember << ";" << nl;

   os << tab << "DDS::Boolean " << DDSMemberSet << ";" << nl;

   //
   // fixed members go in anonymous union
   //

   if (branches.size())
   {
      os << tab << "union {" << nl;
      tab.indent();
      os << tab << "void * m_void;" << nl; // used for all non-FixedLength

      for (bit = branches.begin(); bit != branches.end(); bit++)
      {
         branch = *bit;

         if (!be_array::_narrow(branch->field_type()) && branch->IsFixedLength())
         {
            DDS_StdString privatename = (DDS_StdString)"m_" + branch->LocalName();
            if (done_branches.find(privatename) == done_branches.end())
            {
               os << tab << BE_Globals::RelativeScope(enclosingScope,
                                                      branch->BranchType())
               << " " << privatename << ";" << nl;

               done_branches[privatename] = privatename;
            }
         }
      }

      tab.outdent();
      os << tab << "} _union;" << nl;
   }
}

void
be_union::GenerateEquality(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   pbbool first = pbtrue;
   be_Tab tab(source);
   be_union_branch * branch;
   TList<be_union_branch *>::iterator bit;
   os << tab << "bool operator==(const " << LocalName() << "& that) const" <<
   nl << tab << "{" << nl;

   os << tab << "if (this != &that)" << nl;
   os << tab << "{" << nl;
   source.Indent();

   for (bit = branches.begin(); bit != branches.end(); bit++)
   {
      branch = *bit;

      if (first)
         os << tab;
      else
         os << tab << "else ";

      first = pbfalse;
      os << "if (that._d() == " << branch->Label() << ")" << nl << tab << tab << "return " << branch->LocalName() << "() == "
         << "that." << branch->LocalName() << "();" << nl;
   }
   source.Outdent();
   os << nl << tab << "return false;" << nl;
   os << tab << "}" << nl << tab << "else" << nl << tab << "{" << nl << tab << tab << "return true;" << nl << tab << "}" << nl;
   source.Outdent();
   os << tab << "}" << nl;

   os << tab << "bool operator!=(const " << LocalName() << "& that) const" <<
         nl << tab << "{" << nl << tab << tab << "return !(*this == that);"
         << nl << tab << "}" << nl;

}

void
be_union::GenerateTestMethod(be_ClientHeader& source)
{
    //Get base filename and append _testmethod.h
    DDS_StdString BaseFilename;
    BaseFilename = StripExtension(source.Filename());
    BaseFilename += "_testmethod.h";

    //Open or append to file
    be_Source testStream;
    if(!testStream.Open(BaseFilename))
        cerr << "Cannot open: " << BaseFilename << endl;

    be_Tab tab(testStream);
    ostream & ts = testStream.Stream();
    be_union_branch * branch;
    TList<be_union_branch *>::iterator bit;
    String_map done_branches(idlc_hash_str);

    ts << "namespace {" << nl;
    ts <<  "template <>" << nl <<  "::std::vector< ::"
                      << ScopedName() <<  " > generate_test_values< ::"
                      << ScopedName() << " >()"  << nl
                      << "{" << nl;
    testStream.Indent();
    ts << tab << "::std::vector< " << "::" << ScopedName() << " > values;" << nl;
    ts << tab << "::" << ScopedName() << " next;" << nl;

    for (bit = branches.begin(); bit != branches.end(); bit++)
    {
        branch = *bit;
        DDS_StdString privatename = (DDS_StdString) branch->LocalName();

        if(done_branches.find(privatename) == done_branches.end())
        {
            ts << tab << "::std::vector< " << (branch->Type()->IsStringType() ? "::std::string" : branch->TypeName())
               << " > branch_values_" << branch->LocalName() << " = generate_test_values< "
               << (branch->Type()->IsStringType() ? "::std::string" : branch->TypeName())  << " >();" << nl << nl;

            ts << tab << "for(::std::vector< " << (branch->Type()->IsStringType() ? "::std::string" : branch->TypeName())
               << " >::const_iterator i = branch_values_" << branch->LocalName() << ".begin();" << nl
               << tab << tab << tab << "i != branch_values_"
               << branch->LocalName() << ".end(); ++i)" << nl << tab <<"{" << nl;

            ts << tab << tab <<"next." << branch->LocalName() << "(*i);" << nl
               << tab << tab << "values.push_back(next);" << nl << tab <<"}" << nl << nl;

            done_branches[privatename] = privatename;
        }
    }
    ts << nl << "return values;" << nl << "}" << nl << "}" << nl;
    testStream.Outdent();
    testStream.Close();
}

void
be_union::GenerateCopyMF(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   pbbool first = pbtrue;
   be_Tab tab(source);
   be_union_branch * branch;
   TList<be_union_branch *>::iterator bit;

   os << tab << "void " << nl
   << tab << "_copy(const " << LocalName() << "& that)" << nl;
   os << tab << "{" << nl;
   source.Indent();

   os << tab << "if (this != &that)" << nl;
   os << tab << "{" << nl;
   source.Indent();

   for (bit = branches.begin(); bit != branches.end(); bit++)
   {
      branch = *bit;

      if (first)
         os << tab;
      else
         os << tab << "else ";

      first = pbfalse;

      if (!branch->IsDefault())
      {
         os << "if (that._d() == " << branch->Label() << ") ";
      }

      if (branch->Type()->IsArrayType() && !BE_Globals::isocpp_new_types)
      {
         const be_Type *t = branch->Type();
         os << "( that._union.m_void ? " << branch->LocalName() << "("
         << t->TypeName() << "_dup(that."
         << branch->LocalName() << "())) : deleteMember() );" << nl;
      }
      else if (branch->Type()->IsFixedLength())
      {
         os << branch->LocalName() << "(that."
         << branch->LocalName() << "());" << nl;
      }
      else
      {
         os << "( that._union.m_void ? " << branch->LocalName() << "(that."
         << branch->LocalName() << "()) : deleteMember() );" << nl;
      }
   }

   os << tab << "m__d = that._d();" << nl; //don't forget the discriminant
   os << tab << "m__d_set = that.m__d_set;" << nl;
   source.Outdent();

   os << tab << "}" << nl;

   source.Outdent();
   os << tab << "}" << nl;
}

void
be_union::GenerateDefaultConstructor(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << tab << LocalName() << "()" << nl
   << tab << ":" << tab.indent() << nl
   << tab << "m__d(("
   << BE_Globals::RelativeScope(enclosingScope,
                                DiscTypeName())
   << ")0), m__d_set (FALSE)";

   if (!IsFixedLength())
   {
      os << ", " << nl << tab << "_union()";
   }

   os << tab.outdent() << nl;
   os << tab << "{" << nl
   << tab << "}" << nl;
}

void
be_union::GenerateCopyConstructor(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << tab << LocalName() << "(const "
   << LocalName() << "& that)";
   os << tab << ":" << tab.indent() << nl
   << tab << "m__d(("
   << BE_Globals::RelativeScope(enclosingScope,
                                DiscTypeName())
   << ")0)";

   if (!IsFixedLength())
   {
      os << "," << nl << tab << "_union()";
   }

   os << tab.outdent() << nl;
   os << tab << "{" << nl << tab.indent()
   << tab << "_copy(that);" << nl << tab.outdent()
   << tab << "}" << nl;
}

// The Integrity PPC compiler has broken default copy constructors
// (when used in returns.)

void
be_union::GenerateMemcpyCopyConstructor(be_ClientHeader& source)
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

void
be_union::GenerateDestructor(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << tab << "~" << LocalName() << "()" << nl
   << tab << "{";

   if (!IsFixedLength())
   {
      os << tab.indent() << tab << "deleteMember();"
      << tab.outdent();
   }

   os << tab << "}" << nl;
}

void
be_union::GenerateDeleteMember(be_ClientHeader& source)
{
   be_Type *btype = 0;
   AST_Type * atype = 0;
   ostream & os = source.Stream();
   be_union_branch *branch;
   be_Tab tab(source);
   pbbool first = pbtrue;
   TList<be_union_branch *>::iterator bit;

   /////////////
   // need to instantiate a template before we can call DDS::release on it
   // so here, we will add a dummy method that uses the instantiated type.
   // this method will never be called
   /////////////

   os << tab << "void" << nl << tab << "_eorb_itor()" << nl
   << tab << "{" << nl;
   tab.indent();

   for (bit = branches.begin(); bit != branches.end(); bit++)
   {
      branch = *bit;
      atype = branch->field_type();
      assert(atype);

      btype = (be_Type *)atype->narrow((long) & be_Type::type_id);
      assert(btype);

      if (btype->IsSequenceType())
      {
         os << tab << "new "
         << BE_Globals::RelativeScope(be_Type::EnclosingScopeString(branch),
                                      branch->UnionMemberTypeName())
         << ";" << nl ;
      }
   }

   tab.outdent();
   os << tab << "}" << nl ;


   os << tab << "void" << nl << tab << "deleteMember()" << nl
   << tab << "{" << nl;
   tab.indent();

   for (bit = branches.begin(); bit != branches.end(); bit++)
   {
      branch = *bit;

      if (first)
         os << tab;
      else
         os << tab << "else ";

      first = FALSE;

      if (!branch->IsDefault())
      {
         os << "if (m__d == " << branch->Label() << ") ";
      }

      atype = branch->field_type();
      assert(atype);

      btype = (be_Type *)atype->narrow((long) & be_Type::type_id);
      assert(btype);

      if (btype->IsFixedLength() && !btype->IsArrayType())
      {
         os << "{/*do nothing*/ ;} " << nl;
      }
      else if (BE_Globals::isocpp_new_types && btype->IsStringType() )
      {
         os << "{ if (_union.m_void) "
            << btype->Releaser((DDS_StdString) "((" +
                               branch->UnionMemberTypeName() +
                               "*)_union.m_void)") << "}";
      }
      else if (btype->IsStringType())
      {
         os << "{ if (_union.m_void) "
            << btype->Releaser((DDS_StdString) "(" +
                               branch->UnionMemberTypeName() +
                               ")_union.m_void") << "}";
      }
      else if ((btype->IsArrayType() && !BE_Globals::isocpp_new_types) ||
               btype->IsOpaqueType() ||
               btype->IsTypeCodeType() ||
               btype->IsObjectType() ||
               btype->IsLocalObjectType())
      {
         os << "{ if (_union.m_void) "
            << btype->Releaser((DDS_StdString) "(" +
                               BE_Globals::RelativeScope(
                                  be_Type::EnclosingScopeString(branch),
                                  branch->UnionMemberTypeName()) +
                               ")_union.m_void") << "}";
      }
      else
      {
         os << "{ if (_union.m_void) "
            << btype->Releaser((DDS_StdString) "((" +
                               BE_Globals::RelativeScope(
                                  be_Type::EnclosingScopeString(branch),
                                  branch->UnionMemberTypeName()) +
                               "*)_union.m_void)") << "}";
      }

      os << nl;
   }

   os << tab << "m__d =("
   << BE_Globals::RelativeScope(enclosingScope, DiscTypeName())
   << ")0;";
   os << nl;

   if (!IsFixedLength())
   {
      os << tab << "_union.m_void = 0;";
      os << nl;
   }

   tab.outdent();
   os << tab << "}" << nl;
}

void
be_union::GenerateAssignmentOperator(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << tab << LocalName() << " & " << nl
   << tab << "operator=(const " << LocalName()
   << "& that)" << nl
   << tab << "{" << nl << tab.indent()
   << tab << "_copy(that);" << nl
   << tab << "return *this;" << nl << tab.outdent()
   << tab << "}" << nl;
}

static be_union_branch** s_branchArray;

static bool
BranchCompare(int a, int b)
{
   return s_branchArray[a]->be_Value() < s_branchArray[b]->be_Value();
}

static void
BranchSwap(int a, int b)
{
   be_union_branch* t = s_branchArray[a];
   s_branchArray[a] = s_branchArray[b];
   s_branchArray[b] = t;
}

void
be_union::GenerateAccessors(be_ClientHeader& source)
{
   ostream& os = source.Stream();
   be_Tab tab(source);
   be_union_branch* branch;
   String_map branches_done(idlc_hash_str);
   TList<be_union_branch *>::iterator bit;
   unsigned int i;

   os << tab << "void _d ("
      << BE_Globals::RelativeScope(enclosingScope, DiscTypeName())
      << " val)" << nl
      << tab << "{" << nl << tab.indent()
      << tab << DDSDiscMember << " = val;" << nl
      << tab << DDSMemberSet << " = TRUE;"  << tab.outdent()
      << tab << "}" << nl;
   os << tab << BE_Globals::RelativeScope(enclosingScope,
                                          DiscTypeName())
   << " _d () const" << nl
   << tab << "{" << nl << tab.indent()
   << tab << "return " << DDSDiscMember << ";" << nl << tab.outdent()
   << tab << "}" << nl;

   // BRANCH ACCESSORS

   for (bit = branches.begin(); bit != branches.end(); bit++)
   {
      branch = *bit;

      if (branches_done.find(branch->LocalName()) == branches_done.end())
      {
         branch->GenerateSetAccessor(source);
         branch->GenerateGetAccessor(source);
         branches_done[branch->LocalName()] = branch->LocalName();
      }
   }

   //
   // generate _default() function if there's an implicit default
   //

   if (!defaultBranch)
   {
      //
      // get maximum value for discriminator type
      //

      unsigned long maxPossibleValue = 0; // maximum value discriminator can hold
      // For portability, we ignore negative values and we ignore
      // values higher than 32767 even when the discriminator type
      // supports them.  Presumably no real union will have more than
      // 32768 cases.

      switch (discType->m_typecode->kind)
      {
         case DDS::tk_boolean:
            maxPossibleValue = 1;
            break;

         case DDS::tk_char:
            maxPossibleValue = 127;
            break;

         case DDS::tk_wchar:
         case DDS::tk_short:
         case DDS::tk_long:
         case DDS::tk_ushort:
         case DDS::tk_ulong:
         case DDS::tk_longlong:
         case DDS::tk_ulonglong:
            maxPossibleValue = 32767;
            break;

         case DDS::tk_enum:
         {
            maxPossibleValue = 0;
            // YO BEN the following cast and this whole switch should be
            // replaced by polymorphism
            UTL_ScopeActiveIterator iterEnum((be_enum*)discType, IK_decls);

            while (!iterEnum.is_done())
            {
               AST_Decl* decl = iterEnum.item();
               assert(decl);
               be_enum_val* enumValue =
                  (be_enum_val*)decl->narrow((long) & be_enum_val::type_id);
               assert(enumValue);

               if (enumValue->Value() >= maxPossibleValue)
               {
                  maxPossibleValue = enumValue->Value();
               }

               iterEnum.next();
            }
         }

         break;

         default: break;

         // assert(0); commented out to prevent bogus compiler error
      }

      // assert(maxPossibleValue != 0);

      //
      // find the lowest discriminator value that has no case (i.e. branch)
      //

      s_branchArray = new be_union_branch * [branches.size()];

      for (i = 0, bit = branches.begin(); bit != branches.end(); i++, bit++)
      {
         s_branchArray[i] = *bit;
      }

      Sort(branches.size(), BranchCompare, BranchSwap);

      unsigned long lowestUnusedValue = 0;

      for (i = 0; i < branches.size(); i++)
      {
         if (s_branchArray[i]->be_Value() == lowestUnusedValue)
         {
            ++lowestUnusedValue;
         }
      }

      //
      // if there's an implicit default (discriminator value with no case),
      // then generate a _default() function
      //

      DDS_StdString valStr = "(" + // use full scoping for NT and HP -- gks
         DiscTypeName() +
         ")";


      if (lowestUnusedValue <= maxPossibleValue)
      {
         os << nl << tab << "void" << nl
         << tab << "_default()" << nl
         << tab << "{" << nl << tab.indent()
         << tab << DDSDiscMember << " = " << "("
         << DiscTypeName() << ")"
         << lowestUnusedValue << ";"
         << tab << DDSMemberSet << " = TRUE;"
         << tab.outdent() << nl
         << tab << "}" << nl << nl;
      }
   }
}

DDS_StdString
be_union::Allocater(const DDS_StdString& arg) const
{
   DDS_StdString ret = arg + " = new " + typeName + ";";

   return ret;
}

DDS_StdString
be_union::Initializer(const DDS_StdString& arg, VarType vt) const
{
   DDS_StdString ret = arg ;

   if (!IsFixedLength() && (vt == VT_OutParam || vt == VT_Return))
   {
      ret = arg + " = 0";
   }

   return ret + ";";
}

DDS_StdString
be_union::Releaser(const DDS_StdString& arg) const
{
   return (DDS_StdString)"delete(" + arg + ");" ;
}

DDS_StdString
be_union::Assigner(const DDS_StdString& arg, const DDS_StdString& val) const
{
   return arg + " = " + val + ";";
}

DDS_StdString be_union::Duplicater
(
   const DDS_StdString & arg,
   const DDS_StdString & val,
   const DDS_StdString & currentScope,
   const pbbool isConst
) const
{
   DDS_StdString relativeName = BE_Globals::RelativeScope(currentScope, typeName);
   DDS_StdString ret = arg + " = new " + relativeName + "(" + val + ");";
   return ret;
}

DDS_StdString
be_union::NullReturnArg()
{
   DDS_StdString ret = nullArg;

   if (!IsFixedLength())
   {
      ret = (DDS_StdString)"(" + ScopedName() + "*)0";
   }

   return ret;
}

void
be_union::GenerateUnion(be_ClientHeader& source)
{
   ostream & os = source.Stream();
   be_Tab tab(source);

   g_cppScopeStack.Push(m_cppType);

   //
   // open class definition
   //
   os << tab << "class " << DLLMACRO << LocalName() <<  (BE_Globals::isocpp_new_types ? " OSPL_DDS_FINAL" : "") << nl;
   os << tab << "{" << nl;
   os << tab << "public:" << nl;
   tab.indent();

   // remember module scoping in case recursion usage of union causes
   // scoping to be reset
   bool is_at_module_scope = isAtModuleScope();

   //
   // define nested types
   //
   be_CodeGenerator::Generate(source);

   // Restore scoping
   be_CodeGenerator::isAtModuleScope (is_at_module_scope);

   //
   // additional methods for var len unions
   //

   if (!IsFixedLength())
   {
      GenerateCopyMF(source);
      GenerateDefaultConstructor(source);
      GenerateCopyConstructor(source);
      GenerateDestructor(source);
      GenerateDeleteMember(source);
      if (BE_Globals::isocpp_new_types)
      {
        // C++ 11 move constructor, copy consructor, and assignement ops
        source.Outdent();
        os << "#ifdef OSPL_DDS_CXX11" << nl;
        source.Indent();
        os << tab << LocalName() << "(" << LocalName() << "&& _other) :" << nl;
        os << tab << tab << "m__d(::std::move(_other.m__d))," << nl;
        os << tab << tab << "m__d_set(::std::move(_other.m__d_set))" << nl;
        os << tab << "{" << nl;
        os << tab << tab << "::std::memcpy(&_union, &_other._union, sizeof(_union));" << nl;
        os << tab << tab << "::std::memset(&_other._union, 0, sizeof(_other._union));" << nl;
        os << tab << "}" << nl;
        os << tab <<  LocalName() << "& operator=(" << LocalName() << "&& _other)" << nl;
        os << tab << "{" << nl;
        os << tab << tab << "deleteMember();" << nl;
        os << tab << tab << "m__d = ::std::move(_other.m__d);" << nl;
        os << tab << tab << "m__d_set = ::std::move(_other.m__d_set);" << nl;
        os << tab << tab << "::std::memcpy(&_union, &_other._union, sizeof(_union));" << nl;
        os << tab << tab << "::std::memset(&_other._union, 0, sizeof(_other._union));" << nl;
        os << tab << tab << "return *this;" << nl;
        os << tab << "}" << nl;
        source.Outdent();
        os << "#endif" << nl;
        source.Indent();
      }
      GenerateAssignmentOperator(source);
   }
   else
   {
     if (BE_Globals::isocpp_new_types)
     {
        // C++ 11 move constructor, copy consructor, and assignement ops
        source.Outdent();
        os << "#ifdef OSPL_DDS_CXX11" << nl;
        os << "#  ifdef OSPL_CXX11_NO_FUNCTION_DEFAULTS" << nl;
        source.Indent();
        GenerateCopyMF(source);
        GenerateDefaultConstructor(source);
        GenerateCopyConstructor(source);
        os << tab << LocalName() << "(" << LocalName() << "&& _other) :" << nl;
        os << tab << tab << "m__d(::std::move(_other.m__d))," << nl;
        os << tab << tab << "m__d_set(::std::move(_other.m__d_set))" << nl;
        os << tab << "{" << nl;
        os << tab << tab << "::std::memcpy(&_union, &_other._union, sizeof(_union));" << nl;
        os << tab << tab << "::std::memset(&_other._union, 0, sizeof(_other._union));" << nl;
        os << tab << "}" << nl;
        os << tab <<  LocalName() << "& operator=(" << LocalName() << "&& _other)" << nl;
        os << tab << "{" << nl;
        os << tab << tab << "m__d = ::std::move(_other.m__d);" << nl;
        os << tab << tab << "m__d_set = ::std::move(_other.m__d_set);" << nl;
        os << tab << tab << "::std::memcpy(&_union, &_other._union, sizeof(_union));" << nl;
        os << tab << tab << "::std::memset(&_other._union, 0, sizeof(_other._union));" << nl;
        os << tab << tab << "return *this;" << nl;
        os << tab << "}" << nl;
        source.Outdent();
        os << "#  else" << nl;
        source.Indent();
        os << tab << LocalName() << "() = default;" << nl;
        os << tab << LocalName() << "(const " << LocalName() << "& _other) = default;" << nl;
        os << tab << LocalName() << "(" << LocalName() << "&& _other) = default;" << nl;
        os << tab <<  LocalName() << "& operator=(" << LocalName() << "&& _other) = default;" << nl;
        os << tab <<  LocalName() << "& operator=(const "  << LocalName() << "& _other) = default;" << nl;
        source.Outdent();
        os << "#  endif" << nl;
        os << "#endif" << nl;
        source.Indent();
     }
   }

   if(BE_Globals::gen_equality)
      GenerateEquality(source);

   if(BE_Globals::isocpp_test_methods)
       GenerateTestMethod(source);

   //
   // accessors and mutators
   //
   GenerateAccessors(source);

   //
   // members (public for reader)
   //
   GenerateMembers(source);

   //
   // close union definition
   //
   tab.outdent();

   os << tab << "};" << nl;

   g_cppScopeStack.Pop();
}

void be_union::GenerateAuxTypes (be_ClientHeader& source)
{
   if (BE_Globals::isocpp_new_types)
     return;
   DDS_StdString varName = LocalName() + "_var";
   DDS_StdString outName = LocalName() + "_out";
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
be_union::Generate(be_ClientHeader& source)
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

      Generated(pbtrue);

      be_root::GenerateDependants(source,
                                  SequenceMemberTypeName(),
                                  EnclosingScope());

      GenerateUnion(source);
      GenerateAuxTypes(source);
   }
}

void be_union::Generate (be_ClientImplementation& source)
{
   if (BE_Globals::ignore_interfaces && IsInterfaceDependant ())
   {
      return;
   }

   //
   // define nested types
   //
   UTL_Scope* scope = (UTL_Scope*)narrow((long) & UTL_Scope::type_id);

   UTL_ScopeActiveIterator scopeIter(scope, UTL_Scope::IK_localtypes);

   while (!(scopeIter.is_done()))
   {
      be_CodeGenerator* cg;

      AST_Decl* decl = scopeIter.item();

      if (!decl->imported() &&
            (cg = (be_CodeGenerator*)decl->narrow((long) & be_CodeGenerator::type_id)))
      {
         cg->Generate(source);
      }

      scopeIter.next();
   }
}

be_union *
be_union::_narrow(AST_Type * atype)
{
   be_union * ret = 0;

   if (atype)
   {
      ret = (be_union*)atype->narrow((long) & be_union::type_id);
   }

   return ret;
}

void be_union::isAtModuleScope (bool is_at_module)
{
   be_union_branch * branch;
   TList<be_union_branch *>::iterator bit;

   be_CodeGenerator::isAtModuleScope (is_at_module);

   for (bit = branches.begin(); bit != branches.end(); bit++)
   {
      branch = *bit;
      be_CodeGenerator* generator = (be_CodeGenerator*) branch->Type()
         ->narrow ((long) & be_CodeGenerator::type_id);

      // Only change branch if different to avoid infinite recursion
      if (generator && (generator->isAtModuleScope () != is_at_module))
      {
         generator->isAtModuleScope (is_at_module);
      }
    }
}

bool be_union::isAtModuleScope () const
{
   return be_CodeGenerator::isAtModuleScope ();
}

DDS::Boolean
be_union::is_core_marshaled()
{
   return TRUE;
}

DDS::Boolean be_union::declare_for_stub
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
            os << tab << relTypeName << "_var " << arg
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
be_union::declare_for_struct_put(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return FALSE;
}


DDS::Boolean
be_union::declare_for_union_get(
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


DDS::Boolean
be_union::declare_for_struct_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return FALSE;
}


DDS::Boolean be_union::make_get_param_for_stub
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
               break;
            default: assert (0);
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
            default: assert (0);
         }
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_union::make_put_param_for_stub
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
            os << "(" << TypeName() << "*)" << "&" << arg << ", DDS::PARAM_IN ";
            break;

         case VT_InOutParam:
            os << "&" << arg << ", DDS::PARAM_IN ";
            break;
         default: assert (0);
      }

      os << "}";
      ret = TRUE;
   }

   return ret;
}

DDS::Boolean be_union::make_put_param_for_union
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

DDS::Boolean be_union::make_get_param_for_union
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
      os << "{ " << Scope(TypeCodeTypeName())
      << ", &" << sptr << "->" << fld << ", DDS::PARAM_OUT }";
   }
   else
   {
      os << "{ " << Scope(TypeCodeTypeName())
      << ", " << sptr << "->" << fld << ", DDS::PARAM_OUT }";
   }

   return TRUE;
}

DDS::Boolean
be_union::make_put_param_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
#if defined(OPTIMIZE)
   os << "    " << m_tc_put_val << "(os, (void*)&__" << fld
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
#else

   os << tab << "{ " << Scope(TypeCodeTypeName())
   << ", &" << sptr << "->" << fld << ", DDS::PARAM_IN }";
#endif

   return TRUE;
}

DDS::Boolean
be_union::make_get_param_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
#if defined(OPTIMIZE)
   os << "    " << m_tc_get_val << "(is, (void*&)__" << fld
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
#else

   os << tab << "{ " << Scope(TypeCodeTypeName())
   << ", &" << sptr << "->" << fld << ", DDS::PARAM_OUT }";
#endif

   return TRUE;
}

ostream &
be_union::put_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   tab.indent();
   os << tab << m_tc_put_val << "(os, (void*)&" << sptr << "->" << fld
      << ", DDS::PARAM_IN" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent();

   return os;
}

ostream &
be_union::get_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   //
   // first declare the field
   //
   os << tab << ScopedName() << " * ___" << fld << " = &("
   << sptr << "->" << fld << ");" << nl;

   tab.indent();
   os << tab << m_tc_get_val << "(is, (void*&)___" << fld
      << ", DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent();

   return os;
}

ostream &
be_union::put_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   tab.indent();
   os << tab << m_tc_put_val << "(os, (void*)&" << sptr << "->" << fld
      << ", DDS::PARAM_IN" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent();

   return os;
}

ostream &
be_union::get_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   //
   // first declare the field
   //
   os << tab << ScopedName() << " * ___" << fld << " = &("
   << sptr << "->" << fld << ");" << nl;

   tab.indent ();
   os << tab << m_tc_get_val << "(is, (void*&)___" << fld
      << ", DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent ();

   return os;
}

ostream & be_union::put_for_sequence
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

   tab.indent ();

   os << tab << m_tc_put_val << "(os, " << sptr
      << ", DDS::PARAM_IN" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
   tab.outdent ();

   return os;
}

ostream &
be_union::get_for_sequence(
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

   tab.indent();

   os << nl << tab << m_tc_get_val
      << "(is, (void*&)" << sptr
      << ", DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   tab.outdent();

   return os;
}

ostream &
be_union::put_for_array(
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
   << arg << "[" << index
   << "];" << nl;

   tab.indent();

   os << tab << m_tc_put_val << "(os, " << sptr
      << ", DDS::PARAM_IN" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   tab.outdent();

   return os;
}

ostream &
be_union::get_for_array(
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

   tab.indent();

   os << nl << tab << m_tc_get_val
      << "(is, (void*&)" << sptr
      << ", DDS::PARAM_OUT" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   tab.outdent();

   return os;
}

be_DispatchableType::en_HowStoredInDispatcher
be_union::HowStoredInDispatcher(
   const be_ArgumentDirection& direction) const
{
   be_DispatchableType::en_HowStoredInDispatcher ret = STORED_AS_STACK_VARIABLE;

   if (! IsFixedLength ())
   {
      // variable-length union

      switch (direction)
      {
         case VT_InParam:
         case VT_InOutParam:
            ret = STORED_AS_STACK_VARIABLE;
            break;
         case VT_OutParam:
         case VT_Return:
            ret = STORED_IN_VAR;
            break;
         default: assert (0);
      }
   }

   return ret;
}

DDS_StdString
be_union::kind_string()
{
   return "DDS::tk_union";
}

DDS::ULong
be_union::get_elem_size()
{
   return 0;
}

DDS::ULong
be_union::get_elem_alignment()
{
   be_Type * discBType = be_typedef::_beBase(disc_type());

   return discBType->get_elem_alignment();
}

void be_union::putter
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   unsigned long uid
)
{
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString _in_ = (DDS_StdString)"_in_" + uids;
   TList<be_union_branch *>::iterator bit;
   DDS::Boolean first = TRUE;
   be_Type * discBType = be_typedef::_beBase(disc_type());

   // initialize the member that we'll actually put

   for (bit = branches.begin (); bit != branches.end(); bit++)
   {
      be_union_branch * branch;
      be_Type * btype;

      branch = *bit;
      btype = be_typedef::_beBase (branch->field_type());

      if (first)
      {
         first = FALSE;
      }
      else
      {
         os << tab << "else" << nl;
      }

      if (!branch->IsDefault())
      {
         os << tab << "if (" << sptr << "->" << DDSDiscMember
         << " == " << branch->Label () << ") " << nl;
      }

      os << tab << "{" << nl;

      // declare the branch's put value if necessary

      tab.indent ();
      btype->declare_for_union_put (os, tab, sptr, branch->PrivateName (), uid);
      os << tab << "DDS::Codec::Param " << _in_ << "[2] =" << nl;
      os << tab << "{" << nl;
      tab.indent ();

      // make put args of discriminant and branch value

      discBType->make_put_param_for_struct (os, tab, sptr, DDSDiscMember, uid);
      os << "," << nl;
      btype->make_put_param_for_union (os, tab, sptr, branch->PrivateName (), uid);
      tab.outdent ();
      os << nl << tab << "};" << nl;

      // and finally, let's put 'em

      os << tab << "os.put (" << _in_ << ", 2"
          << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

      tab.outdent ();
      os << tab << "}" << nl;
   }

   //
   // put the discriminator by itself (for implicit defaults)
   //

   if (!HasExplicitDefault())
   {
      os << tab << "else" << nl;
      os << tab << "{" << nl;
      tab.indent();
      os << tab << "DDS::Codec::Param putArg = " << nl;
      tab.indent();
      discBType->make_put_param_for_struct(os, tab, sptr, DDSDiscMember, uid);
      os << ";" << nl;
      tab.outdent();
      os << tab << "os.put (&putArg, 1" << XBE_Ev::arg (XBE_ENV_VARN)
         << ");" << nl;
      tab.outdent();
      os << tab << "}" << nl;
   }
}

void be_union::getter
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   unsigned long uid
)
{
   DDS_StdString uids = BE_Globals::ulong_to_string(uid);
   DDS_StdString uids1 = BE_Globals::ulong_to_string(uid + 1);
   TList<be_union_branch *>::iterator bit;
   DDS_StdString getDiscArgs = (DDS_StdString)"_out_" + uids;
   DDS_StdString getMemArgs = (DDS_StdString)"_out_" + uids1;
   DDS::Boolean first = TRUE;
   be_Type * discBType = be_typedef::_beBase (disc_type ());

   // manage old data's memory

   if (!IsFixedLength ())
   {
      os << nl << tab << sptr << "->deleteMember ();" << nl << nl;
   }

   // make get args of discriminant

   os << tab << "DDS::Codec::Param " << getDiscArgs << "[1] = {";
   discBType->make_get_param_for_struct (os, tab, sptr, DDSDiscMember, uid);
   os << "};" << nl;

   // let's get it

   os << tab << "is.get (" << getDiscArgs << ", 1"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl << nl;

   uid++;
   first = pbtrue;

   for (bit = branches.begin(); bit != branches.end(); bit++)
   {
      be_union_branch * branch;
      be_Type * btype;

      branch = *bit;
      btype = be_typedef::_beBase(branch->field_type());

      if (first)
      {
         os << tab;
      }
      else
      {
         os << tab << "else" << nl;
      }

      first = pbfalse;

      if (!branch->IsDefault ())
      {
         os << "if (" << sptr << "->" << DDSDiscMember
         << " == " << branch->Label () << ") " << nl;
      }

      os << tab << "{" << nl;
      //
      // declare the branch's get value if necessary
      //
      tab.indent ();
      btype->declare_for_union_get (os, tab, sptr, branch->PrivateName(), uid);

      // make get args of branch value

      os << tab << "DDS::Codec::Param " << getMemArgs << "[1] = {";
      btype->make_get_param_for_union (os, tab, sptr, branch->PrivateName (), uid);
      os << "};" << nl;

      // and finally, let's get 'em

      os << tab << "is.get (" << getMemArgs << ", 1"
         << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;
      tab.outdent ();
      os << tab << "}" << nl;
   }
}

void be_union::generate_tc_ctor_val (be_Source & source)
{
   be_Type::generate_tc_ctor_val(source);
}


void be_union::generate_tc_dtor_val
(
   be_Source & source,
   pbbool isCounted
)
{
   be_Type::generate_tc_dtor_val (source, FALSE);
}


void be_union::generate_tc_put_val (be_Source & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   unsigned long uid = 0;

   // declare writer body

   os << tab << "void " << m_tc_put_val << nl;
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
   os << tab << ")" << nl;
   os << tab << "{" << nl;
   tab.indent ();

   // first, cast that pesky void *

   os << tab << ScopedName () << " * p = (" << ScopedName () << "*) arg;" << nl;

   // check if initialised

   os << tab << "if (!p->m__d_set)" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   os << tab
      << "DDS::BAD_PARAM ex (DDS_BAD_PARAM_M18, DDS::COMPLETED_NO);"
      << nl;
   os << tab;
   XBE_Ev::throwex (os, "ex");
   os << nl;
   tab.outdent ();
   os << tab << "}" << nl;

   // now, let's put our fields

   putter (os, tab, "p", uid);

   tab.outdent ();
   os << tab << "}" << nl << nl;
}

void be_union::generate_tc_get_val (be_Source & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   unsigned long uid = 0;

   // declare reader body

   os << tab << "void " << m_tc_get_val << nl;
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
   os << tab << ")" << nl;
   os << tab << "{" << nl;
   tab.indent ();

   // first, cast that pesky void *

   os << tab << ScopedName () << " * p = (" << ScopedName () << "*) arg;" << nl;

   // mark as initialised

   os << tab << "p->m__d_set = TRUE;" << nl;

   // now, let's get our fields

   getter (os, tab, "p", uid);

   tab.outdent ();
   os << tab << "}" << nl << nl;
}

void be_union::generate_tc_assign_val (be_Source & source)
{
   be_Type::generate_tc_assign_val (source);
}

DDS_StdString
be_union::SyncStreamOut(const DDS_StdString& arg,
                        const DDS_StdString& out,
                        VarType vt) const
{
   DDS_StdString ret = out;

   if ( !IsFixedLength() &&
         (vt == VT_Return || vt == VT_OutParam) )
   {
      ret += " << (" + ScopedName() + " *)" + arg + ";";
   }
   else
   {
      ret += " << " + arg + ";";
   }

   return ret;
}

// -------------------------------------------------
//  BE_UNIONBRANCH IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS2(be_union_branch, AST_UnionBranch, be_TypeMap)
IMPL_NARROW_FROM_DECL(be_union_branch)

be_union_branch::be_union_branch()
{}

be_union_branch::be_union_branch
(
   AST_UnionLabel * lab,
   AST_Type * ft,
   UTL_ScopedName * n,
   const UTL_Pragmas & p
)
:
   AST_Decl (AST_Decl::NT_union_branch, n, p),
   AST_Field (AST_Decl::NT_union_branch, ft, n, p),
   AST_UnionBranch (lab, ft, n, p),
   be_value (0),
   type (0)
{
   assert(field_type());

   if (field_type())
   {
      type = (be_Type*)field_type()->narrow((long) & be_Type::type_id);
   }

   InitializeTypeMap(type);
   branchType = UnionMemberTypeName();
   branchlabel = LabelString(label());
}

void
be_union_branch::Initialize()
{
   if (type)
   {
      type->Initialize();
   }
}

long
be_union_branch::ast_Value()
{
   long ret = 0;
   AST_UnionLabel * lab = label();

   assert(lab);

   if (lab && (lab->label_kind() != AST_UnionLabel::UL_default))
   {
      AST_Expression::AST_ExprValue *val = lab->label_val()->ev();
      assert(val);
      ret = ExprToLong(val);
   }

   return ret;
}

DDS_StdString
be_union_branch::LabelString(AST_UnionLabel* label)
{
   DDS_StdString ret = "Unknown Union Label";

   if (label)
   {
      ostringstream os;
      AST_Expression * expr = label->label_val();
      AST_Expression::AST_ExprValue * val;

      label->dump(os);
      os << ends;

      if (expr && (val = expr->ev()) && (val->et == AST_Expression::EV_char))
      {
         ret = (DDS_StdString) "'" + os.str().c_str() + "'";
      }
      else
      {
         ret = (DDS_StdString) os.str().c_str();
      }
   }
   else
   {
      DDSError((DDS_StdString)"unable to obtain label for union branch");
   }

   return ret;
}

pbbool
be_union_branch::IsDefault()
{
   pbbool ret = pbfalse;
   AST_UnionLabel * lab = label();

   assert(lab);

   if (lab && (lab->label_kind() == AST_UnionLabel::UL_default))
   {
      ret = pbtrue;
   }

   return ret;
}

/*
 * PrivateName()  --  Returns the name of the C++ union member that
 *        corresponds to this branch
 */

DDS_StdString
be_union_branch::PrivateName()
{
   DDS_StdString privatename = " _union.m_void";

   if (!be_array::_narrow(field_type()) && IsFixedLength())
   {
      privatename = (DDS_StdString)" _union.m_" + LocalName();
   }

   return privatename;
}

void be_union_branch::GenerateASetAccessor
(
   be_ClientHeader & source,
   const DDS_StdString & argType,
   const pbbool isConst
)
{
   AST_Type * atype = field_type();
   be_Type * btype = 0;
   ostream & os = source.Stream();
   be_Tab tab (source);
   DDS_StdString assignee("");
   DDS_StdString deleteMember("");
   UTL_Scope * enclosingScope = 0;
   be_union * enclosingUnion = 0;

   // CHECK FOR TYPEDEF AND GET BASE TYPE

   if (be_typedef::_narrow (atype))
   {
      atype = be_typedef::_astBase(atype);
   }

   btype = (be_Type *)atype->narrow((long) & be_Type::type_id);
   assert(btype);

   os << tab << "void " << LocalName () << " (";

   be_interface* bi;
   DDS_StdString strRelativeScope;

   if ((bi = be_interface::_narrow (atype)))
   {
      strRelativeScope = BE_Globals::RelativeScope
      (
         be_Type::EnclosingScopeString(this),
         UnionMemberTypeName()
      );

      os << strRelativeScope << DDSPtrExtension << " _val_)";
   }
   else
   {
      strRelativeScope = BE_Globals::RelativeScope
      (
         be_Type::EnclosingScopeString (this),
         argType
      );

      os << strRelativeScope << " _val_)";
   }

   // Assign

   enclosingScope = defined_in ();
   assert (enclosingScope);

   enclosingUnion = (be_union*)enclosingScope->narrow((long) & be_union::type_id);
   assert(enclosingUnion);

   if (!enclosingUnion->IsFixedLength())
   {
      deleteMember = " deleteMember (); ";
   }
   else
   {
      deleteMember = " _union.m_void = 0; ";
   }

   // IF IT's AN ANY

   be_predefined_type * tmp = be_predefined_type::_narrow(atype);

   if ((tmp != 0) && (tmp->pt() == AST_PredefinedType::PT_any) )
   {
      os << tab << "{ " << deleteMember << "_union.m_void = new " << argType << "(_val_);";
   }
   else if (!btype->IsArrayType() && IsFixedLength())
   {
      os << tab << "{ " << deleteMember << PrivateName() << " = _val_; ";
   }
   else
   {
      os << tab << "{ " << deleteMember
         << btype->Duplicater ("_union.m_void", "_val_", be_Type::EnclosingScopeString(this), isConst);
   }

   // AND SET DISCRIMINANT
   os << DDSDiscMember << " = " << Label() << "; ";
   os << DDSMemberSet << " = TRUE; }" << nl;
}

void be_union_branch::GenerateSetAccessor (be_ClientHeader& source)
{
   ostream & os = source.Stream ();
   AST_Type * atype = field_type ();
   DDS_StdString setParam;
   be_interface * bi;
   pbbool isConst = 0;

   os << nl;

   // Check for typedef and get base type

   if (be_typedef::_narrow (atype))
   {
      atype = be_typedef::_astBase (atype);
   }

   // Then set parameter type

   if
   (
      be_predefined_type::_narrow (atype) ||
      be_enum::_narrow (atype) ||
      be_array::_narrow (atype)
   )
   {
      setParam = BranchType ();
   }
   else if (be_Type::_narrow (atype)->IsInterfaceType ())
   {
      bi = be_interface::_narrow (atype);
      setParam = BE_Globals::RelativeScope
         (be_Type::EnclosingScopeString(this), bi->BaseClassname())
         + DDSPtrExtension;
   }
   else if (be_Type::_narrow (atype)->IsOpaqueType ())
   {
      setParam = (DDS_StdString) "const "
         + BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this),
           BranchType()) + "const &";
   }
   else
   {
      setParam = (DDS_StdString) "const "
         + BE_Globals::RelativeScope (be_Type::EnclosingScopeString(this), BranchType()) + "&";

      if (be_Type::_narrow(field_type())->IsStringType())
      {
         be_string* sbt = be_string::_narrow(atype);

         if (BE_Globals::isocpp_new_types)
         {
            setParam = (DDS_StdString)"const ::std::string&";
            isConst = 1;
         }
         else if (sbt->IsWide())
         {
            GenerateASetAccessor (source, "DDS::WChar*", 0);
            GenerateASetAccessor (source, "const DDS::WChar*", 1);
            setParam = (DDS_StdString)"const DDS::WString_var&";
            isConst = 1;
         }
         else
         {
            GenerateASetAccessor (source, "char*", 0);
            GenerateASetAccessor (source, "const char*", 1);
            setParam = (DDS_StdString)"const DDS::String_var&";
            isConst = 1;
         }
      }
   }

   // Generate set accessor

   GenerateASetAccessor (source, setParam, isConst);
}

void be_union_branch::GenerateAGetAccessor
(
   be_ClientHeader& source,
   const DDS_StdString& getReturn,
   pbbool isConst
)
{
   AST_Type * atype = field_type();
   be_Type * btype;
   ostream & os = source.Stream();
   be_Tab tab(source);

   if (be_typedef::_narrow(atype))
   {
      atype = be_typedef::_astBase(atype);
   }

   os << tab << getReturn << " " << BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), LocalName()) << "() ";

   if (isConst)
   {
      os << "const ";
   }

   os << "{ return ";

   btype = (be_Type *)atype->narrow((long) & be_Type::type_id);
   assert (btype);

   if (!btype->IsArrayType() && IsFixedLength())
   {
      os << PrivateName() << "; }" << nl;
   }
   else
   {
      if ((btype->IsStringType() || btype->IsArrayType()) && !BE_Globals::isocpp_new_types)
      {
         os << "(" << BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), UnionMemberTypeName()) << ")_union.m_void; }" << nl;
      }
      else if (btype->IsObjectType())
      {
         os << "(" << BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), UnionMemberTypeName()) << ")_union.m_void; }" << nl;
      }
      else if (btype->IsLocalObjectType())
      {
         os << "(" << BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), UnionMemberTypeName()) << ")_union.m_void; }" << nl;
      }
      else if (btype->IsInterfaceType())
      {
         os << "(" << BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), UnionMemberTypeName()) << "*)_union.m_void; }" << nl;
      }
      else if (btype->IsOpaqueType())
      {
         DDS_StdString const_stuff("");

         if (isConst)
            const_stuff = " const ";

         os << "*((" << const_stuff
         << BE_Globals::RelativeScope(
            be_Type::EnclosingScopeString(this), UnionMemberTypeName())
         << "*)&_union.m_void); }" << nl;
      }
      else if (btype->IsTypeCodeType())
      {
         os << "("
            << BE_Globals::RelativeScope (be_Type::EnclosingScopeString (this),
                                          UnionMemberTypeName ())
            << ") _union.m_void; }" << nl;
      }
      else
      {
         os << "*(" << BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), UnionMemberTypeName()) << "*)_union.m_void; }" << nl;
      }
   }
}

void be_union_branch::GenerateGetAccessor (be_ClientHeader& source)
{
   AST_Type * atype = field_type();
   DDS_StdString getReturn;
   be_array * ba;
   be_interface* bi;

   // CHECK FOR TYPEDEF AND GET BASE TYPE

   if (be_typedef::_narrow(atype))
   {
      atype = be_typedef::_astBase(atype);
   }

   // NOW GENERATE GET ACCESSOR(S)
   be_predefined_type * tmp = be_predefined_type::_narrow(atype);

   if (((tmp != 0) && (tmp->pt() != AST_PredefinedType::PT_any)) || be_enum::_narrow(atype))
   {
      getReturn = BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), BranchType());
   }
   else if (be_string::_narrow(atype) && !BE_Globals::isocpp_new_types)
   {
      be_string* sbt = be_string::_narrow(atype);

      if (sbt->IsWide())
         getReturn = "const DDS::WChar*";
      else
         getReturn = "const char*";
   }
   else if ((ba = be_array::_narrow(atype)) && !BE_Globals::isocpp_new_types)
   {
      getReturn = BE_Globals::RelativeScope
      (
         be_Type::EnclosingScopeString(this),
         UnionMemberTypeName()
      );
   }
   else if ((bi = be_interface::_narrow (atype)))
   {
      DDS_StdString strRelScope;
      strRelScope = BE_Globals::RelativeScope
      (
         be_Type::EnclosingScopeString(this),
         UnionMemberTypeName()
      );

      getReturn = strRelScope + DDSPtrExtension;
   }
   else  // struct, union, sequence, any, opaque
   {
      getReturn = BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), BranchType()) + "&";

      GenerateAGetAccessor(source, getReturn, pbfalse);

      getReturn = (DDS_StdString)"const " + BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), UnionMemberTypeName()) + "&";
   }

   GenerateAGetAccessor(source, getReturn, pbtrue);
}

void
be_union_branch::InitializeTypeMap(be_Type*)
{
   if (type)
   {
      type->Initialize();
      typeName = type->TypeName();
      inTypeName = type->InTypeName();
      inoutTypeName = type->InOutTypeName();
      outTypeName = type->OutTypeName();
      returnTypeName = type->MakeSignature(VT_Return);
      dmfAdtMemberTypeName = type->DMFAdtMemberTypeName();
      structMemberTypeName = type->StructMemberTypeName();
      unionMemberTypeName = type->UnionMemberTypeName();
      sequenceMemberTypeName = type->SequenceMemberTypeName();
      streamOpTypeName = type->StreamOpTypeName();
      istreamOpTypeName = type->IStreamOpTypeName();
   }
   else
   {
      assert(0);
   }
}

pbbool
be_union_branch::IsFixedLength() const
{
   pbbool ret = (type) ? type->IsFixedLength() && !(type->IsStructuredType() && BE_Globals::isocpp_new_types) : pbfalse;

   return ret;
}

pbbool
be_union_branch::IsFixedLengthPrimitiveType() const
{
   return FALSE;
}

// -------------------------------------------------
//  BE_UNION_LABEL IMPLEMENTATION
// -------------------------------------------------
be_union_label::be_union_label()
{}

be_union_label::be_union_label(AST_UnionLabel::UnionLabel ul, AST_Expression *v)
      :
      AST_UnionLabel(ul, v)
{
   if (v)
   {
      ostringstream os;
      v->dump(os);
      os << ends;
      label = os.str().c_str();
   }
}

DDS_StdString
be_union::UnionStreamOut(const DDS_StdString& arg, const DDS_StdString& out) const
{
   if (arg != "_union.m_void")
      return out + " << " + arg + ";";
   else
      return out + " << (const " + ScopedName() + "*)" + arg + ";";
}

DDS_StdString
be_union::UnionStreamIn(const DDS_StdString& arg, const DDS_StdString& in) const
{
   if (arg != "_union.m_void")
      return in + " >> " + arg + ";";

   return in + " >> (" + ScopedName() + "*&)" + arg + ";";
}

pbbool be_union::IsInterfaceDependant () const
{
   return m_interface_dependant;
}
