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
#ifndef _XBE_PREDEFINED_HH
#define _XBE_PREDEFINED_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"
#include "xbe_dispatchable.h"

class be_typedef;

/*
 * BE_PredefinedType
 */

class be_predefined_type :
   public virtual AST_PredefinedType,
   public be_DispatchableType
{

public:

   be_predefined_type
   (
      AST_PredefinedType::PredefinedType t,
      UTL_ScopedName *n,
      const UTL_Pragmas &p
   );

   virtual DDS::Boolean IsFixedLengthPrimitiveType() const;
   
   static be_predefined_type * _narrow(AST_Type * atype);

   DDS_StdString swap_call(const DDS_StdString& arg);

   // BE_TYPE_MAP VIRTUALS
   virtual void InitializeTypeMap(be_Type*);
   virtual DDS::Boolean IsFixedLength() const;

   // BE_TYPE VIRTUALS
   virtual inline void Initialize()
   {}

   virtual void GenerateType(be_ClientHeader&)
   {}

   virtual void GenerateTypedefs(
      const DDS_StdString &scope,
      const be_typedef & alias,
      be_ClientHeader& source);
   virtual DDS::Boolean IsPrimitiveType() const
   {
      return pbtrue;
   }

   virtual DDS::Boolean IsStructuredType() const;
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
      return pbfalse;
   }

   virtual DDS::Boolean IsInterfaceType() const
   {
      return m_isInterfaceType;
   }

   virtual pbbool IsReturnedByVar () const
   {
      return m_isReturnedByVar;
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

   virtual void GeneratePutGetOps(be_ClientHeader& source)
   {}

   virtual void GenerateStreamOps(be_ClientHeader&)
   {}

   virtual void GenerateTypeCode(be_ClientHeader &)
   {}

   virtual void GenerateTypeCode(be_ClientImplementation &)
   {}

   virtual void GenerateTypeCodeInit(be_ClientHeader &source);
   virtual void GenerateTypeCodeInit(
      be_ClientImplementation &source)
   {}

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
   virtual DDS::ULong get_OS_elem_alignment();

   virtual void make_get_param_field(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg);
   virtual void make_put_param_field(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg);

   // PUBLIC METHODS FOR THE NEW WORLD ORDER

   virtual be_CppType CppTypeWhenSequenceMember() const;

   // BE_DISPATCHABLETYPE VIRTUALS
   virtual be_DispatchableType::en_HowStoredInDispatcher
   HowStoredInDispatcher(const be_ArgumentDirection& direction) const;

   DEF_NARROW_METHODS3(be_predefined_type, AST_PredefinedType,
                       be_DispatchableType, be_Type);
   DEF_NARROW_FROM_DECL(be_predefined_type);

private:

   DDS::Boolean m_isInterfaceType;
   DDS::Boolean m_isReturnedByVar;
   DDS::ULong m_elemSize;
   DDS::ULong m_elemAlign;
   DDS_StdString m_swapCall;

   // PRIVATE HELPERS

   bool IsPointerTypeWhenSequenceMember() const;
};

#endif
