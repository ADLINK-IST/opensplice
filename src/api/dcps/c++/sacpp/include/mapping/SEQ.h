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
#ifndef SACPP_MAPPING_SEQ_H
#define SACPP_MAPPING_SEQ_H

#include "cpp_dcps_if.h"

template <class T> class DDS_DCPSSequence_var
{
public:

   DDS_DCPSSequence_var () : m_ptr (NULL) {}

   DDS_DCPSSequence_var (T * p) : m_ptr (p) {}

   DDS_DCPSSequence_var (const DDS_DCPSSequence_var<T> & that)
   {
      m_ptr = (that.m_ptr) ? new T (*(that.m_ptr)) : NULL;
   }

   ~DDS_DCPSSequence_var () { delete m_ptr; }

   DDS_DCPSSequence_var<T> & operator = (T * p)
   {
      delete m_ptr;
      m_ptr = p;
      return *this;
   }

   DDS_DCPSSequence_var<T> & operator =
   (
      const DDS_DCPSSequence_var<T> & that
   )
   {
      if (this != &that)
      {
         delete m_ptr;
         m_ptr = (that.m_ptr) ? new T (*(that.m_ptr)) : NULL;
      }
      return *this;
   }

   typename T::_subscript_type operator [] (DDS::ULong index)
   {
      assert (m_ptr);
      return m_ptr->operator[] (index);
   }

   typename T::_const_subscript_type operator [] (DDS::ULong index) const
   {
      assert (m_ptr);
      return ((const T *)m_ptr)->operator[] (index);
   }

   typename T::_subscript_type operator [] (int index)
   {
      assert (m_ptr);
      assert (index >= 0);
      return m_ptr->operator[] (index);
   }

   T * operator -> () { return m_ptr; }

   operator T & () { return *m_ptr; }

   operator const T & () const { return *m_ptr; }

   operator T * () { return m_ptr; }

   operator const T * () const { return m_ptr; }

   const T & in () const { return *m_ptr; }

   T & inout () { return *m_ptr; }

   T *& val () { return m_ptr; }

   T *& out ()
   {
      delete m_ptr;
      m_ptr = NULL;
      return m_ptr;
   }

   T * _retn ()
   {
      T * ret = m_ptr;
      m_ptr = NULL;
      return ret;
   }

public:

   T * m_ptr;
};

template <class T> class DDS_DCPSSequence_out
{
public:

   DDS_DCPSSequence_out (T *& p) : m_ptr (p) {}

   DDS_DCPSSequence_out (DDS_DCPSSequence_var<T> & v)
      : m_ptr (v.m_ptr)
   {
      delete m_ptr;
   }

   DDS_DCPSSequence_out (const DDS_DCPSSequence_out<T> & v)
      : m_ptr (v.m_ptr)
   {}

   DDS_DCPSSequence_out<T>& operator=(T * p)
   {
      m_ptr = p;
      return *this;
   }

   typename T::_subscript_type operator [] (DDS::ULong index)
   {
      assert (m_ptr);
      return m_ptr->operator[] (index);
   }

   typename T::_subscript_type operator [] (int index)
   {
      assert (m_ptr);
      assert (index >= 0);
      return m_ptr->operator[] (index);
   }

   T * operator -> () { return m_ptr; }

   T *& out () { return m_ptr; }

   operator T * () const { return m_ptr; }

public:

   T *& m_ptr;
};

#undef OS_API
#endif /* SACPP_MAPPING_SEQ_H */
