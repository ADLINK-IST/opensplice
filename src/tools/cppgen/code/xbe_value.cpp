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
#include "xbe_value.h"
#include "xbe_typedef.h"
#include "xbe_cppfwd.h"
#include "xbe_scopestack.h"
#include "xbe_genlist.h"
#include "xbe_cpptype.h"
#include "xbe_literals.h"
#include "xbe_predefined.h"
#include "xbe_enum.h"
#include "xbe_array.h"

// -------------------------------------------------
//  BE_VALUE IMPLEMENTATION
// -------------------------------------------------
IMPL_NARROW_METHODS5(be_value, be_interface, AST_Value, AST_Interface,
                     be_CodeGenerator, be_Type)
IMPL_NARROW_FROM_DECL(be_value)
IMPL_NARROW_FROM_SCOPE(be_value)

be_value::be_value()
{
   isAtModuleScope(pbfalse);
}

be_value::be_value
(
   idl_bool abstract,
   idl_bool custom,
   idl_bool truncatable,
   UTL_ScopedName *n,
   AST_Value **ih,
   long nih,
   AST_Interface **supports,
   long nsupports,
   const UTL_Pragmas &p
)
:
   AST_Decl (AST_Decl::NT_value, n, p),
   UTL_Scope (AST_Decl::NT_value, n, p),
   AST_Interface (pbtrue, n, supports, nsupports, p),
   AST_Value (abstract, custom, truncatable, n, ih, nih, supports, nsupports, p),
   be_interface (pbtrue, n, supports, nsupports, p, pbfalse),
   m_cppScope (g_feScopeStack.Top()),
   m_cppType (g_feScopeStack.Top(), *n)
{
   isAtModuleScope(pbfalse);
   InitializeTypeMap(this);
   localName = local_name()->get_string();
   enclosingScope = be_Type::EnclosingScopeString(this);
   baseClassname = localName;
}

be_value::~be_value ()
{
}

const DDS_StdString& be_value::BaseClassname()
{
   return baseClassname;
}

AST_StateMember * be_value::add_state_member(AST_StateMember *ast_member)
{
   if (AST_Value::add_state_member (ast_member))
   {
      be_state_member * member;

      if ((member = (be_state_member*)ast_member->narrow((long) & be_state_member::type_id)))
      {
         member->Initialize(this);
      }

      return ast_member;
   }

   return 0;
}

AST_Initializer *be_value::add_initializer(AST_Initializer *ast_initializer)
{
   if (AST_Value::add_initializer (ast_initializer))
   {
      be_initializer * initializer;

      if ((initializer = (be_initializer*)ast_initializer->narrow((long) & be_initializer::type_id)))
      {
         initializer->Initialize(this);
      }

      return ast_initializer;
   }

   return 0;
}

void be_value::GenerateStaticMFs (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);
   DDS_StdString ptrClass = BaseClassname () + "*";

   os << tab << "static " << ptrClass << " _downcast (DDS::ValueBase *);" << nl
      << nl;
}

void be_value::GenerateDefaultConstructor (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   os << tab << BaseClassname () << " ();" << nl;
}

void be_value::GenerateDestructor (be_ClientHeader & source)
{
   be_Tab tab(source);

   source.Stream() << tab << "virtual ~" << LocalClassName()
   << "()" << ";" << nl;
}

void be_value::GenerateCopyConstructor (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   os << tab << BaseClassname () << " (const " 
      << BaseClassname () << " &) {};" << nl;
}

void be_value::GenerateAssignmentOperator (be_ClientHeader & source)
{
   ostream & os = source.Stream ();
   be_Tab tab (source);

   os << tab << BaseClassname () << " & " << "operator = (const "
      << BaseClassname () << " &);" << nl;
}

void
be_value::InitializeTypeMap(be_Type* t)
{
   assert(t);

   idlType = t;

   AST_Type* t_ast = (AST_Type*)t->narrow((long) & AST_Type::type_id);

   assert(t_ast);

   DDS_StdString scopedName = NameToString(t_ast->name());
   DDS_StdString ptrName = scopedName + "*";

   t->TypeName(scopedName);
   t->InTypeName(ptrName);
   t->InOutTypeName(t->TypeName() + "*&");
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

void be_value::Generate (be_ClientHeader & source)
{
   DDS_StdString base = "ValueBase";

   if (!Generated())
   {
      DDS_StdString declKey = ScopedName();

      Generated(pbtrue);

      if (!be_interface_fwd::is_Generated(declKey))
      {
         // dbw 1/18/02
         // WinNT msdev has a problem with forward declaring a class
         // more than once in the same scope. This checks to see if
         // the be_CppFwdDecl has already generated a forward decl.
         pbbool needsForwardDecl = pbtrue;

         if (be_CppFwdDecl::IsAlreadyDeclared("class", LocalName(), m_cppScope))
         {
            needsForwardDecl = pbfalse;
         }

         GenerateRefAndVar
         (
            source,
            ScopedName(),
            LocalName(),
            needsForwardDecl
         );

         be_interface_fwd::Generated(declKey);
      }

      SetName (BaseClassname ());
      SetScopedClassName (Scope (baseClassname));
      if (n_value_inherits() > 0)
      {
         for (int i = 0; i < n_value_inherits(); i++)
         {
            be_value * ip;

            assert (value_inherits ()[i]);

            if ((ip = be_value::_narrow (value_inherits ()[i])))
            {
               AddParent
               (
                  new be_ClassParent
                  (
                     ip->BaseClassname (),
                     ip->Scope (ip->BaseClassname ()),
                     pbtrue,
                     pbtrue
                  )
               );
            }
         }
      }
      else
      {
         AddParent
         (
            new be_ClassParent
            (
               "ValueBase",
               "DDS::ValueBase",
               pbtrue,
               pbtrue
            )
         );
      }

      // Inheritance from object interfaces is ignored for now.

      g_cppScopeStack.Push(m_cppType);

      GenerateOpenClassDefinition (source);
      SetAccess (source, CA_PUBLIC);

      // DEFINE NESTED TYPES
      be_CppFwdDecl::GenerateAllWithinScope (source, m_cppType);
      be_CodeGenerator::Generate(source);

      GenerateVirtualMFs (source, BaseClassname());
      GenerateStateMFs (source, BaseClassname(), pbtrue);
      GenerateStaticMFs (source);
      SetAccess (source, CA_PROTECTED);
      GenerateDefaultConstructor (source);
      GenerateDestructor (source);
      GenerateStateMFs (source, BaseClassname(), pbfalse);
      GenerateCloseClassDefinition (source);
      source.Stream () << nl;

      g_cppScopeStack.Pop();
   }
}

void
be_value::Generate(be_ServerImplementation& source)
{
}

void be_value::Generate (be_ClientImplementation& source)
{
   GenerateStaticMFs(source);
   GenerateDefaultConstructor(source);
   GenerateDestructor (source);
}

void be_value::Generate (be_ServerHeader & source)
{
}

void be_value::GenerateStateMFs 
(
   be_Source& source,
   const DDS_StdString& implclassname,
   idl_bool public_access
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
      be_state_member *member;

      d = i->item();

      if ((member = (be_state_member*)
           d->narrow((long) & be_state_member::type_id)))
      {
         if (member->public_access () == public_access)
         {
            member->GenerateGetAccessor (source);
            member->GenerateSetAccessor (source);
         }
      }
   }

   delete i;

   os << nl;
}

void be_value::GenerateStaticMFs (be_ClientImplementation & source)
{
   DDS_StdString scopedName = Scope(baseClassname);
   DDS_StdString ptrClass = scopedName + "*";
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << nl;
   os << ptrClass << " " << scopedName << "::_downcast (DDS::ValueBase* v)" << nl;
   os << "{" << nl;
   tab.indent ();

   os << tab << "return dynamic_cast < " << ptrClass << "> (v);" << nl;
   tab.outdent ();
   os << "}" << nl << nl;
}

void be_value::GenerateDefaultConstructor (be_ClientImplementation &source)
{
   DDS_StdString scopedName = Scope(baseClassname);
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << nl;
   os << scopedName << "::" << baseClassname << " ()" << nl;
   os << "{" << nl;
   os << "}" << nl << nl;
}

void be_value::GenerateDestructor (be_ClientImplementation &source)
{
   DDS_StdString scopedName = Scope(baseClassname);
   ostream & os = source.Stream();
   be_Tab tab(source);

   os << nl;
   os << scopedName << "::~" << baseClassname << " ()" << nl;
   os << "{" << nl;
   os << "}" << nl << nl;
}

void be_value::GenerateRefAndVar
(
   be_ClientHeader & source,
   const DDS_StdString & scope,
   const DDS_StdString & localName,
   pbbool needsForwardDecl
)
{
   ostream& os = source.Stream ();
   be_Tab tab (source);

   // forward declare the class

   //if (not already forward declared at this scope)

   if (needsForwardDecl)
   {
      os << nl;
      os << tab << "class " << DLLMACRO << localName << ";" << nl;
   }

   os << tab << "typedef DDSValueBase_var < " << localName << "> " << localName << "_var;" << nl;
   os << tab << "typedef DDSValueBase_out < " << localName << "> " << localName << "_out;" << nl;
}

void be_value::GenerateTypedefs
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

   os << tab << "typedef " << relTypeName << DDSVarExtension
   << " " << alias.LocalName() << DDSVarExtension << ";" << nl;
   os << tab << "typedef " << relTypeName << DDSOutExtension
   << " " << alias.LocalName() << DDSOutExtension << ";" << nl;
}

be_value * be_value::_narrow (AST_Type * atype)
{
   be_value * ret = 0;

   if (atype)
   {
      ret = (be_value*)atype->narrow((long) & be_value::type_id);
   }

   return ret;
}

IMPL_NARROW_METHODS2(be_value_fwd, AST_ValueFwd, be_CodeGenerator)
IMPL_NARROW_FROM_DECL(be_value_fwd)

String_map be_value_fwd::generated(idlc_hash_str);

be_value_fwd::be_value_fwd ()
{
}

be_value_fwd::be_value_fwd
(
   idl_bool abstract,
   UTL_ScopedName *n, 
   const UTL_Pragmas &p
)
   : AST_Decl(AST_Decl::NT_value_fwd, n, p),
     AST_Type(AST_Decl::NT_value_fwd, n, p),
     AST_ValueFwd (abstract, n, p)
{
   isAtModuleScope(pbfalse);
}

be_value_fwd::~be_value_fwd ()
{
}

void be_value_fwd::Generate (be_ClientHeader& source)
{
   DDS_StdString localName = local_name()->get_string();
   DDS_StdString declKey = Scope(localName);

   if (!is_Generated(declKey))
   {
      pbbool needsForwardDecl = pbtrue;
      be_CppEnclosingScope cppScope((g_cppScopeStack.Top()));

      if ( be_CppFwdDecl::IsAlreadyDeclared("class", localName, cppScope) )
      {
         needsForwardDecl = pbfalse;
      }
      else
      {
         be_Tab tab(source);

         source.Stream () << tab << "class " << DLLMACRO << localName << ";"
                          << nl;
         be_value::GenerateRefAndVar
         (
            source,
            declKey,
            localName,
            pbfalse
         );
      }

      Generated(declKey);
   }
}

void
be_value_fwd::Generate(be_ServerImplementation& source)
{
}

void be_value_fwd::Generate (be_ClientImplementation& source)
{
}

void be_value_fwd::Generate (be_ServerHeader & source)
{
}

DDS_StdString be_value_fwd::Scope(const DDS_StdString& name)
{
   DDS_StdString ret = be_Type::EnclosingScopeString(this);

   if (ret.length())
   {
      ret += "::";
   }

   return ret + name;
}

pbbool
be_value_fwd::is_Generated(const DDS_StdString& declKey)
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
be_value_fwd::Generated(const DDS_StdString& declKey)
{
   generated[declKey] = declKey;
}

IMPL_NARROW_METHODS1(be_state_member, AST_StateMember);
IMPL_NARROW_FROM_DECL(be_state_member);

be_state_member::be_state_member ()
{
}

be_state_member::be_state_member
(
   idl_bool public_access,
   AST_Type *ft,
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
  : AST_Decl (AST_Decl::NT_state_member, n, p),
    AST_Field (AST_Decl::NT_state_member, ft, n, p),
    AST_StateMember (public_access, ft, n, p),
    fieldType (0)
{
   assert(field_type());

   if (field_type())
   {
      fieldType =
         (be_DispatchableType*)field_type()->narrow((long) & be_Type::type_id);
      assert(fieldType);
   }
}

be_state_member::~be_state_member ()
{
}

void be_state_member::GenerateAGetAccessor
(
   be_Source& source,
   const DDS_StdString& getReturn,
   pbbool isConst
)
{
   AST_Type * atype = field_type();
   ostream & os = source.Stream();
   be_Tab tab(source);

   if (be_typedef::_narrow(atype))
   {
      atype = be_typedef::_astBase(atype);
   }

   os << tab << "virtual " << getReturn << " "
      << BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this),
                                   local_name()->get_string ()) << "() ";

   if (isConst)
   {
      os << "const";
   }

   os << " = 0;";
}

void be_state_member::GenerateGetAccessor (be_Source& source)
{
   AST_Type * atype = field_type();
   DDS_StdString getReturn;
   be_array * ba;
   be_interface* bi;
   be_value* bv;
   be_Type *member_type =
      (be_Type *) field_type ()->narrow ((long) &be_Type::type_id);
   DDS_StdString type = member_type->UnionMemberTypeName ();

   // CHECK FOR TYPEDEF AND GET BASE TYPE

   if (be_typedef::_narrow(atype))
   {
      atype = be_typedef::_astBase(atype);
   }

   // NOW GENERATE GET ACCESSOR(S)
   be_predefined_type * tmp = be_predefined_type::_narrow(atype);

   if (((tmp != 0) && (tmp->pt() != AST_PredefinedType::PT_any)) || be_enum::_narrow(atype))
   {
      getReturn = BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), type);
   }
   else if (be_string::_narrow(atype))
   {
      be_string* sbt = be_string::_narrow(atype);

      if (sbt->IsWide())
         getReturn = "const DDS::WChar*";
      else
         getReturn = "const char*";
   }
   else if ((ba = be_array::_narrow(atype)))
   {
      getReturn = BE_Globals::RelativeScope     
      (
         be_Type::EnclosingScopeString(this), 
         type
      ); 
   }
   else if ((bv = be_value::_narrow (atype)))
   {
      DDS_StdString strRelScope;
      strRelScope = BE_Globals::RelativeScope
                     (
                        be_Type::EnclosingScopeString(this),
                        type
                     );

      getReturn = strRelScope + "*";
   }
   else if ((bi = be_interface::_narrow (atype)))
   {
      DDS_StdString strRelScope;
      strRelScope = BE_Globals::RelativeScope
                     (
                        be_Type::EnclosingScopeString(this),
                        type
                     );

      getReturn = strRelScope + DDSPtrExtension;
   }
   else  // struct, union, sequence, any, opaque
   {
      getReturn = BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), type) + "&";

      GenerateAGetAccessor(source, getReturn, pbfalse);

      getReturn = (DDS_StdString)"const " + BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), type) + "&";
   }

   GenerateAGetAccessor(source, getReturn, pbtrue);
}

void be_state_member::GenerateASetAccessor
(
   be_Source & source,
   const DDS_StdString & argType,
   const pbbool isConst
)
{
   AST_Type * atype = field_type();
   be_Type * btype = 0;
   ostream & os = source.Stream();
   be_Tab tab(source);
   DDS_StdString assignee("");
   DDS_StdString deleteMember("");

   // CHECK FOR TYPEDEF AND GET BASE TYPE

   if (be_typedef::_narrow(atype))
   {
      atype = be_typedef::_astBase(atype);
   }

   btype = (be_Type *)atype->narrow((long) & be_Type::type_id);
   assert(btype);

   os << tab << "virtual void " << local_name ()->get_string () << "(";

   be_Type *member_type =
      (be_Type *) field_type ()->narrow ((long) &be_Type::type_id);
   be_interface* bi;
   be_value* bv;
   DDS_StdString strRelativeScope;

   if ((bv = be_value::_narrow(atype)))
   {
      strRelativeScope = 
         BE_Globals::RelativeScope
         (
            be_Type::EnclosingScopeString(this), 
            member_type->UnionMemberTypeName()
         );

      os << strRelativeScope << "*" << " _val_)";
   }
   else if ((bi = be_interface::_narrow(atype)))
   {
      strRelativeScope = 
         BE_Globals::RelativeScope
         (
            be_Type::EnclosingScopeString(this), 
            member_type->UnionMemberTypeName()
         );

      os << strRelativeScope << DDSPtrExtension << " _val_)";
   }
   else
   {
      strRelativeScope =
         BE_Globals::RelativeScope
         (
            be_Type::EnclosingScopeString(this),
            argType
         );
                                                                                
      os << strRelativeScope << " _val_)";
   }

   os << " = 0;" << nl;
}

void be_state_member::GenerateSetAccessor (be_Source& source)
{
   ostream & os = source.Stream();
   AST_Type * atype = field_type();
   DDS_StdString setParam;
   be_interface * bi;
   be_value * bv;
   pbbool isConst = 0;
   be_Type *member_type =
      (be_Type *) field_type ()->narrow ((long) &be_Type::type_id);

   os << nl;
   // CHECK FOR TYPEDEF AND GET BASE TYPE

   if (be_typedef::_narrow(atype))
   {
      atype = be_typedef::_astBase(atype);
   }

   // THEN SET PARAMETER TYPE
   if (be_predefined_type::_narrow(atype) || be_enum::_narrow(atype)
         || be_array::_narrow(atype))
   {
      setParam = BE_Globals::RelativeScope
      (
         be_Type::EnclosingScopeString(this),
         member_type->UnionMemberTypeName ()
      );
   }
   else if ((bv = be_value::_narrow(atype)))
   {
      setParam = BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), bv->BaseClassname()) + "*";
   }
   else if (be_Type::_narrow(atype)->IsInterfaceType())
   {
      bi = be_interface::_narrow(atype);
      setParam = BE_Globals::RelativeScope(be_Type::EnclosingScopeString(this), bi->BaseClassname()) + DDSPtrExtension;
   }
   else if (be_Type::_narrow(atype)->IsOpaqueType())
   {
      setParam = (DDS_StdString)"const "
      + BE_Globals::RelativeScope
      (
         be_Type::EnclosingScopeString(this),
         member_type->UnionMemberTypeName ()
      ) + "const &";
   }
   else
   {
      setParam = (DDS_StdString)"const " 
      + BE_Globals::RelativeScope
      (
         be_Type::EnclosingScopeString(this),
         member_type->UnionMemberTypeName ()
      ) + "&";

      if (be_Type::_narrow(field_type())->IsStringType())
      {
         be_string* sbt = be_string::_narrow(atype);

         if (sbt->IsWide())
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

void be_state_member::Initialize(be_value* owner)
{
   if (fieldType)
   {
      fieldType->Initialize();
   }
}

IMPL_NARROW_METHODS1(be_initializer, AST_Initializer);
IMPL_NARROW_FROM_DECL(be_initializer);

be_initializer::be_initializer ()
{
}

be_initializer::be_initializer
(
   UTL_ScopedName *n,
   const UTL_Pragmas &p
)
  : AST_Decl (AST_Decl::NT_init, n, p),
    UTL_Scope (AST_Decl::NT_init, n, p),
    AST_Initializer (n, p)
{
}

be_initializer::~be_initializer ()
{
}

void be_initializer::Initialize(be_value* owner)
{
}

IMPL_NARROW_METHODS1(be_boxed_valuetype, AST_BoxedValue);
IMPL_NARROW_FROM_DECL(be_boxed_valuetype);

be_boxed_valuetype::be_boxed_valuetype ()
{
}

be_boxed_valuetype::be_boxed_valuetype
(
   UTL_ScopedName *n,
   AST_Type *t,
   const UTL_Pragmas &p
)
   : AST_BoxedValue (n, t, p)
{
}

be_boxed_valuetype::~be_boxed_valuetype ()
{
}

