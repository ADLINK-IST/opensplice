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
#include "xbe_root.h"
#include "xbe_operation.h"
#include "xbe_attribute.h"
#include "xbe_interface.h"
#include "xbe_typedef.h"
#include "xbe_cppfwd.h"
#include "xbe_scopestack.h"
#include "xbe_genlist.h"
#include "xbe_cpptype.h"
#include "xbe_literals.h"

// -------------------------------------------------
//  BE_INTERFACE IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS4(be_interface, AST_Interface, be_CodeGenerator,
                     be_DispatchableType, be_Type)
IMPL_NARROW_FROM_DECL(be_interface)
IMPL_NARROW_FROM_SCOPE(be_interface)

be_interface::be_interface()
{
   isAtModuleScope(pbfalse);
}

be_interface::be_interface
(
   idl_bool local,
   UTL_ScopedName* n,
   AST_Interface** ih,
   long nih,
   const UTL_Pragmas &p,
   idl_bool forward_declare
)
:
   AST_Decl (AST_Decl::NT_interface, n, p),
   UTL_Scope (AST_Decl::NT_interface, n, p),
   AST_Interface (local ? I_TRUE : I_FALSE, n, ih, nih, p),
   m_allOpsLoaded (false),
   m_cppScope (g_feScopeStack.Top()),
   m_cppType (g_feScopeStack.Top(), *n)
{
   isAtModuleScope(pbfalse);
   DDS_StdString barScopedName = NameToString(name(), "_");

   localName = local_name()->get_string();
   enclosingScope = be_Type::EnclosingScopeString(this);
   baseClassname = localName;
   stubClassname = baseClassname + DDSStubExtension;
   m_stubFactoryName = baseClassname + "Factory";
   m_colocFactoryName = NoColons(Scope(baseClassname)) +
                        "ColocatedFactory";
   m_mgrName = baseClassname + DDSMgrExtension;
   m_implClassname = DDSPOAImplPrefix + Scope(localName);
   m_dirstubClassname = m_implClassname + "_direct_stub";

   // initialize typecode

   if (this->local() != I_TRUE)
   {
      m_tc_ctor_val = (DDS_StdString) barScopedName + "_ctor";
      m_tc_dtor_val = (DDS_StdString) barScopedName + "_dtor";
      m_tc_put_val = (DDS_StdString) barScopedName + "_put";
      m_tc_get_val = (DDS_StdString) barScopedName + "_get";
      m_tc_assign_val = (DDS_StdString) barScopedName +
      "_copy";
   }
   else
   {
      m_tc_ctor_val = (DDS_StdString) "0";
      m_tc_dtor_val = (DDS_StdString) "0";
      m_tc_put_val = (DDS_StdString) "0";
      m_tc_get_val = (DDS_StdString) "0";
      m_tc_assign_val = (DDS_StdString) "0";
   }

   m_any_op_id = barScopedName;
   m_typecode->kind = DDS::tk_objref;
   m_typecode->id = get_decl_pragmas().get_repositoryID()->get_string();
   m_typecode->name_of_type = localName;

   InitializeTypeMap(this);

   if (!imported())
   {
      if (!be_interface_fwd::is_FwdDeclared(Scope(localName)))
      {
         be_root::AddFwdDecls(*this);
         be_interface_fwd::FwdDeclared(Scope(localName));
      }

      if (forward_declare)
      {
         be_CppFwdDecl::Add(be_CppFwdDecl::INTERFACE, this, m_cppScope);
      }
   }
}

be_interface::~be_interface ()
{
   be_root::RemoveFwdDecls(*this);
   be_CppFwdDecl::Remove (this);
}

AST_Operation*
be_interface::add_operation(AST_Operation * ast_op)
{
   if (AST_Interface::add_operation(ast_op))
   {
      be_operation * op;

      if ((op = (be_operation*)ast_op->narrow((long) & be_operation::type_id)))
      {
         op->Initialize(this);
      }

      return ast_op;
   }

   return 0;
}

AST_Attribute*
be_interface::add_attribute(AST_Attribute * ast_at)
{
   if (AST_Interface::add_attribute(ast_at))
   {
      be_attribute * at;

      if ((at = (be_attribute*)ast_at->narrow((long) & be_attribute::type_id)))
      {
         at->Initialize(this, Scope(baseClassname));
      }

      return ast_at;
   }

   return 0;
}

void be_interface::GenerateStaticMFs (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   DDS_StdString ptrClass = BaseClassname () + DDSPtrExtension;
   DDS_StdString varClass = BaseClassname () + DDSVarExtension;

   // typedefs for _var and _ptr

   os << tab << "typedef " << ptrClass << " _ptr_type;" << nl;
   os << tab << "typedef " << varClass << " _var_type;" << nl << nl;

   // _duplicate

   os << tab << "static " << ptrClass << " _duplicate (" << ptrClass
   << " obj);" << nl;

   // _is_a on servants, _local_is_a on local objects

   os << tab << "DDS::Boolean ";
   if (local ())
   {
      os << "_local";
   }
   os << "_is_a (const char * id);" << nl << nl;

   // _narrow

   os << tab << "static " << ptrClass << " _narrow (DDS::Object"
      << DDSPtrExtension << " obj" << XBE_Ev::arg (XBE_ENV_ARGN)
      << ");" << nl;

   // _unchecked_narrow

   os << tab << "static " << ptrClass << " _unchecked_narrow (DDS::Object"
      << DDSPtrExtension << " obj" << XBE_Ev::arg (XBE_ENV_ARGN)
      << ");" << nl;

   // _nil

   os << tab << "static " << ptrClass << " _nil () { return 0; }" << nl;

   // _repository_id

   UTL_String * repID = get_decl_pragmas().get_repositoryID();
   assert(repID);

   os << tab << "static const char * _local_id;";
   os << nl;

   // Stub factory

   if (!local())
   {
      os << tab << "static void _assert_stub (" << localName
         << DDSPtrExtension << " & obj);" << nl;
   }
}

void be_interface::GenerateDefaultConstructor (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   os << tab << BaseClassname () << " () {};" << nl;
}

void be_interface::GenerateDestructor (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   os << tab << "~" << BaseClassname () << " () {};" << nl;
}

void be_interface::GenerateCopyConstructor (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   os << tab << BaseClassname () << " (const "
      << BaseClassname () << " &);" << nl;
}

void be_interface::GenerateAssignmentOperator (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   os << tab << BaseClassname () << " & " << "operator = (const "
      << BaseClassname () << " &);" << nl;
}

void be_interface::GenerateVirtualMFs
(
   be_Source& source,
   const DDS_StdString& implclassname
)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   UTL_ScopeActiveIterator * i;
   AST_Decl * d;

   os << nl;

   i = new UTL_ScopeActiveIterator(this, UTL_Scope::IK_decls);

   for (; !(i->is_done()); i->next())
   {
      be_operation * op;
      be_attribute * at;

      d = i->item();

      if ((op = (be_operation*)d->narrow((long) & be_operation::type_id)))
      {
         op->GenerateVirtual (source, implclassname);
      }
      else if ((at = (be_attribute*)d->narrow((long) & be_attribute::type_id)))
      {
         at->GenerateVirtual (source, implclassname);
      }
   }

   delete i;

   os << nl;
}

void be_interface::GenerateStaticMFs (be_ClientImplementation & source)
{
   DDS_StdString scopedName = Scope(baseClassname);
   DDS_StdString ptrClass = scopedName + DDSPtrExtension;
   DDS_StdString varClass = scopedName + DDSVarExtension;
   ostream & os = source.Stream();
   be_Tab tab(source);

   // Static init of local object rep id string

   UTL_String * repID = get_decl_pragmas().get_repositoryID();
   assert(repID);
   os << tab << "const char * " << scopedName << "::_local_id = " << "\"" << repID->get_string() << "\";" << nl;

   // _duplicate

   os << nl;
   os << ptrClass << " " << scopedName << "::_duplicate ("
      << ptrClass << " p)" << nl ;
   os << "{" << nl;
   tab.indent ();

   os << tab << "if (p) p->m_count++;" << nl;
   os << tab << "return p;" << nl;
   tab.outdent ();
   os << "}" << nl << nl;

   // _is_a

   os << tab << "DDS::Boolean" << " " << scopedName << "::";
   if (local ())
   {
      os << "_local";
   }
   os << "_is_a (const char * _id)" << nl;
   os << tab << "{" << nl;
   os << tab << "   if (strcmp (_id, " << scopedName
      << "::_local_id) == 0)" << nl;
   os << tab << "   {" << nl;
   os << tab << "      return true;" << nl;
   os << tab << "   }" << nl;
   os << tab << nl;
   tab.outdent ();

   GenerateHierachySearch (source);

   os << tab << "   return false;" << nl;
   os << tab << "}" << nl << nl;

   // _narrow

   os << tab << ptrClass << " " << scopedName
      << "::_narrow (DDS::Object_ptr p"
      << XBE_Ev::arg (XBE_ENV_ARGN) << ")" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   os << tab << ptrClass << " result = NULL;" << nl;
   os << tab << "if (p && p->_is_a (" << scopedName << "::_local_id"
      << XBE_Ev::arg (XBE_ENV_VARN) << "))" << nl;
   os << tab << "{" << nl;
   tab.indent ();

   if (local ())
   {
      os << tab  << "result = dynamic_cast < " << ptrClass << "> (p);" << nl;
      os << tab  << "if (result) result->m_count++;" << nl;
   }
   else
   {
      if (BE_Globals::collocated_direct)
      {
         os << tab << "PortableServer::Servant c = NULL;" << nl << nl;
         os << tab << "if ((c =" << nl;
         tab.indent ();
         os << tab
            << "  ((::PortableServer::Servant) (p->_collocated (p)))" << nl;
         os << tab << " )" << nl;
         os << tab << " != NULL)" << nl;
         tab.outdent ();
         os << tab << "{" << nl;
         tab.indent ();
         os << tab << "result = new " << m_dirstubClassname
            << " (p, dynamic_cast < " << m_implClassname << " *> (c));" << nl;
         tab.outdent ();
         os << tab << "}" << nl;
         os << tab << "else" << nl;
         os << tab << "{" << nl;
         tab.indent ();
         os << tab << "result = new " << baseClassname
            << "_stub (p);" << nl;
         tab.outdent ();
         os << tab << "}" << nl;
      }
      else
      {
         os << tab << "result = new " << baseClassname
            << "_stub (p);" << nl;
      }
   }
   tab.outdent ();
   os << tab << "}" << nl;
   os << tab << "return result;" << nl;
   tab.outdent ();
   os << "}" << nl << nl;

   // _unchecked_narrow

   os << tab << ptrClass << " " << scopedName
      << "::_unchecked_narrow (DDS::Object_ptr p"
      << XBE_Ev::arg (XBE_ENV_ARGN) << ")" << nl;
   os << tab << "{" << nl;
   tab.indent ();
   os << tab << ptrClass << " result;" << nl;

   if (local())
   {
      os << tab  << "result = dynamic_cast < " << ptrClass << "> (p);" << nl;
      os << tab  << "if (result) result->m_count++;" << nl;
   }
   else
   {
      os << tab << "result = new " << baseClassname
         << "_stub (p);" << nl;
   }
   os << tab << "return result;" << nl;
   tab.outdent ();
   os << "}" << nl << nl;
}

void be_interface::GenerateDefaultConstructor (be_ClientImplementation &)
{
}

void be_interface::GenerateDestructor (be_ClientImplementation &)
{
}

void be_interface::GenerateCopyConstructor (be_ClientImplementation &)
{
}

void be_interface::GenerateTypedefs
(
   const DDS_StdString &scope,
   const be_typedef& alias,
   be_ClientHeader& source
)
{
   ostream & os = source.Stream();
   be_Tab tab(source);
   DDS_StdString relTypeName = BE_Globals::RelativeScope(scope, typeName);

   // TYPEDEF THE _PTR AND REF and VAR TYPEDEFS
   os << tab << "typedef " << relTypeName << " "
   << alias.LocalName() << ";" << nl;

   os << tab << "typedef " << relTypeName << DDSPtrExtension
   << " " << alias.LocalName() << DDSPtrExtension << ";" << nl;

   os << tab << "typedef " << relTypeName << DDSVarExtension
   << " " << alias.LocalName() << DDSVarExtension << ";" << nl;

   os << tab << "typedef " << relTypeName << DDSOutExtension
   << " " << alias.LocalName() << DDSOutExtension << ";" << nl;
   os << tab << "typedef " << relTypeName << DDSMgrExtension
   << " " << alias.LocalName() << DDSMgrExtension << ";" << nl;
   os << tab << "typedef " << relTypeName << DDSStubExtension
   << " " << alias.LocalName() << DDSStubExtension << ";" << nl;
}

void
be_interface::InitializeTypeMap(be_Type* t)
{
   assert(t);

   idlType = t;

   AST_Type* t_ast = (AST_Type*)t->narrow((long) & AST_Type::type_id);

   assert(t_ast);

   DDS_StdString scopedName = NameToString(t_ast->name());
   DDS_StdString ptrName = scopedName + DDSPtrExtension;

   t->TypeName(scopedName);
   t->InTypeName(ptrName);
   t->InOutTypeName(t->TypeName() + "_ptr&");
   t->OutTypeName(t->TypeName() + DDSOutExtension);
   t->ReturnTypeName(ptrName);
   t->DMFAdtMemberTypeName(ptrName);
   t->UnionMemberTypeName(t->TypeName());
   t->StructMemberTypeName(t->TypeName() + DDSVarExtension);
   t->SequenceMemberTypeName(ptrName);
   t->StreamOpTypeName(ptrName);
   t->IStreamOpTypeName(ptrName + "&");

   t->VarSignature(VT_InParam, ptrName, VT_NonConst, VT_Var, VT_NonReference);
   t->VarSignature(VT_InOutParam, t->InOutTypeName(), VT_NonConst, VT_Var, VT_NonReference);
   t->VarSignature(VT_OutParam, t->OutTypeName(), VT_NonConst, VT_Var, VT_NonReference);
   t->VarSignature(VT_Return, ptrName, VT_NonConst, VT_Var, VT_NonReference);

   t->TypeCodeTypeName(BE_Globals::TCPrefix + t->LocalName());
   DDS_StdString scopedbar = NameToString(t_ast->name(), "_");
   t->MetaTypeTypeName(BE_Globals::MTPrefix + scopedbar);
   t->TypeCodeBaseName(BE_Globals::TCBasePrefix + scopedbar);
   t->TypeCodeRepName(BE_Globals::TCRepPrefix + scopedbar);
}

void be_interface::GenerateType (be_ClientHeader & source)
{
   be_interface::GeneratePtrAndRef (source, ScopedName(), LocalName());
}

DDS_StdString
be_interface::Allocater(const DDS_StdString& arg) const
{
   return Initializer(arg, VT_OutParam);
}

DDS_StdString
be_interface::Initializer(const DDS_StdString& arg, VarType vt) const
{
   DDS_StdString ret = arg + " = 0;";

   return ret;
}

DDS_StdString
be_interface::InRequestArgumentDeclaration(be_Type& btype, const DDS_StdString& arg, VarType vt)
{
   DDS_StdString ret = btype.TypeName() + "_var " + arg + ";";

   return ret;
}

DDS_StdString
be_interface::Releaser(const DDS_StdString& arg) const
{
   return BE_Globals::CorbaScope("release") + "(" + arg + "); ";
}

DDS_StdString
be_interface::Assigner(const DDS_StdString& arg, const DDS_StdString& val) const
{
   DDS_StdString ret = arg + " = " + val + ";";

   return ret;
}

DDS_StdString be_interface::Duplicater
(
   const DDS_StdString & arg,
   const DDS_StdString & val,
   const DDS_StdString & currentScope,
   const pbbool isConst
) const
{
   DDS_StdString relativeName = BE_Globals::RelativeScope(currentScope, typeName);
   DDS_StdString ret = arg + " = " + relativeName + "::_duplicate (" + val + ");";

   return ret;
}


DDS_StdString
be_interface::NullReturnArg()
{
   return (DDS_StdString)"(" + Scope(baseClassname)
          + DDSPtrExtension + ")0";
}

DDS_StdString
be_interface::SyncStreamOut( const DDS_StdString& arg,
                             const DDS_StdString& out,
                             VarType /*vt*/) const
{
   return out + " <<  (" + Scope(baseClassname)
          + DDSPtrExtension + ")" + arg + ";";
}

void be_interface::Generate (be_ClientHeader& source)
{
   if (BE_Globals::ignore_interfaces)
   {
      return;
   }

   DDS_StdString main_class;
   if (local())
   {
      main_class = "DDS::LocalObject";
   }
   else
   {
      main_class = "DDS::Stub";
   }
   const DDS_StdString stub = main_class;
   const DDS_StdString objRefPropScope = "DDSObjectRefPropertyScope";
   DDS_StdString ptrClass = BaseClassname() + DDSPtrExtension;
   ostream & os = source.Stream();
   be_Tab tab(source);

   if (!Generated())
   {
      DDS_StdString declKey = ScopedName();

      be_root::AddAnyOps(*this);
      be_root::AddPutGetOps(*this);
      be_root::AddStreamOps(*this);
      be_root::AddTypedef(*this);

      Generated(pbtrue);

      // generate interface _ptr class

      if (!be_interface_fwd::is_Generated(declKey))
      {
         // dbw 1/18/02
         // WinNT msdev has a problem with forward declaring a class
         // more than once in the same scope. This checks to see if
         // the be_CppFwdDecl has already generated a forward decl.
         pbbool needsForwardDecl = pbtrue;

         if ( be_CppFwdDecl::IsAlreadyDeclared("class", LocalName(), m_cppScope) )
         {
            needsForwardDecl = pbfalse;
         }

         be_interface::GeneratePtrAndRef(source, ScopedName(), LocalName(), needsForwardDecl);
         GenerateVarOutAndMgr(source, ScopedName(), LocalName());

         be_interface_fwd::Generated(declKey);
      }

      // Add typecode
      if (!be_interface_fwd::isTypecodeGenerated (declKey))
      {
         be_interface_fwd::TypecodeGenerated (declKey);
         be_root::AddTypecode (*this);
      }

      // have to set inheritance here because the constructor is
      // called for forward decls as well.
      SetName(BaseClassname());

      SetScopedClassName(Scope(baseClassname));

      if (n_inherits())
      {
         bool derived_localobject = false;
         for (int i = 0; i < n_inherits(); i++)
         {
            be_interface * ip;

            assert(inherits()[i]);

            if ((ip = be_interface::_narrow(inherits()[i])))
            {
               AddParent
               (
                  new be_ClassParent
                  (
                     ip->BaseClassname (),
                     ip->Scope (ip->BaseClassname ()),
                     ip->local () ? I_TRUE : I_FALSE,
                     true
                  )
               );
               if (ip->local ())
               {
                  derived_localobject = true;
               }
            }
         }
         if (!derived_localobject && local ())
         {
            AddParent (new be_ClassParent (stub, stub, true, true));
         }
      }
      else
      {
         AddParent (new be_ClassParent (stub, stub, local () ? I_TRUE : I_FALSE, true));
      }

      // initialize hashtable for client stub generation

      LoadAllOps();

      g_cppScopeStack.Push(m_cppType);

      GenerateOpenClassDefinition(source);

      SetAccess(source, CA_PUBLIC);

      // DEFINE NESTED TYPES
      be_CppFwdDecl::GenerateAllWithinScope (source, m_cppType);

      be_CodeGenerator::Generate(source);

      GenerateClassDeclarations(source);

      // DDS OBJECT STUFF
      GenerateStaticMFs(source);

      GenerateObjectVirtuals(source);

      // _this

      os << tab << localName << DDSPtrExtension << " _this () { return this; }" << nl;

      // Operation and attribute virtuals

      GenerateVirtualMFs (source, BaseClassname());
      SetAccess (source, CA_PROTECTED);
      GenerateDefaultConstructor (source);
      GenerateDestructor (source);

      // Constructor and assignment restrictions

      SetAccess (source, CA_PRIVATE);
      GenerateCopyConstructor (source);
      GenerateAssignmentOperator (source);

      // END BASE CLASS
      GenerateCloseClassDefinition(source);

      g_cppScopeStack.Pop();

      // Stub
      if (local() != I_TRUE)
      {
         GenerateStubDefinition (source);
      }

      os << nl;

      be_root::GenerateDependants
         (source, SequenceMemberTypeName(), EnclosingScope());

   }
}

void be_interface::GenerateStreamOps (be_ClientHeader& source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   // insertion

   os << tab << DLLMACRO << "inline void" << nl
      << "IOP::put (DDS::Codec::OutStream& os, const "
      << ScopedName() << "_ptr v" << XBE_Ev::arg (XBE_ENV_ARGN)
      << ")" << nl;
   os << "{" << nl;
   tab.indent ();
   os << tab << "DDS::Codec::Param putArg = ";
   os << "{ " << Scope(TypeCodeTypeName()) << ", ";
   os << "v->get_ior(), DDS::PARAM_IN };" << nl;
   os << tab << "os.put (&putArg, 1" << XBE_Ev::arg (XBE_ENV_VARN)
      << ");" << nl;
   tab.outdent ();
   os << "}" << nl << nl;

   // extraction

   os << DLLMACRO << "inline void" << nl
      << "IOP::get(DDS::Codec::InStream& is, "
      << ScopedName() << "_ptr v" << XBE_Ev::arg (XBE_ENV_ARGN)
      << ")" << nl;
   os << "{" << nl;
   tab.indent();
   os << tab << "DDS::Codec::Param getArg = ";
   os << "{ " << Scope(TypeCodeTypeName()) << ", ";

   // eCPP896 this fixes leaks in null ref tests gen code
   // however may need to discriminate between OUT and INOUT
   //os << "&v->get_ior(), DDS::PARAM_OUT };" << nl;

   os << "&v->get_ior(), DDS::PARAM_INOUT };" << nl;

   os << tab << "is.get (&getArg, 1" << XBE_Ev::arg (XBE_ENV_VARN)
      << ");" << nl;
   tab.outdent();
   os << "}" << nl << nl;
}

void
be_interface::Generate(be_ServerImplementation& source)
{
   if (BE_Globals::ignore_interfaces)
   {
      return;
   }

   // ostream & os = source.Stream();
   // be_Tab tab(source);
   DDS_StdString factRegistrar = "DDSColocated_";

   factRegistrar += NoColons(Scope(baseClassname)) + "_init";

   be_CodeGenerator::Generate(source);

   LoadAllOps();

   // IMPL IMPLEMENTATION

   GenerateImplImplementations(source);

   if (BE_Globals::collocated_direct)
   {
      GenerateDirectStubImplImplementations(source);
   }
}

void be_interface::Generate (be_ClientImplementation& source)
{
   if (BE_Globals::ignore_interfaces)
   {
      return;
   }

   ostream & os = source.Stream();
   be_Tab tab (source);

   if (!local())
   {
      be_CodeGenerator::Generate(source);
   }

   // DDS OBJECT STUFF
   GenerateStaticMFs(source);
   GenerateObjectVirtuals(source);

   // DDS OBJECT BEHAVIOR
   GenerateDefaultConstructor(source);
   GenerateDestructor(source);
   GenerateCopyConstructor(source);

   //
   // STATIC STUB FACTORY
   //
   if (!local())
   {
      os << tab << "void " << Scope(localName)
         << "::_assert_stub (" << localName
         << DDSPtrExtension << " & obj )" << nl;
      os << tab << "{" << nl;
      tab.indent ();
      os << tab << "if (DDS::is_nil (obj)) obj = new " << localName << DDSStubExtension << " ();" << nl;
      tab.outdent ();
      os << tab << "}" << nl;

      GenerateStubImplementations(source);
   }
}

void be_interface::Generate (be_ServerHeader & source)
{
   if (BE_Globals::ignore_interfaces)
   {
      return;
   }

   if (!local())
   {
      GenerateImplDefinition (source);

      if (BE_Globals::collocated_direct)
      {
         GenerateDirectImplDefinition (source);
      }
   }
}

void be_interface::GenerateClassDeclarations (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   DDS_StdString ptrName = localName + DDSPtrExtension;

   if (!local())
   {
      os << tab << "typedef " << ptrName << " (*" << m_stubFactoryName
         << ") (IOP::IOR_ptr ior);" << nl;
   }
}

be_interface * be_interface::_narrow (AST_Type * atype)
{
   be_interface * ret = 0;

   if (atype)
   {
      ret = (be_interface*)atype->narrow((long) & be_interface::type_id);
   }

   return ret;
}

void be_interface::GenerateFwdDecls (be_ClientHeader &source)
{
}

void be_interface::GenerateGlobalTypedef (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   if (local() != I_TRUE)
   {
      os << tab << "typedef " << Scope (stubClassname)
         << " DDS_IDLC_" << TypedefName () << "_stub" << ";" << nl;
   }
}

// -------------------------------------------------
//  BE_INTERFACE_FWD IMPLEMENTATION
// -------------------------------------------------
String_map be_interface_fwd::declared(idlc_hash_str);
String_map be_interface_fwd::generated(idlc_hash_str);
String_map be_interface_fwd::typecodeGenerated(idlc_hash_str);

IMPL_NARROW_METHODS2(be_interface_fwd, AST_InterfaceFwd, be_CodeGenerator)
IMPL_NARROW_FROM_DECL(be_interface_fwd)

be_interface_fwd::be_interface_fwd()
{
   isAtModuleScope(pbfalse);
}

be_interface_fwd::be_interface_fwd
(
   idl_bool local,
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
:
   AST_Decl (AST_Decl::NT_interface_fwd, n, p),
   AST_InterfaceFwd (local ? true : false, n, p)
{
   isAtModuleScope(pbfalse);
   enclosingScope = be_Type::EnclosingScopeString(this);
   localName = local_name()->get_string();

   if (!is_FwdDeclared(Scope(localName)))
   {
      FwdDeclared(Scope(localName));
   }
}

void be_interface_fwd::Generate (be_ClientHeader& source)
{
   if (BE_Globals::ignore_interfaces)
   {
      return;
   }

   DDS_StdString declKey = Scope(localName);

   if (!is_Generated(declKey))
   {
      // FORWARD DECLARE THE CLASS
      // generate forward declaration
      // dbw 1/18/02
      // WinNT msdev has a problem with forward declaring a class
      // more than once in the same scope. This checks to see if
      // the be_CppFwdDecl has already generated a forward decl.
      pbbool needsForwardDecl = pbtrue;
      be_CppEnclosingScope cppScope((g_cppScopeStack.Top()));

      if ( be_CppFwdDecl::IsAlreadyDeclared("class", localName, cppScope) )
      {
         needsForwardDecl = pbfalse;
      }

      be_interface::GeneratePtrAndRef(source, Scope(localName), localName, needsForwardDecl);
      be_interface::GenerateVarOutAndMgr(source, Scope(localName), localName);

      Generated(declKey);

      // do the type code or it will not be declared init'd
      // in the right order
   }
   if (!isTypecodeGenerated (declKey))
   {
      AST_Interface *ai = full_definition();
      assert(ai);
      be_interface *bi = (be_interface *)ai->narrow((long) & be_interface::type_id);
      assert(bi);
      TypecodeGenerated (declKey);
      be_root::AddTypecode(*bi);
   }
}

void
be_interface_fwd::Generate(be_ClientImplementation&)
{}

void
be_interface_fwd::Generate(be_ServerHeader&)
{}

void
be_interface_fwd::Generate(be_ServerImplementation&)
{}

pbbool
be_interface_fwd::is_Generated(const DDS_StdString& declKey)
{
   // this checks to see if key  (declKey) is in map

   if ((generated.find(declKey)) == generated.end())
   {
      return pbfalse;
   }
   else
   {
      return pbtrue;
   }
}

void
be_interface_fwd::Generated(const DDS_StdString& declKey)
{
   generated[declKey] = declKey;
}

pbbool
be_interface_fwd::is_FwdDeclared(const DDS_StdString& declKey)
{
   // this checks to see if key  (declKey) is in map

   if (declared.find(declKey) == declared.end())
   {
      return pbfalse;
   }
   else
   {
      return pbtrue;
   }
}

void
be_interface_fwd::FwdDeclared(const DDS_StdString& declKey)
{
   declared[declKey] = declKey;
}

pbbool be_interface_fwd::isTypecodeGenerated (const DDS_StdString & declKey)
{
   if (typecodeGenerated.find (declKey) == typecodeGenerated.end ())
   {
      return pbfalse;
   }
   return pbtrue;
}

void be_interface_fwd::TypecodeGenerated (const DDS_StdString & declKey)
{
   typecodeGenerated[declKey] = declKey;
}

DDS_StdString
be_interface::UnionStreamOut(const DDS_StdString& arg, const DDS_StdString& out) const
{
   return out + " << (" + UnionMemberTypeName() + "_ptr)" + arg + ";";
}

DDS_StdString
be_interface::UnionStreamIn(const DDS_StdString& arg, const DDS_StdString& in) const
{
   return in + " >> (" + UnionMemberTypeName() + "*&)" + arg + ";";
}

DDS::Boolean be_interface::is_core_marshaled ()
{
   return true;
}

DDS::Boolean be_interface::declare_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & stubScope,
   VarType vt
)
{
   DDS_StdString stub = BE_Globals::RelativeScope
      (stubScope, Scope (baseClassname) + "_stub");

   if (vt == VT_InOutParam)
   {
     os << tab << "if (" << arg << " == 0) { " << arg << " = new " << stub
     << " (); }" << nl;
   }
   else if (vt == VT_OutParam)
   {
      os << tab << arg << " = new " << stub << " ();" << nl;
   }
   else if (vt == VT_Return)
   {
      os << tab << ScopedName () << "_var _ret_ = new ::"
         << ScopedName() << "_stub ();" << nl;
   }

   return true;
}

DDS::Boolean
be_interface::declare_for_struct_put(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return false;
}

DDS::Boolean
be_interface::declare_for_struct_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   os << tab << ScopedName() << "::_assert_stub("
   << sptr << "->" << fld << ");" << nl;

   return false;
}

DDS::Boolean
be_interface::declare_for_union_put(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   return declare_for_struct_put(os, tab, sptr, fld, uid);
}

DDS::Boolean
be_interface::declare_for_union_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   os << tab << ScopedName() << "::_assert_stub(("
   << ScopedName() << "*&)" << sptr << "->" << fld << ");"
   << nl;

   return true;
}

DDS::Boolean be_interface::make_get_param_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & argname,
   VarType vt
) const
{
   DDS::Boolean ret = false;
   const char * pt;

   pt = (vt == VT_InOutParam) ? "DDS::PARAM_INOUT" : "DDS::PARAM_OUT";

   if (vt == VT_InOutParam || vt == VT_OutParam || vt == VT_Return)
   {
      os << tab << "{ " << Scope (TypeCodeTypeName ()) << ", &";
      os << argname << "->get_ior (), " << pt ;
      os << " }";
      ret = true;
   }

   return ret;
}

DDS::Boolean be_interface::make_put_param_for_stub
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & argname,
   VarType vt
) const
{
   DDS::Boolean ret = false;

   if (vt == VT_InParam)
   {
      os << tab;
      os << "{ " << Scope(TypeCodeTypeName()) << ", ";
      os << "(" << argname << ") ? " << argname << "->get_ior() : 0, ";
      os << "DDS::PARAM_IN ";
      os << "}";
      ret = true;
   }
   else if (VT_InOutParam)
   {
      os << tab;
      os << "{ " << Scope(TypeCodeTypeName()) << ", ";
      os << argname << "->get_ior(), DDS::PARAM_IN ";
      os << "}";
      ret = true;
   }

   /*
   if (vt == VT_InParam || vt == VT_InOutParam)
   {
      os << tab;
      os << "{ " << Scope(TypeCodeTypeName()) << ", ";
      os << argname << "->get_ior(), DDS::PARAM_IN ";
      os << "}";
      ret = true;
   }
   */

   return ret;
}

DDS::Boolean
be_interface::make_put_param_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   os << " { " << Scope(TypeCodeTypeName()) << ", (" << sptr << "->"
   << fld << ") ? (void*)" << "((" << ScopedName() << "*)(" << sptr << "->"
   << fld << "))->get_ior() : 0, DDS::PARAM_IN " << "}";

   return true;
}

DDS::Boolean
be_interface::make_get_param_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   os << "{ " << Scope(TypeCodeTypeName()) << ", (void*)&";
   os << "((" << ScopedName() << "*)(" << sptr << "->"

   // eCPP896 this fixes leaks in null ref tests gen code
   // however may need to discriminate between OUT and INOUT
   //   << fld << "))->get_ior(), DDS::PARAM_OUT ";
   << fld << "))->get_ior(), DDS::PARAM_INOUT ";
   os << "}";

   return true;
}

DDS::Boolean
be_interface::make_put_param_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   os << " { " << Scope(TypeCodeTypeName()) << ", (" << sptr << "->"
   << fld << ") ? (void*)" << "((" << ScopedName() << "*)(" << sptr << "->"
   << fld << "))->get_ior() : 0, DDS::PARAM_IN " << "}";

   return true;
}

DDS::Boolean
be_interface::make_get_param_for_union(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid)
{
   os << "{ " << Scope(TypeCodeTypeName()) << ", (void*)&";
   os << "((" << ScopedName() << "*)(" << sptr << "->"

   // eCPP896 this fixes leaks in null ref tests gen code
   // however may need to discriminate between OUT and INOUT
   //   << fld << "))->get_ior(), DDS::PARAM_OUT ";
   << fld << "))->get_ior(), DDS::PARAM_INOUT ";
   os << "}";

   return true;
}

ostream & be_interface::put_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "{ " << Scope(TypeCodeTypeName()) << ", ";
   os << sptr << "->" << fld << "->get_ior(), DDS::PARAM_IN ";
   os << "}";

   return os;
}

ostream & be_interface::get_for_struct
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << ScopedName() << "::_assert_stub ("
      << sptr << "->" << fld << ".m_ptr);" << nl;
   os << tab << "is.cdr_get(" << sptr << "->" << fld
      << "->get_ior()" << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_interface::put_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << "os.cdr_put(((" << Scope(localName) << "*)"
      << sptr << "->" << fld << ")->get_ior()"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_interface::get_for_union
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   const DDS_StdString & fld,
   unsigned long uid
)
{
   os << tab << ScopedName() << "::_assert_stub("
      << sptr << "->" << fld << ".m_ptr);" << nl;
   os << tab << "is.cdr_get(((" << Scope(localName) << "*)"
      << sptr << "->" << fld << ")->get_ior()"
      << XBE_Ev::arg (XBE_ENV_VARN) << ");" << nl;

   return os;
}

ostream & be_interface::put_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   os << tab << "DDS::Codec::Param putArg = "
      << "{ " << Scope(TypeCodeTypeName()) << ", (" << arg << "[" << index
      << "]) ? " << arg << "[" << index << "]->get_ior() : 0, DDS::PARAM_IN };"
      << nl;
   os <<  tab << "os.put (&putArg, 1" << XBE_Ev::arg (XBE_ENV_VARN)
      << ");" << nl;

   return os;
}

ostream & be_interface::get_for_sequence
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid
)
{
   os << tab.indent();
   os << tab << ScopedName() << "::_assert_stub("
   << arg << "[" << index << "]);" << nl;
   os << tab << "DDS::Codec::Param getArg = "
   << "{ " << Scope(TypeCodeTypeName()) << ", &"

   // eCPP896 this fixes leaks in null ref tests gen code
   // This should not have been IN at all but OUT in the first place
   // however may need to discriminate between OUT and INOUT
   //   << arg << "[" << index << "]->get_ior(), DDS::PARAM_IN };"
   << arg << "[" << index << "]->get_ior(), DDS::PARAM_INOUT };"

   << nl;
   os << tab << "is.get (&getArg, 1" << XBE_Ev::arg (XBE_ENV_VARN)
      << ");" << nl;
   os << tab << arg << "[" << index << "]->unbind ();" << nl;
   os << tab.outdent();

   return os;
}

ostream &
be_interface::put_for_array(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   return put_for_sequence(os, tab, arg, index, uid);
}

ostream &
be_interface::get_for_array(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & arg,
   const DDS_StdString & index,
   unsigned long uid)
{
   return get_for_sequence(os, tab, arg, index, uid);
}

void
be_interface::generate_writer(
   be_Source & source)
{}

void
be_interface::generate_reader(
   be_Source & source)
{}


void be_interface::generate_tc_ctor_val (be_Source & source)
{
   ostream & os = source.Stream ();

   // declare ctor body

   os << "static void * " << m_tc_ctor_val << " ()" << nl;
   os << "{" << nl;

   //
   // This must return a DDS::Object_ptr, since the object returned
   // by ctor_val is passed to the get_val typecode method by the any library.
   // You must pass a DDS::Object_ptr through the void* to get_val, since it
   // has no knowledge about the specific interfaces and it uses the
   // DDS::Object_ptr to set the ior read in.
   //

   os << "   return (DDS::Object_ptr) new " << ScopedName () << DDSStubExtension
      << " ();" << nl;
   os << "}" << nl << nl;
}

void be_interface::generate_tc_dtor_val
(
   be_Source & source,
   pbbool isCounted
)
{
   ostream & os = source.Stream ();

   // declare dtor body

   os << "static void " << m_tc_dtor_val << " (void * arg)" << nl;
   os << "{" << nl;

   //
   // This method is called by the any library when the value in the any
   // is released. The any assumes the void* that it had stored, points
   // to a DDS::Object, therefore we must cast it to such while
   // releasing it.
   //
   os << "   DDS::release ((DDS::Object_ptr) arg);" << nl;
   os << "}" << nl << nl;
}

void be_interface::generate_tc_assign_val (be_Source & source)
{
   ostream & os = source.Stream ();

   //
   // declare assign body
   //
   os << "static void " << m_tc_assign_val << " (void * dest, void * src)" << nl;
   os << "{" << nl;

   //
   // This method is called by the any library, when assigning one any's
   // value to another. The void* ptr MUST be DDS::Object_ptr. The entire
   // code for getting and putting object references depends on the generic
   // DDS::Object since any's have no knowledge of specific interfaces
   // in the idl.
   //

   os << "   ((DDS::Object_ptr)dest)->set_ior ((("
      << "DDS::Object_ptr)src)->get_ior ());" << nl;

   os << "}" << nl << nl;
}

be_DispatchableType::en_HowStoredInDispatcher
be_interface::HowStoredInDispatcher(const be_ArgumentDirection&) const
{
   return STORED_IN_IOR_VAR;
}

DDS::ULong be_interface::get_elem_size ()
{
   return 0;
}

DDS::ULong be_interface::get_elem_alignment ()
{
   return 4;
}

be_CppType be_interface::CppTypeWhenSequenceMember () const
{
   return be_CppType(TypeName() + DDSPtrExtension);
}
