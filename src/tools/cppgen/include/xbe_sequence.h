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
#ifndef _XBE_SEQUENCE_HH
#define _XBE_SEQUENCE_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"
#include "xbe_dispatchable.h"

class be_typedef;

class be_sequence
         :
         public virtual AST_Sequence,
         public be_CodeGenerator,
         public be_DispatchableType
{

public:

   be_sequence();
   be_sequence(AST_Expression *v, AST_Type *bt);

   unsigned long MaxSize() const;
   DDS::Boolean IsBounded() const;
   const DDS_StdString & BaseTypeName() const;
   void OsOpSignature(const DDS_StdString& osopsignature);
   void IsOpSignature(const DDS_StdString& isopsignature);
   DDS::Boolean SetName(const DDS_StdString& scope, const DDS_StdString& name);
   void putter( ostream & os,
                be_Tab & tab,
                const DDS_StdString & seqptr,
                unsigned long uid);
   void getter( ostream & os,
                be_Tab & tab,
                const DDS_StdString & seqptr,
                unsigned long uid);

   // BE_SEQUENCE STATICS
   static be_sequence * _narrow(AST_Type * type);

   // BE_TYPE_MAP VIRTUALS
   virtual void InitializeTypeMap(be_Type*);
   virtual DDS::Boolean IsFixedLength() const;
   virtual DDS::Boolean IsFixedLengthPrimitiveType() const;

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& source);
   virtual void Generate(be_ClientImplementation& source);
   virtual void Generate(be_ServerHeader&)
   {}

   virtual void Generate(be_ServerImplementation&)
   {}

   virtual void isAtModuleScope(bool is_at_module);
   virtual bool isAtModuleScope() const;

   // BE_TYPE VIRTUALS
   virtual void Initialize();
   virtual void GenerateType(be_ClientHeader& source);
   virtual void GenerateTypedefs(
      const DDS_StdString &scope,
      const be_typedef & alias,
      be_ClientHeader& source);
   virtual void GeneratePutGetOps(be_ClientHeader& source);
   virtual void GenerateStreamOps(be_ClientHeader& source);
   virtual DDS::Boolean IsPrimitiveType() const
   {
      return pbfalse;
   }

   virtual DDS::Boolean IsStructuredType() const
   {
      return pbtrue;
   }

   virtual DDS::Boolean IsStringType() const
   {
      return pbfalse;
   }

   virtual DDS::Boolean IsArrayType() const
   {
      return pbfalse;
   }

   virtual DDS::Boolean IsSequenceType() const
   {
      return pbtrue;
   }

   virtual DDS::Boolean IsInterfaceType() const
   {
      return pbfalse;
   }

   virtual DDS::Boolean IsReturnedByVar () const
   {
      return pbtrue;
   }

   virtual pbbool IsInterfaceDependant () const;

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

   virtual void GenerateGlobalTypedef(be_ClientHeader &source);

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

   // BE_DISPATCHABLETYPE VIRTUALS
   virtual be_DispatchableType::en_HowStoredInDispatcher
   HowStoredInDispatcher(const be_ArgumentDirection& direction) const;

   DEF_NARROW_METHODS4(be_sequence, AST_Sequence, be_CodeGenerator,
                       be_DispatchableType, be_Type);
   DEF_NARROW_FROM_DECL(be_sequence);

private:

   static String_map generatedSequences;
   static String_map generatedSequencesInstantiations;

   DDS_StdString baseTypeName;
   DDS::Boolean initialized;
   unsigned long maxSize;
   DDS::Boolean anonymous;
   DDS::Boolean deferred;
   DDS_StdString allocater;
   DDS_StdString releaser;
   DDS_StdString assigner;
   DDS_StdString isOpSignature;
   DDS_StdString osOpSignature;
   DDS::Boolean isPrimitiveSeq;
   DDS::Boolean isStringSeq;
   DDS::Boolean isInterfaceSeq;
   DDS::Boolean isValueSeq;
   be_Type * baseType;

   void GenerateAuxTypes(be_ClientHeader& source);
   void GenerateSequence(be_ClientHeader& source);
   void GenerateSequence (be_ClientImplementation & source);
   unsigned long isRecursive();
   void init_type(const DDS_StdString& scope, const DDS_StdString& name);
};

inline unsigned long
be_sequence::MaxSize() const
{
   return maxSize;
}

inline DDS::Boolean
be_sequence::IsBounded() const
{
   return (DDS::Boolean)(maxSize != 0);
}

inline const DDS_StdString&
be_sequence::BaseTypeName() const
{
   return baseTypeName;
}

inline void
be_sequence::OsOpSignature(const DDS_StdString& osopsignature)
{
   osOpSignature = osopsignature;
}

inline void
be_sequence::IsOpSignature(const DDS_StdString& isopsignature)
{
   isOpSignature = isopsignature;
}

#endif
