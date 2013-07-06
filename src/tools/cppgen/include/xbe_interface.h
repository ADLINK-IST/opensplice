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
#ifndef _XBE_INTERFACE_HH
#define _XBE_INTERFACE_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"
#include "xbe_dispatchable.h"
#include "xbe_opnameset.h"

class be_typedef;

class be_interface;

class be_interface :
   public virtual AST_Interface,
   public be_ClassGenerator,
   public be_DispatchableType
{

public:

   typedef DDS::Boolean Boolean;
   typedef be_OpNameSet::Be_OpMap Be_OpMap;

   be_interface();
   be_interface
   (
      idl_bool local,
      UTL_ScopedName *n,
      AST_Interface **ih,
      long nih,
      const UTL_Pragmas &p,
      idl_bool forward_declare = pbtrue
   );

   ~be_interface ();

   const DDS_StdString & ImplClassname();
   const DDS_StdString & StubClassname();
   const DDS_StdString & BaseClassname();

   // BE_INTERFACE STATICS

   static void GeneratePtrAndRef
   (
      be_ClientHeader& source,
      const DDS_StdString& scope,
      const DDS_StdString& localName,
      pbbool needsForwardDecl = pbtrue
   );
   static void GenerateVarOutAndMgr(
      be_ClientHeader& source,
      const DDS_StdString& scope,
      const DDS_StdString& localName);
   static be_interface * _narrow(AST_Type * atype);

   // BE_TYPE_MAP VIRTUALS
   virtual DDS::Boolean IsFixedLength() const;
   virtual DDS::Boolean IsFixedLengthPrimitiveType() const;

   // BE_TYPE VIRTUALS
   virtual inline void Initialize()
   {}

   virtual void GenerateType(be_ClientHeader& source);
   virtual void GenerateTypedefs(
      const DDS_StdString &scope,
      const be_typedef & alias,
      be_ClientHeader& source);
   virtual void GenerateFwdDecls(be_ClientHeader &source);
   virtual Boolean IsPrimitiveType() const
   {
      return pbfalse;
   }

   virtual Boolean IsStructuredType() const
   {
      return pbfalse;
   }

   virtual Boolean IsArrayType() const
   {
      return pbfalse;
   }

   virtual Boolean IsStringType() const
   {
      return pbfalse;
   }

   virtual Boolean IsSequenceType() const
   {
      return pbfalse;
   }

   virtual Boolean IsInterfaceType() const
   {
      return pbtrue;
   }

   virtual pbbool IsReturnedByVar () const
   {
      return pbtrue;
   }

   virtual pbbool IsInterfaceDependant () const
   {
      return pbtrue;
   }

   virtual DDS_StdString Allocater(const DDS_StdString&) const;
   virtual DDS_StdString Initializer(const DDS_StdString&, VarType) const;
   virtual DDS_StdString InRequestArgumentDeclaration(
      be_Type& btype,
      const DDS_StdString&,
      VarType vt);
   virtual DDS_StdString Releaser(const DDS_StdString&) const;
   virtual DDS_StdString Assigner(
      const DDS_StdString&,
      const DDS_StdString&) const;
   virtual DDS_StdString Duplicater(
      const DDS_StdString&,
      const DDS_StdString&,
      const DDS_StdString&,
      const pbbool) const;
   virtual DDS_StdString NullReturnArg();
   virtual DDS_StdString SyncStreamOut(
      const DDS_StdString& arg,
      const DDS_StdString& out,
      VarType vt) const;
   virtual DDS_StdString UnionStreamOut(
      const DDS_StdString&,
      const DDS_StdString&) const;
   virtual DDS_StdString UnionStreamIn(
      const DDS_StdString&,
      const DDS_StdString&) const;
   virtual void GeneratePutGetOps(be_ClientHeader& source)
   {}

   virtual void GenerateStreamOps (be_ClientHeader&);
   //
   // NEW MARSHALING CALLS
   //
   virtual DDS::Boolean is_core_marshaled();
   virtual DDS::Boolean declare_for_stub(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg,
      const DDS_StdString & stubScope,
      VarType vt);
   virtual DDS::Boolean declare_for_struct_put(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean declare_for_struct_get(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean declare_for_union_put(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean declare_for_union_get(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean make_get_param_for_stub(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & argname,
      VarType vt) const;
   virtual DDS::Boolean make_put_param_for_stub(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & argname,
      VarType vt) const;
   virtual DDS::Boolean make_put_param_for_struct(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean make_get_param_for_struct(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean make_put_param_for_union(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual DDS::Boolean make_get_param_for_union(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual ostream & put_for_struct(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual ostream & get_for_struct(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual ostream & put_for_union(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual ostream & get_for_union(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
   virtual ostream & put_for_sequence(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg,
      const DDS_StdString & index,
      unsigned long uid);
   virtual ostream & get_for_sequence(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg,
      const DDS_StdString & index,
      unsigned long uid);
   virtual ostream & put_for_array(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg,
      const DDS_StdString & index,
      unsigned long uid);
   virtual ostream & get_for_array(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg,
      const DDS_StdString & index,
      unsigned long uid);
   virtual void generate_writer(
      be_Source & source);
   virtual void generate_reader(
      be_Source & source);
   virtual void generate_tc_ctor_val(
      be_Source & source);
   virtual void generate_tc_dtor_val
   (
       be_Source & source,
       pbbool isCounted
   );
   virtual void generate_tc_assign_val(
      be_Source & source);
   virtual DDS_StdString kind_string();
   virtual DDS::ULong get_elem_size();
   virtual DDS::ULong get_elem_alignment();

   // BE_CLASSGENERATOR VIRTUALS
   virtual void GenerateClassDeclarations(
      be_ClientHeader& source);

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& clientHeader);
   virtual void Generate(be_ClientImplementation& clientImpl);
   virtual void Generate(be_ServerHeader& serverHeader);
   virtual void Generate(be_ServerImplementation& serverImpl);

   // AST_INTERFACE VIRTUALS
   virtual AST_Operation * add_operation(AST_Operation* op);
   virtual AST_Attribute * add_attribute(AST_Attribute* at);

   // CALLED BY BE_MODULE
   void GeneratePOATypedef(
      be_ServerHeader& source);

   virtual void GenerateGlobalTypedef(be_ClientHeader &source);

   DEF_NARROW_METHODS4(be_interface, AST_Interface, be_CodeGenerator,
                       be_DispatchableType, be_Type);
   DEF_NARROW_FROM_DECL(be_interface);
   DEF_NARROW_FROM_SCOPE(be_interface);

   // PUBLIC METHODS FOR THE NEW WORLD ORDER

   virtual be_CppType CppTypeWhenSequenceMember() const;

   // BE_DISPATCHABLETYPE VIRTUALS
   virtual be_DispatchableType::en_HowStoredInDispatcher
   HowStoredInDispatcher(const be_ArgumentDirection& direction) const;
   void GenerateVirtualMFs(be_Source& source, const DDS_StdString& implclassname);

private:

   DDS_StdString baseClassname;
   DDS_StdString stubClassname;
   DDS_StdString m_implClassname;
   DDS_StdString m_dirstubClassname;
   DDS_StdString m_mgrName;
   DDS_StdString m_stubFactoryName;
   DDS_StdString m_colocFactoryName;
   be_OpNameSet m_ops;
   be_OpNameSet m_lops;
   bool m_allOpsLoaded; // has LoadAllOps() been called?
   const be_CppEnclosingScope m_cppScope; // scope enclosing this interface
   const be_CppEnclosingScope m_cppType;  // the scope that this interface is

   unsigned short CountDispatchTableEntries();
   void LoadAllOps ();
   void LoadOpTable (DDS_StdString & mdiImpl, be_OpNameSet & ops, pbbool all);
   void GenerateInheritanceInit(ostream& os, String_map &ancestors, const char* args);
   void GenerateStaticMFs(be_ClientHeader& source);
   void GenerateObjectVirtuals(be_ClientHeader& source);
   void GenerateVarClass(be_ClientHeader& source);
   void GenerateDefaultConstructor(be_ClientHeader& source);
   void GenerateDestructor(be_ClientHeader& source);
   void GenerateCopyConstructor(be_ClientHeader& source);
   void GenerateAssignmentOperator(be_ClientHeader& source);
   void GenerateStubDefinition(be_ClientHeader& source);
   void DefineStubOperations(be_ClientHeader& source);
   void DefineDirectStubOperations(be_ServerHeader& source);
   void GenerateStaticMFs(be_ClientImplementation& source);
   void GenerateObjectVirtuals(be_ClientImplementation& source);
   void GenerateDefaultConstructor(be_ClientImplementation& source);
   void GenerateDestructor(be_ClientImplementation& source);
   void GenerateCopyConstructor(be_ClientImplementation& source);
   void GenerateImpureImplementations(be_ClientImplementation& source);
   void GenerateStubImplementations(be_ClientImplementation& source);
   void GenerateDirectStubImplImplementations(be_ServerImplementation& source);
   void ImplementStubOperations(be_ClientImplementation& source);
   void GenerateWrite(be_ClientImplementation& source);
   void GenerateRead(be_ClientImplementation& source);
   void GenerateTie(be_ServerHeader& source);
#if defined(DDS_TIE_HEADER)

   void GenerateTieConstructor(be_ServerTieHeader& source,
                               DDS_StdString& tmplt,
                               const char* signature,
                               const char* uninheritedInits);
   void DefineTieOperations(be_ServerTieHeader& source);
#else

   void GenerateTieConstructor(be_ServerHeader& source,
                               DDS_StdString& tmplt,
                               const char* signature,
                               const char* uninheritedInits);
   void DefineTieOperations(be_ServerHeader& source);
#endif

   void DeclareStaticDispatchers(
      be_ServerHeader& source,
      const char * servantClass);
   void GenerateImplDefinition(be_ServerHeader &source);
   void GenerateDirectImplDefinition(be_ServerHeader &source);

   // void GenerateDispatchTableEntries(be_ServerImplementation& source,
   //                                   const DDS_StdString& implName);
   void GenerateInvoke(be_ServerImplementation& source);
//   void GenerateObjectRequestDispatchers(be_ServerImplementation& source,const DDS_StdString& implbasename);
   void GenerateImplImplementations(be_ServerImplementation& source);
   void GenerateRequestDispatchers(be_ServerImplementation& source,
                                   const DDS_StdString& implbasename);
   void GenerateLocalIsA(be_ServerImplementation& source);
//   void GenerateObjectRequestDispatchers(be_ServerImplementation& source,
//                                         const DDS_StdString& implbasename);

protected:

   virtual void InitializeTypeMap(be_Type*);
};


class be_interface_fwd
         :
         public virtual AST_InterfaceFwd,
         public be_CodeGenerator
{

public:

   static DDS::Boolean is_Generated(const DDS_StdString& declKey);
   static void Generated(const DDS_StdString& declKey);
   static DDS::Boolean is_FwdDeclared(const DDS_StdString& declKey);
   static void FwdDeclared(const DDS_StdString& declKey);
   static DDS::Boolean isTypecodeGenerated (const DDS_StdString & declKey);
   static void TypecodeGenerated (const DDS_StdString & declKey);

   be_interface_fwd();
   be_interface_fwd(idl_bool local, 
                    UTL_ScopedName *n,
                    const UTL_Pragmas &p);

   DDS_StdString Scope(const DDS_StdString& name);

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& baseHeader);
   virtual void Generate(be_ClientImplementation&);
   virtual void Generate(be_ServerHeader&);
   virtual void Generate(be_ServerImplementation&);

   DEF_NARROW_METHODS2(be_interface_fwd, AST_InterfaceFwd, be_CodeGenerator);
   DEF_NARROW_FROM_DECL(be_interface_fwd);

private:

   static String_map declared;
   static String_map generated;
   static String_map typecodeGenerated;

   DDS_StdString enclosingScope;
   DDS_StdString localName;
};

inline const DDS_StdString&
be_interface::ImplClassname()
{
   return m_implClassname;
}

inline const DDS_StdString&
be_interface::StubClassname()
{
   return stubClassname;
}

inline const DDS_StdString&
be_interface::BaseClassname()
{
   return baseClassname;
}

inline DDS::Boolean
be_interface::IsFixedLength() const
{
   return FALSE;
}

inline DDS::Boolean
be_interface::IsFixedLengthPrimitiveType() const
{
   return FALSE;
}

inline DDS_StdString
be_interface::kind_string()
{
   return "DDS::tk_objref";
}

inline DDS_StdString
be_interface_fwd::Scope(const DDS_StdString& name)
{
   DDS_StdString ret = enclosingScope;

   if (ret.length())
   {
      ret += "::";
   }

   return ret + name;
}

#endif
