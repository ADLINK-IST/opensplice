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
#ifndef _XBE_ENUM_HH
#define _XBE_ENUM_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"
#include "xbe_dispatchable.h"

class be_typedef;

class be_enum
         :
         public virtual AST_Enum,
         public be_CodeGenerator,
         public be_DispatchableType
{

public:

   typedef DDS::Boolean Boolean;

   static be_enum * _narrow(AST_Type * atype);

   be_enum();
   be_enum(UTL_ScopedName *n, const UTL_Pragmas &p);

   DDS_StdString swap_call(const DDS_StdString & arg);

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& source);
   virtual void Generate(be_ClientImplementation&)
   {}

   virtual void Generate(be_ServerHeader&)
   {}

   virtual void Generate(be_ServerImplementation&)
   {}

   // BE_TYPE_MAP VIRTUALS
   virtual void InitializeTypeMap(be_Type*);
   virtual Boolean IsFixedLength() const;
   virtual Boolean IsFixedLengthPrimitiveType() const;

   // BE_TYPE VIRTUALS
   virtual void Initialize()
   {}

   virtual void GenerateType(be_ClientHeader& source);
   virtual void GenerateTypedefs(
      const DDS_StdString &scope,
      const be_typedef & alias,
      be_ClientHeader& source);
   virtual Boolean IsPrimitiveType() const
   {
      return pbtrue;
   }

   virtual Boolean IsEnumeratedType() const
   {
      return pbtrue;
   }

   virtual Boolean IsStructuredType() const
   {
      return pbfalse;
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
      return pbfalse;
   }

   virtual DDS_StdString Allocater(const DDS_StdString&) const;
   virtual DDS_StdString Initializer(const DDS_StdString&, VarType) const;
   virtual DDS_StdString Releaser(const DDS_StdString&) const;
   virtual DDS_StdString Assigner(
      const DDS_StdString&,
      const DDS_StdString&) const;
   virtual DDS_StdString Duplicater
   (
      const DDS_StdString&,
      const DDS_StdString&,
      const DDS_StdString&,
      const pbbool
   ) const;
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

   // BE_DISPATCHABLETYPE VIRTUALS
   virtual be_DispatchableType::en_HowStoredInDispatcher
   HowStoredInDispatcher(const be_ArgumentDirection& direction) const;

   virtual void FinishProtoTypeCode();

   DEF_NARROW_METHODS4(be_enum, AST_Enum, be_CodeGenerator,
                       be_DispatchableType, be_Type);
   DEF_NARROW_FROM_DECL(be_enum);
   DEF_NARROW_FROM_SCOPE(be_enum);

private:

   DDS_StdString m_swapCall;
};

class be_enum_val
         :
         public virtual AST_EnumVal
{

public:

   be_enum_val();
   be_enum_val(unsigned long v, UTL_ScopedName *n, const UTL_Pragmas &p);

   unsigned long Value();

   DEF_NARROW_METHODS1(be_enum_val, AST_EnumVal);
   DEF_NARROW_FROM_DECL(be_enum_val);
};

#endif
