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
#ifndef _XBE_STRING_HH
#define _XBE_STRING_HH

#include "idl.h"
#include "Std.h"
#include "StdString.h"
#include "xbe_type.h"
#include "xbe_dispatchable.h"

class be_typedef;
class be_ClientHeader;
class be_ClientImplementation;
class be_ServerHeader;
class be_ServerImplementation;

class be_string
         :
         public virtual AST_String,
         public be_DispatchableType
{

public:

   be_string();
   be_string(AST_Expression *v);
   be_string(AST_Expression *v, long wide);

   // BE_STRING STATICS
   static be_string * _narrow(AST_Type * type);

   // BE_TYPE_MAP VIRTUALS
   virtual void InitializeTypeMap(be_Type*);
   virtual pbbool IsFixedLength() const;
   virtual pbbool IsFixedLengthPrimitiveType() const;

   // BE_TYPE VIRTUALS
   virtual inline void Initialize()
   {}

   virtual void GenerateType(be_ClientHeader&)
   {}

   virtual void GenerateTypedefs(
      const DDS_StdString &scope,
      const be_typedef & alias,
      be_ClientHeader& source);
   virtual void GeneratePutGetOps(be_ClientHeader& source)
   {}

   virtual inline void GenerateStreamOps(be_ClientHeader&)
   {}

   virtual pbbool IsPrimitiveType() const
   {
      return pbfalse;
   }

   virtual pbbool IsStructuredType() const
   {
      return pbfalse;
   }

   virtual pbbool IsStringType() const
   {
      return pbtrue;
   }

   virtual pbbool IsArrayType() const
   {
      return pbfalse;
   }

   virtual pbbool IsSequenceType() const
   {
      return pbfalse;
   }

   virtual pbbool IsInterfaceType() const
   {
      return pbfalse;
   }

   virtual pbbool IsReturnedByVar () const
   {
      return pbfalse;
   }

   virtual pbbool IsWide() const;
   virtual DDS_StdString Allocater(const DDS_StdString&) const;
   virtual DDS_StdString Initializer(const DDS_StdString&, VarType) const;
   virtual DDS_StdString InRequestArgumentDeclaration(
      be_Type&,
      const DDS_StdString&,
      VarType vt);
   virtual DDS_StdString Releaser(const DDS_StdString&) const;
   virtual DDS_StdString Assigner(const DDS_StdString&, const DDS_StdString&) const;
   virtual DDS_StdString Duplicater(
      const DDS_StdString&,
      const DDS_StdString&,
      const DDS_StdString&,
      const pbbool) const;
   virtual DDS_StdString NullReturnArg();
   virtual DDS_StdString SyncStreamIn(
      const DDS_StdString&,
      const DDS_StdString&,
      VarType) const;
   virtual DDS_StdString SyncStreamOut(
      const DDS_StdString&,
      const DDS_StdString&,
      VarType) const;
   virtual DDS_StdString StructStreamIn(
      const DDS_StdString&,
      const DDS_StdString&) const;
   virtual DDS_StdString StructStreamOut(
      const DDS_StdString&,
      const DDS_StdString&) const;
   virtual DDS_StdString UnionStreamIn(
      const DDS_StdString&,
      const DDS_StdString&) const;
   virtual DDS_StdString UnionStreamOut(
      const DDS_StdString&,
      const DDS_StdString&) const;

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

   virtual DDS::Boolean make_put_param_for_struct
   (
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid
   );
   virtual DDS::Boolean make_get_param_for_struct
   (
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid
   );
   virtual DDS::Boolean make_put_param_for_union
   (
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid
   );
   virtual DDS::Boolean make_get_param_for_union
   (
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid
   );

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
   long m_wide;

   DEF_NARROW_METHODS3(be_string, AST_String, be_DispatchableType, be_Type);
   DEF_NARROW_FROM_DECL(be_string);
};

#endif
