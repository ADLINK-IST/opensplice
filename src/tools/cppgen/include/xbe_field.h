/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef _XBE_FIELD_HH
#define _XBE_FIELD_HH

#include "xbe_codegen.h"
#include "xbe_type.h"
#include "xbe_classgen.h"

class be_field
:
   public virtual AST_Field,
   public be_TypeMap
{

public:

   static be_field * _narrow(AST_Decl * decl);

   be_field();
   be_field(AST_Type *ft, UTL_ScopedName *n, const UTL_Pragmas &p);

   const char * get_local_name();
   be_Type * get_be_type();
   DDS::ULong get_elem_alignment();
   DDS::ULong get_OS_elem_alignment();
   DDS::ULong get_elem_size();
   void initialize();
   void declare_for_struct_put(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      unsigned long uid);
   void declare_for_struct_get(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      unsigned long uid);
   void make_put_param(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      unsigned long uid);
   void make_get_param(
      ostream & os,
      be_Tab & tab,
      const DDS_StdString & sptr,
      unsigned long uid);
   ostream & put_for_struct( ostream & os,
                             be_Tab & tab,
                             const DDS_StdString & sptr,
                             unsigned long uid);
   ostream & get_for_struct( ostream & os,
                             be_Tab & tab,
                             const DDS_StdString & sptr,
                             unsigned long uid);
   DDS::Boolean is_core_marshaled();

   // BE_TYPE_MAP VIRTUALS
   virtual void InitializeTypeMap(be_Type*);
   virtual pbbool IsFixedLength() const;
   virtual pbbool IsFixedLengthPrimitiveType() const;
   DEF_NARROW_METHODS2(be_field, AST_Field, be_TypeMap);
   DEF_NARROW_FROM_DECL(be_field);

private:

   be_Type * m_type;
};

inline const char*
be_field::get_local_name()
{
   return local_name()->get_string();
}

inline be_Type*
be_field::get_be_type()
{
   return m_type;
}

inline void
be_field::declare_for_struct_put(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   unsigned long uid)
{
   m_type->declare_for_struct_put( os,
                                   tab,
                                   sptr,
                                   local_name()->get_string(),
                                   uid);
}

inline void
be_field::declare_for_struct_get(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   unsigned long uid)
{
   m_type->declare_for_struct_get( os,
                                   tab,
                                   sptr,
                                   local_name()->get_string(),
                                   uid);
}

inline void be_field::make_put_param
(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   unsigned long uid
)
{
   m_type->make_put_param_for_struct (os, tab, sptr, local_name()->get_string(), uid);
}

inline void
be_field::make_get_param(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   unsigned long uid)
{
   m_type->make_get_param_for_struct(
      os,
      tab,
      sptr,
      local_name()->get_string(),
      uid);
}

inline ostream&
be_field::put_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   unsigned long uid)
{
   return m_type->put_for_struct(os,
                                 tab,
                                 sptr,
                                 local_name()->get_string(),
                                 uid);
}

inline ostream&
be_field::get_for_struct(
   ostream & os,
   be_Tab & tab,
   const DDS_StdString & sptr,
   unsigned long uid)
{
   return m_type->get_for_struct(os,
                                 tab,
                                 sptr,
                                 local_name()->get_string(),
                                 uid);
}

inline DDS::Boolean
be_field::is_core_marshaled()
{
   return m_type->is_core_marshaled();
}

inline DDS::ULong
be_field::get_elem_alignment()
{
   return m_type->get_elem_alignment();
}

inline DDS::ULong
   be_field::get_OS_elem_alignment()
{
   return m_type->get_OS_elem_alignment();
}

inline DDS::ULong
be_field::get_elem_size()
{
   return m_type->get_elem_size();
}

#endif
