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
#ifndef _XBE_STRUCTURE_HH
#define _XBE_STRUCTURE_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"
#include "xbe_field.h"
#include "xbe_dispatchable.h"

class be_typedef;

class be_structure :
   public virtual AST_Structure,
   public virtual be_CodeGenerator,
   public virtual be_DispatchableType
{

public:

   typedef TList<be_field *> FieldList;

   static be_structure * _narrow(AST_Type * atype);

   be_structure();
   be_structure(UTL_ScopedName *n, const UTL_Pragmas &p);

   void is_fixed_length(DDS::Boolean val);

   void putter( ostream & os,
                be_Tab & tab,
                const DDS_StdString & seqptr,
                unsigned long uid);
   void getter( ostream & os,
                be_Tab & tab,
                const DDS_StdString & seqptr,
                unsigned long uid);

   // BE_CODEGENERATOR VIRTUALS
   virtual void Generate(be_ClientHeader& source);
   virtual void Generate(be_ClientImplementation&);
   virtual void Generate(be_ServerHeader&)
   {}

   virtual void Generate(be_ServerImplementation&)
   {}

   // BE_TYPE_MAP VIRTUALS
   virtual DDS::Boolean IsFixedLength() const;
   virtual DDS::Boolean IsFixedLengthPrimitiveType() const;
   virtual pbbool IsOptimizable ();
   virtual void InitializeTypeMap(be_Type*);

   // BE_TYPE VIRTUALS
   virtual inline void Initialize()
   {}

   virtual void GenerateType(be_ClientHeader& source)
   { //Generate(source);
   }

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

   virtual pbbool IsReturnedByVar () const
   {
      return ! m_isFixedLength;
   }

   virtual pbbool IsInterfaceDependant () const;

   virtual DDS_StdString Allocater(const DDS_StdString&) const;
   virtual DDS_StdString Initializer(const DDS_StdString&, VarType) const;
   virtual DDS_StdString Releaser(const DDS_StdString&) const;
   virtual DDS_StdString Assigner(const DDS_StdString&, const DDS_StdString&) const;
   virtual DDS_StdString Duplicater(
      const DDS_StdString&,
      const DDS_StdString&,
      const DDS_StdString&,
      const pbbool) const;
   virtual DDS_StdString SyncStreamOut(
      const DDS_StdString& arg,
      const DDS_StdString& out, VarType vt) const;
   virtual DDS_StdString NullReturnArg();
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
   virtual DDS::Boolean declare_for_union_get(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      const DDS_StdString & fld,
      unsigned long uid);
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
   virtual DDS::ULong get_max_elem_alignment();

   // AST_STRUCTURE VIRTUALS
   virtual AST_Field * add_field(AST_Field *f);

   // BE_DISPATCHABLETYPE VIRTUALS
   virtual be_DispatchableType::en_HowStoredInDispatcher
   HowStoredInDispatcher(const be_ArgumentDirection& direction) const;

   DEF_NARROW_METHODS4(be_structure, AST_Structure, be_CodeGenerator,
                       be_DispatchableType, be_Type);
   DEF_NARROW_FROM_DECL(be_structure);
   DEF_NARROW_FROM_SCOPE(be_structure);

private:

   DDS::Boolean m_isFixedLength;
   DDS::Boolean m_marshalInCore;
   FieldList m_fields;
   DDS_StdString m_nullArg;
   DDS::ULong m_elemAlignment;
   DDS::ULong m_maxElemAlignment;
   DDS::ULong m_elemSize;
   DDS::Boolean m_canOptimize;
   DDS::ULong m_lastFieldSize;
   const be_CppEnclosingScope m_cppScope;
   const be_CppEnclosingScope m_cppType;
   pbbool m_interface_dependant;

   void GenerateAuxTypes(be_ClientHeader& source);
   void GenerateMemcpyCopyConstructor(be_ClientHeader& source);
   void GenerateDefaultConstructor(be_ClientHeader& source);
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


#endif
