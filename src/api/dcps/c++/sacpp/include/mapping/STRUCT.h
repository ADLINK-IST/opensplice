/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef SACPP_MAPPING_STRUCT_H
#define SACPP_MAPPING_STRUCT_H

#include "cpp_dcps_if.h"

template <class Type> class DDS_DCPSStruct_var
{
public:

   DDS_DCPSStruct_var (Type * p = NULL) : m_ptr (p) {}

   DDS_DCPSStruct_var (const DDS_DCPSStruct_var<Type>& that)
   {
      m_ptr = (that.m_ptr) ? new Type (*(that.m_ptr)) : NULL;
   }

   DDS_DCPSStruct_var (const Type & that)
      : m_ptr (new Type (that))
   {}

   ~DDS_DCPSStruct_var () { delete m_ptr; }

   DDS_DCPSStruct_var<Type> & operator = (Type * p)
   {
      delete m_ptr;
      m_ptr = p;
      return *this;
   }

   DDS_DCPSStruct_var<Type> & operator = (const DDS_DCPSStruct_var<Type> & that)
   {
      if (this != &that)
      {
         delete m_ptr;
         m_ptr = (that.m_ptr) ? new Type (*(that.m_ptr)) : NULL;
      }
      return *this;
   }

   Type & operator = (const Type & that)
   {
      delete m_ptr;
      m_ptr = new Type (that);
      return *m_ptr;
   }

   Type * operator -> () { return m_ptr; }

   operator Type * () const { return m_ptr; }

   operator Type & () { return *m_ptr; }

   operator const Type & () const { return *m_ptr; }

   const Type & in () const { return *m_ptr; }

   Type & inout () { return *m_ptr; }

   Type *& val () { return m_ptr; }

   Type *& out ()
   {
      delete m_ptr;
      m_ptr = NULL;
      return m_ptr;
   }

   Type * _retn ()
   {
      Type * ret = m_ptr;
      m_ptr = NULL;
      return ret;
   }

public:

   Type * m_ptr;
};


template <class Type> class DDS_DCPSStruct_out
{
public:

   DDS_DCPSStruct_out (Type *& p) : m_ptr (p) {}

   DDS_DCPSStruct_out (const DDS_DCPSStruct_out<Type> & that)
      : m_ptr (that.m_ptr)
   {}

   DDS_DCPSStruct_out (DDS_DCPSStruct_var<Type> & that)
      : m_ptr (that.m_ptr)
   {
      if (m_ptr)
      {
         delete m_ptr;
      }
   }

   DDS_DCPSStruct_out<Type> & operator= (Type * p)
   {
      m_ptr = p;
      return *this;
   }

   Type * operator-> () { return m_ptr; }

   operator Type * () const { return m_ptr; }

   operator const Type * () const { return m_ptr; }

   Type *& out () { return m_ptr; }

public:

   Type *& m_ptr;
};

#undef OS_API
#endif /* SACPP_MAPPING_STRUCT_H */
