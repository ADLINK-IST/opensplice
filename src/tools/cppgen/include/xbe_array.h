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
#ifndef _XBE_ARRAY_HH
#define _XBE_ARRAY_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"
#include "xbe_dispatchable.h"

class be_typedef;

class be_array
         :
         public virtual AST_Array,
         public be_CodeGenerator,
         public virtual be_DispatchableType
{

public:

   typedef DDS::Boolean Boolean;

   be_array();
   be_array(UTL_ScopedName *n, unsigned long ndims, UTL_ExprList *dims);

   Boolean
   isAnonymous() const
   {
      return anonymous;
   }

   const DDS_StdString&
   ArrayType() const
   {
      return arrayType;
   }

   const DDS_StdString&
   SliceType() const
   {
      return sliceType;
   }

   const DDS_StdString&
   BaseTypeName() const
   {
      return baseTypeName;
   }

   const DDS_StdString & SliceTypeName () const
   {
      return sliceTypeName;
   }

   unsigned long
   ArraySize() const
   {
      return arraySize;
   }

   unsigned long
   MatrixSize() const
   {
      return matrixSize;
   }

   Boolean SetName(
      const DDS_StdString& scope,
      const DDS_StdString& name
   );
   DDS_StdString DimExpr(int i);
   void SetAccess(
      be_ClientHeader& source,
      const DDS_StdString& access
   );
   void GenerateStreamTypes(be_ClientHeader& source);

   void putter( ostream & os,
                be_Tab & tab,
                const DDS_StdString & seqptr,
                unsigned long uid);
   void getter( ostream & os,
                be_Tab & tab,
                const DDS_StdString & seqptr,
                unsigned long uid);

   Boolean is_primitive_array();

   // BE_ARRAY STATICS
   static be_array * _narrow(AST_Type * type);
   static be_array * _narrow_from_alias(AST_Type * type);

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& baseHeader);
   virtual void Generate(be_ClientImplementation& clientImplementation);
   virtual void Generate(be_ServerHeader&)
   {}

   virtual void Generate(be_ServerImplementation&)
   {}

   virtual void GeneratePutGetOps (be_ClientHeader & source) {};
   virtual void GenerateGlobal (be_ClientHeader& clientHeader);

   virtual void isAtModuleScope(bool is_at_module);
   virtual bool isAtModuleScope() const;

   // BE_TYPE_MAP VIRTUALS
   virtual void InitializeTypeMap(be_Type*);
   virtual Boolean IsFixedLength() const;
   virtual Boolean IsFixedLengthPrimitiveType() const;

   // BE_TYPE VIRTUALS
   virtual inline void Initialize();
   virtual void GenerateType(be_ClientHeader& source)
   {
      Generate(source);
   }

   virtual void GenerateTypedefs(const DDS_StdString &scope, const be_typedef& alias, be_ClientHeader& source);

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
      return pbtrue;
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
      return pbtrue;
   }

   virtual DDS_StdString Allocater(const DDS_StdString&) const;
   virtual DDS_StdString Initializer(const DDS_StdString&, VarType) const;
   virtual DDS_StdString InRequestArgumentDeclaration(
      be_Type& btype,
      const DDS_StdString&,
      VarType vt);
   virtual DDS_StdString Releaser(
      const DDS_StdString&) const;
   virtual DDS_StdString Assigner(
      const DDS_StdString&,
      const DDS_StdString&) const;
   virtual DDS_StdString Duplicater
   (
      const DDS_StdString &,
      const DDS_StdString &,
      const DDS_StdString &,
      const pbbool
   ) const;
   virtual DDS_StdString NullReturnArg();
   virtual DDS_StdString SyncStreamOut(
      const DDS_StdString& arg,
      const DDS_StdString& out,
      VarType vt) const;
   virtual DDS_StdString SyncStreamIn(
      const DDS_StdString& arg,
      const DDS_StdString& out,
      VarType vt) const;
   virtual DDS_StdString StructStreamOut(
      const DDS_StdString& arg,
      const DDS_StdString& out) const;
   virtual DDS_StdString StructStreamIn(
      const DDS_StdString& arg,
      const DDS_StdString& in) const;

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
   virtual void generate_tc_ctor_val(
      be_Source & source);
   virtual void generate_tc_dtor_val
   (
       be_Source & source,
       pbbool isCounted
   );
   virtual void generate_tc_get_val(
      be_Source & source);
   virtual void generate_tc_put_val(
      be_Source & source);
   virtual void generate_tc_assign_val(
      be_Source & source);
   virtual DDS_StdString kind_string();
   virtual DDS::ULong get_elem_size();
   virtual DDS::ULong get_elem_alignment();

   // BE_DISPATCHABLETYPE VIRTUALS
   virtual be_DispatchableType::en_HowStoredInDispatcher
   HowStoredInDispatcher(const be_ArgumentDirection& direction) const;

   virtual void      // code to initialize a variable, comes
   InitializeInDispatcher(   // right after declaration
      ostream& os,
      be_Tab& tab,
      const be_CppName& argName,
      const be_ArgumentDirection& direction) const;

   DEF_NARROW_METHODS4(be_array, AST_Array, be_CodeGenerator,
                       be_DispatchableType, be_Type);
   DEF_NARROW_FROM_DECL(be_array);

private:

   static String_map generatedArrays;

   DDS_StdString forAnyName;
   DDS_StdString arrayType;
   DDS_StdString sliceType;
   DDS_StdString baseTypeName;
   DDS_StdString sliceTypeName;
   DDS_StdString allocater;
   DDS_StdString copier;
   DDS_StdString releaser;
   DDS_StdString assigner;
   Boolean initialized;
   Boolean anonymous;
   unsigned long arraySize;
   unsigned long matrixSize;
   Boolean isPrimitiveArray;
   be_Type * baseType;
   const be_CppEnclosingScope m_cppScope;

   // PRIVATE HELPERS

   void GenerateAuxTypes(be_ClientHeader& source);
   void GenerateAuxTypes(be_ClientImplementation& source);
   void SetUpTypeCode(const unsigned int, ProtoTypeCode *);

   DDS_StdString CppScoped(const DDS_StdString& identifier) const;

   void NilOutArray(ostream& os, be_Tab& tab, const be_CppName& argName) const;
};

#endif
