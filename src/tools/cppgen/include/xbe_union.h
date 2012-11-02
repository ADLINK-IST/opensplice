/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef _XBE_UNION_HH
#define _XBE_UNION_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"
#include "xbe_dispatchable.h"
#include "xbe_cppscope.h"

class be_union_branch;
class be_typedef;

class be_union
:
   public virtual AST_Union,
   public be_CodeGenerator,
   public virtual be_DispatchableType
{

public:

   typedef DDS::Boolean Boolean;

   be_union();
   be_union(AST_ConcreteType *dt,
            UTL_ScopedName *n,
            const UTL_Pragmas &p);

   void putter( ostream & os,
                be_Tab & tab,
                const DDS_StdString & seqptr,
                unsigned long uid);
   void getter( ostream & os,
                be_Tab & tab,
                const DDS_StdString & seqptr,
                unsigned long uid);

   inline void
   IsFixedLength(Boolean val)
   {
      m_isFixedLength = val;
   }
   
   inline void
   IsFixedLengthPrimtiveType()
   {

   }

   inline bool
   HasExplicitDefault()
   {
      return defaultIndex != NO_DEFAULT_INDEX;
   }

   const DDS_StdString
   DiscTypeCodeName() const
   {
      DDS_StdString tc = discType->Scope(discType->TypeCodeTypeName());
      return tc;
   }

   const DDS_StdString&
   DiscTypeName() const
   {
      return discType->TypeName();
   }

   void Append(be_union_branch& branch);

   // BE_TYPE_MAP VIRTUALS
   inline virtual Boolean
   IsFixedLength() const
   {
      return m_isFixedLength;
   }

   inline virtual Boolean
   IsFixedLengthPrimitiveType() const
   {
      return FALSE;
   }

   virtual void InitializeTypeMap(be_Type*);

   static be_union * _narrow(AST_Type * atype);

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& source);
   virtual void Generate(be_ClientImplementation&);
   virtual void Generate(be_ServerHeader&)
   {}

   virtual void Generate(be_ServerImplementation&)
   {}

   virtual void isAtModuleScope(bool is_at_module);
   virtual bool isAtModuleScope() const;

   // BE_TYPE VIRTUALS
   virtual inline void Initialize()
   {}

   virtual void GenerateType(be_ClientHeader& source)
   {}

   virtual void GenerateTypedefs(
      const DDS_StdString &scope,
      const be_typedef & alias,
      be_ClientHeader& source);
   virtual Boolean IsPrimitiveType() const
   {
      return pbfalse;
   }

   virtual Boolean IsStructuredType() const
   {
      return pbtrue;
   }

   virtual Boolean IsStringType() const
   {
      return pbfalse;
   }

   virtual Boolean IsArrayType() const
   {
      return pbfalse;
   }

   virtual Boolean IsSequenceType() const
   {
      return pbfalse;
   }

   virtual Boolean IsInterfaceType() const
   {
      return pbfalse;
   }

   virtual pbbool IsReturnedByVar () const
   {
      return ! m_isFixedLength;
   }

   virtual DDS::Boolean IsInterfaceDependant () const;

   virtual DDS_StdString Allocater(const DDS_StdString&) const;
   virtual DDS_StdString Initializer(const DDS_StdString&, VarType) const;
   virtual DDS_StdString Releaser(const DDS_StdString&) const;
   virtual DDS_StdString Assigner(
      const DDS_StdString&,
      const DDS_StdString&) const;
   virtual DDS_StdString Duplicater(
      const DDS_StdString&,
      const DDS_StdString&,
      const DDS_StdString&,
      const pbbool) const;
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
   virtual DDS_StdString NullReturnArg();

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
   virtual void generate_tc_ctor_val(
      be_Source & source);
   virtual void generate_tc_dtor_val
   (
      be_Source & source,
      pbbool isCounted
   );
   virtual void generate_tc_put_val(
      be_Source & source);
   virtual void generate_tc_get_val(
      be_Source & source);
   virtual void generate_tc_assign_val(
      be_Source & source);
   virtual DDS_StdString kind_string();
   virtual DDS::ULong get_elem_size();
   virtual DDS::ULong get_elem_alignment();

   // AST_STRUCTURE VIRTUALS
   virtual AST_UnionBranch * add_union_branch(AST_UnionBranch *b);

   // BE_DISPATCHABLETYPE VIRTUALS
   virtual be_DispatchableType::en_HowStoredInDispatcher
   HowStoredInDispatcher(const be_ArgumentDirection& direction) const;

   DEF_NARROW_METHODS4(be_union, AST_Union, be_CodeGenerator,
                       be_DispatchableType, be_Type);
   DEF_NARROW_FROM_DECL(be_union);
   DEF_NARROW_FROM_SCOPE(be_union);

private:

   Boolean m_isFixedLength;
   DDS_StdString nullArg;
   DDS_StdString osOpSignature;
   DDS_StdString isOpSignature;
   be_union_branch * defaultBranch;
   be_Type * discType;
   unsigned long defaultValue;
   const be_CppEnclosingScope m_cppScope;
   const be_CppEnclosingScope m_cppType;
   long defaultIndex;
   pbbool m_interface_dependant;

   static const long NO_DEFAULT_INDEX; // = 0xffffffff as value of defaultIndex,
   // means that the union has no default branch

   TList<be_union_branch *> branches;

   void GenerateOpenClassDefinition(be_ClientHeader& source);
   void GenerateMembers(be_ClientHeader& source);
   void GenerateCopyMF(be_ClientHeader& source);
   void GenerateDefaultConstructor(be_ClientHeader& source);
   void GenerateCopyConstructor(be_ClientHeader& source);
   void GenerateMemcpyCopyConstructor(be_ClientHeader& source);
   void GenerateDestructor(be_ClientHeader& source);
   void GenerateDeleteMember(be_ClientHeader& source);
   void GenerateAssignmentOperator(be_ClientHeader& source);
   void GenerateAccessors(be_ClientHeader& source);
   void GenerateCloseClassDefinition(be_ClientHeader& source);
   void GenerateUnion(be_ClientHeader& source);
   void GenerateAuxTypes(be_ClientHeader& source);
};


class be_union_branch
         :
         public virtual AST_UnionBranch,
         public be_TypeMap
{

public:

   typedef DDS::Boolean Boolean;

   be_union_branch();
   be_union_branch(AST_UnionLabel *lab, AST_Type *ft, UTL_ScopedName *n,
                   const UTL_Pragmas &p);

   const DDS_StdString&
   Label() const
   {
      return branchlabel;
   }

   void
   Label(const DDS_StdString& label)
   {
      branchlabel = label;
   }

   const DDS_StdString&
   BranchType() const
   {
      return branchType;
   }

   const be_Type*
   Type() const
   {
      return type;
   }

   inline DDS_StdString
   LocalName()
   {
      assert (local_name());
      return BE_Globals::KeywordToLabel(local_name()->get_string());
   }

   unsigned long be_Value()
   {
      return be_value;
   }

   void be_Value(unsigned long val)
   {
      be_value = val;
   }

   long ast_Value();         // The sun ast default value may conflict
   // with other values, so always use the
   // be_Value for code generation.
   Boolean IsDefault();
   DDS_StdString PrivateName();
   void Initialize();
   DDS_StdString LabelString(AST_UnionLabel* label);
   void GenerateSetAccessor(be_ClientHeader& source);
   void GenerateGetAccessor(be_ClientHeader& source);

   // BE_TYPE_MAP VIRTUALS
   virtual void InitializeTypeMap(be_Type*);
   virtual Boolean IsFixedLength() const;
   virtual Boolean IsFixedLengthPrimitiveType() const;

   DEF_NARROW_METHODS2(be_union_branch, AST_UnionBranch, be_TypeMap);
   DEF_NARROW_FROM_DECL(be_union_branch);

private:

   DDS_StdString branchlabel;
   DDS_StdString branchType;
   unsigned long be_value;
   be_Type * type;

   void GenerateASetAccessor (be_ClientHeader&, const DDS_StdString&, const pbbool);
   void GenerateAGetAccessor (be_ClientHeader&, const DDS_StdString&, Boolean);
};


class be_union_label
         :
         public virtual AST_UnionLabel
{

public:

   typedef DDS::Boolean Boolean;

public:

   be_union_label();
   be_union_label(AST_UnionLabel::UnionLabel ul, AST_Expression *v);

   const DDS_StdString&
   Label()
   {
      return label;
   }

private:

   DDS_StdString label;
};

#endif
