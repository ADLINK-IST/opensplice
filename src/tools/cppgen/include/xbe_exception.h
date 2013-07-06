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
#ifndef _XBE_EXCEPTION_HH
#define _XBE_EXCEPTION_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"
#include "xbe_field.h"

class be_typedef;

class be_exception
:
   public virtual AST_Exception,
   public virtual be_ClassGenerator,
   public virtual be_Type
{

public:

   typedef DDS::Boolean Boolean;
   typedef TList<be_field *> FieldList;

   be_exception();
   be_exception(UTL_ScopedName *n,
                const UTL_Pragmas &p);

   void is_fixed_length(DDS::Boolean val);
   void putter( ostream & os,
                be_Tab & tab,
                const DDS_StdString & seqptr,
                unsigned long uid);
   void getter( ostream & os,
                be_Tab & tab,
                const DDS_StdString & seqptr,
                unsigned long uid);

   void GenerateMembers(be_ClientHeader& source);
   void GenerateConvenienceConstructor(be_ClientHeader& source);

   void GenerateConvenienceConstructor(be_ClientImplementation& source);
   void GenerateCopyConstructor(be_ClientImplementation& source);
   void GenerateAssignmentOperator(be_ClientImplementation& source);


   // BE_TYPE_MAP VIRTUALS
   virtual DDS::Boolean IsFixedLength() const;
   virtual DDS::Boolean IsFixedLengthPrimitiveType() const;
   virtual void InitializeTypeMap(be_Type*);

   // BE_TYPE VIRTUALS
   virtual inline void Initialize()
   {}

   virtual void GenerateType(be_ClientHeader& source)
   {}

   virtual void GenerateTypedefs(
      const DDS_StdString &scope,
      const be_typedef & alias,
      be_ClientHeader& source);
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
      return pbfalse;
   }

   virtual DDS::Boolean IsInterfaceType() const
   {
      return pbfalse;
   }

   virtual DDS::Boolean IsReturnedByVar  () const
   {
      return pbfalse;
   }

   virtual DDS::Boolean IsExceptionType() const
   {
      return pbtrue;
   }

   virtual DDS_StdString Allocater(const DDS_StdString&) const
   {
      return "";
   }

   virtual DDS_StdString Initializer(
      const DDS_StdString&,
      VarType) const
   {
      return "" ;
   }

   virtual DDS_StdString Releaser(const DDS_StdString&) const
   {
      return "" ;
   }

   virtual DDS_StdString Assigner(
      const DDS_StdString&,
      const DDS_StdString&) const
   {
      return "" ;
   }

   virtual DDS_StdString Duplicater(
      const DDS_StdString&,
      const DDS_StdString&,
      const DDS_StdString&,
      const pbbool) const
   {
      return "" ;
   }

   virtual DDS_StdString NullReturnArg ()
   {
      return "" ;
   }

   virtual void GeneratePutGetOps(be_ClientHeader& source)
   {}

   virtual void GenerateStreamOps(be_ClientHeader&)
   {}

   virtual void GenerateGlobalDecls (be_ClientHeader & source);

   //
   // NEW MARSHALING CALLS
   //
   virtual DDS::Boolean is_core_marshaled();
   virtual DDS::Boolean declare_for_stub
   (
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & arg,
      const DDS_StdString & stubScope,
      VarType vt
   );
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

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& baseHeader);
   virtual void Generate(be_ClientImplementation& baseImpl);
   virtual void Generate(be_ServerHeader&);
   virtual void Generate(be_ServerImplementation&);

   // AST_EXCEPTION VIRTUALS
   AST_Field * add_field(AST_Field *af);

   DEF_NARROW_METHODS3(be_exception, AST_Exception, be_ClassGenerator, be_Type);
   DEF_NARROW_FROM_DECL(be_exception);
   DEF_NARROW_FROM_SCOPE(be_exception);

   //
   // YO These are being deprecated and will soon be removed!!!
   //

private:

   FieldList m_fields;
   DDS::Boolean m_isFixedLength;
   DDS::ULong m_elemAlignment;
   DDS::ULong m_elemSize;
   DDS::ULong m_marshalInCore;

   void put_core_fields(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      FieldList & coreFields,
      unsigned long uid);
   void get_core_fields(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      FieldList & coreFields,
      unsigned long uid);
};

inline void
be_exception::is_fixed_length(DDS::Boolean val)
{
   m_isFixedLength = val;
}

#endif
