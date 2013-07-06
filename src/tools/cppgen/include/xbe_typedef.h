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
#ifndef _XBE_TYPEDEF_HH
#define _XBE_TYPEDEF_HH

#include "idl.h"
#include "xbe_dispatchable.h"

class be_ClientHeader;
class be_ClientImplementation;
class be_ServerHeader;
class be_ServerImplementation;

#include "xbe_codegen.h"
#include "xbe_type.h"

class be_typedef
         :
         public virtual AST_Typedef,
         public be_CodeGenerator,
         public virtual be_DispatchableType
{

public:

   be_typedef();
   be_typedef(AST_Type *bt, UTL_ScopedName *n, const UTL_Pragmas &p);

   // BE TYPEDEF STATICS
   static be_typedef * _narrow(AST_Type * type);
   static AST_Type * _astBase(AST_Type * at);
   static be_Type * _beBase(AST_Type * at);

   be_Type * get_base_be_type() const;

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& baseHeader);
   virtual void Generate(be_ClientImplementation&)
   {}

   virtual void Generate(be_ServerHeader&)
   {}

   virtual void Generate(be_ServerImplementation&)
   {}

   // BE_TYPE_MAP VIRTUALS
   virtual inline void Initialize();
   virtual void InitializeTypeMap(be_Type*);
   virtual DDS::Boolean IsFixedLength() const;
   virtual DDS::Boolean IsFixedLengthPrimitiveType() const;

   // BE_TYPE VIRTUALS
   virtual void GenerateType(
      be_ClientHeader& source)
   {
      if (!m_generated)
         Generate(source);
   }

   virtual void GenerateTypedefs(
      const DDS_StdString &scope,
      const be_typedef & alias,
      be_ClientHeader& source);
   virtual void GeneratePutGetOps(be_ClientHeader& source)
{}

   virtual void GenerateStreamOps(be_ClientHeader& source);
   virtual DDS::Boolean IsPrimitiveType() const;
   virtual DDS::Boolean IsEnumeratedType() const;
   virtual DDS::Boolean IsStructuredType() const;
   virtual DDS::Boolean IsStringType() const;
   virtual DDS::Boolean IsArrayType() const;
   virtual DDS::Boolean IsSequenceType() const;
   virtual DDS::Boolean IsInterfaceType() const;
   virtual DDS::Boolean IsValueType() const;
   virtual DDS::Boolean IsReturnedByVar () const;
   virtual DDS::Boolean IsExceptionType() const;
   virtual DDS::Boolean IsInterfaceDependant () const;
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
      const DDS_StdString& out, VarType vt) const;
   virtual DDS_StdString SyncStreamIn(
      const DDS_StdString&arg,
      const DDS_StdString &in, VarType vt) const;
   virtual DDS_StdString StructStreamIn(
      const DDS_StdString& arg,
      const DDS_StdString& in) const;
   virtual DDS_StdString StructStreamOut(
      const DDS_StdString& arg,
      const DDS_StdString& out) const;
   virtual DDS_StdString UnionStreamIn(
      const DDS_StdString& arg,
      const DDS_StdString& in) const;
   virtual DDS_StdString UnionStreamOut(
      const DDS_StdString& arg,
      const DDS_StdString& out) const;

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
   virtual void generate_tc_put_val(
      be_Source & source);
   virtual void generate_tc_get_val(
      be_Source & source);
   virtual void generate_tc_assign_val(
      be_Source & source);
   virtual DDS_StdString kind_string();
   virtual DDS::ULong get_elem_size();
   virtual DDS::ULong get_elem_alignment();

   // PUBLIC METHODS FOR THE NEW WORLD ORDER

   virtual be_CppType CppTypeWhenSequenceMember() const;

   // BE_DISPATCHABLETYPE VIRTUALS
   virtual be_DispatchableType::en_HowStoredInDispatcher
   HowStoredInDispatcher(const be_ArgumentDirection& direction) const;

   virtual void
   InitializeInDispatcher(
      ostream& os,
      be_Tab& tab,
      const be_CppName& argName,
      const be_ArgumentDirection& direction) const;

   DEF_NARROW_METHODS4(be_typedef, AST_Typedef, be_CodeGenerator,
                       be_DispatchableType, be_Type);
   DEF_NARROW_FROM_DECL(be_typedef);

private:

   DDS::Boolean m_generateBase;
   DDS::Boolean m_generated;
   be_Type * m_baseBeType;
};

#endif
