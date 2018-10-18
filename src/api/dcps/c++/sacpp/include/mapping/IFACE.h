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
#ifndef SACPP_MAPPING_IFACE_H_
#define SACPP_MAPPING_IFACE_H_

#include "cpp_dcps_if.h"
// Template classes used to implement interface _var, _out
// and _mgr types.
// See section 1.3.7 of IDL to C++ mapping specification.

template <class Type> class DDS_DCPSInterface_mgr;

template <class Type> class DDS_DCPSInterface_var
{
   public:
      DDS_DCPSInterface_var (Type * p = NULL);

      DDS_DCPSInterface_var (const DDS_DCPSInterface_var<Type> & v);

      DDS_DCPSInterface_var (const DDS_DCPSInterface_mgr<Type> & m);

      ~DDS_DCPSInterface_var ();

      DDS_DCPSInterface_var<Type> & operator = (Type * p);

      DDS_DCPSInterface_var<Type> & operator = (const DDS_DCPSInterface_var<Type> & p);

      DDS_DCPSInterface_var<Type> & operator = (const DDS_DCPSInterface_mgr<Type> & p);

      operator Type *& ();

      Type * operator -> () const;

      Type * in () const;

      Type *& inout ();

      Type *& val ();

      Type *& out ();

      Type * _retn ();

      Type * m_ptr;
};


template <class Type> class DDS_DCPSInterface_out
{
   public:
      DDS_DCPSInterface_out (Type *& p);

      DDS_DCPSInterface_out (DDS_DCPSInterface_var<Type> & v);

      DDS_DCPSInterface_out<Type> & operator = (Type * p);

      Type * operator -> ();

      operator Type * () const;

      Type *& m_ptr;
};

template <class Type> class DDS_DCPSInterface_mgr
{
   public:
      DDS_DCPSInterface_mgr (Type *& p, DDS::Boolean rel);

      DDS_DCPSInterface_mgr (const DDS_DCPSInterface_mgr<Type> & p);

      DDS_DCPSInterface_mgr<Type> & operator = (Type* p);

      DDS_DCPSInterface_mgr<Type> & operator = (const DDS_DCPSInterface_mgr<Type>& m);

      DDS_DCPSInterface_mgr<Type> & operator = (const DDS_DCPSInterface_var<Type>& v);

      Type * operator -> ();

      const Type * operator -> () const;

      operator const Type * () const;

      operator Type *& ();

      Type * in () const;

      Type *& inout ();

      Type *& out ();

   private:
      Type ** m_ptr;
      DDS::Boolean m_rel;
};

template <class Type> inline
DDS_DCPSInterface_var<Type>::DDS_DCPSInterface_var(Type * p)
   : m_ptr (p)
{
}

template <class Type> inline
DDS_DCPSInterface_var<Type>::DDS_DCPSInterface_var(const DDS_DCPSInterface_var<Type> & v)
   : m_ptr (Type::_duplicate (v.m_ptr))
{
}

template <class Type> inline
DDS_DCPSInterface_var<Type>::DDS_DCPSInterface_var(const DDS_DCPSInterface_mgr<Type> & m)
   : m_ptr (Type::_duplicate (m.in ()))
{
}

template <class Type> inline DDS_DCPSInterface_var<Type>::~DDS_DCPSInterface_var ()
{
   DDS::release (m_ptr);
}

template <class Type> inline DDS_DCPSInterface_var<Type>&
DDS_DCPSInterface_var<Type>::operator = (Type * p)
{
   DDS::release (m_ptr);
   m_ptr = p;

   return *this;
}

template <class Type> inline DDS_DCPSInterface_var<Type>&
DDS_DCPSInterface_var<Type>::operator = (const DDS_DCPSInterface_var<Type> & p)
{
   DDS::release (m_ptr);
   m_ptr = Type::_duplicate (p.m_ptr);

   return *this;
}

template <class Type> inline DDS_DCPSInterface_var<Type>&
DDS_DCPSInterface_var<Type>::operator = (const DDS_DCPSInterface_mgr<Type> & p)
{
   DDS::release (m_ptr);
   m_ptr = Type::_duplicate (p.in ());

   return *this;
}

template <class Type> inline DDS_DCPSInterface_var<Type>::operator Type *& ()
{
   return m_ptr;
}

template <class Type> inline Type *
DDS_DCPSInterface_var<Type>::operator -> () const
{
   return m_ptr;
}

template <class Type> inline Type * DDS_DCPSInterface_var<Type>::in () const
{
   return m_ptr;
}

template <class Type> inline Type *& DDS_DCPSInterface_var<Type>::inout ()
{
   return m_ptr;
}

template <class Type> inline Type *& DDS_DCPSInterface_var<Type>::val ()
{
   return m_ptr;
}

template <class Type> inline Type *& DDS_DCPSInterface_var<Type>::out ()
{
   DDS::release (m_ptr);
   m_ptr = Type::_nil ();
   return m_ptr;
}

template <class Type> inline Type * DDS_DCPSInterface_var<Type>::_retn ()
{
   Type * ret = m_ptr;
   m_ptr = Type::_nil ();
   return ret;
}

template <class Type> inline
DDS_DCPSInterface_out<Type>::DDS_DCPSInterface_out(Type *& p)
   : m_ptr (p)
{
}

template <class Type> inline
DDS_DCPSInterface_out<Type>::DDS_DCPSInterface_out(DDS_DCPSInterface_var<Type> & v)
   : m_ptr (v.m_ptr)
{
   DDS::release (m_ptr);
}

template <class Type> inline DDS_DCPSInterface_out<Type> &
DDS_DCPSInterface_out<Type>::operator = (Type * p)
{
   m_ptr = p;
   return *this;
}

template <class Type> inline Type * DDS_DCPSInterface_out<Type>::operator -> ()
{
   return m_ptr;
}

template <class Type> inline DDS_DCPSInterface_out<Type>::operator Type * () const
{
   return m_ptr;
}

template <class Type> inline
DDS_DCPSInterface_mgr<Type>::DDS_DCPSInterface_mgr (Type *& p, DDS::Boolean rel)
   : m_ptr (&p), m_rel (rel)
{
}

template <class Type> inline
DDS_DCPSInterface_mgr<Type>::DDS_DCPSInterface_mgr (const DDS_DCPSInterface_mgr<Type> & p)
   : m_ptr (p.m_ptr), m_rel (p.m_rel)
{
}

template <class Type> inline
DDS_DCPSInterface_mgr<Type> & DDS_DCPSInterface_mgr<Type>::operator = (Type* p)
{
   if (m_rel && (*m_ptr))
   {
      DDS::release (*m_ptr);
   }

   *m_ptr = p;
   m_rel = TRUE;

   return *this;
}

template <class Type> inline DDS_DCPSInterface_mgr<Type> &
DDS_DCPSInterface_mgr<Type>::operator = (const DDS_DCPSInterface_mgr<Type>& m)
{
   if (m_rel && (*m_ptr))
   {
      DDS::release (*m_ptr);
   }

   m_ptr = m.m_ptr;
   m_rel = m.m_rel;

   return *this;
}

template <class Type> inline DDS_DCPSInterface_mgr<Type> &
DDS_DCPSInterface_mgr<Type>::operator = (const DDS_DCPSInterface_var<Type>& v)
{
   if (m_rel && (*m_ptr))
   {
      DDS::release (*m_ptr);
   }

   *m_ptr = Type::_duplicate (v.in ());
   m_rel = TRUE;

   return *this;
}

template <class Type> inline
Type * DDS_DCPSInterface_mgr<Type>::operator -> ()
{
   return *m_ptr;
}

template <class Type> inline
const Type * DDS_DCPSInterface_mgr<Type>::operator -> () const
{
   return *m_ptr;
}

template <class Type> inline
DDS_DCPSInterface_mgr<Type>::operator const Type * () const
{
   return *m_ptr;
}

template <class Type> inline
DDS_DCPSInterface_mgr<Type>::operator Type *& ()
{
   return *m_ptr;
}

template <class Type> inline
Type * DDS_DCPSInterface_mgr<Type>::in () const
{
   return *m_ptr;
}

template <class Type> inline
Type *& DDS_DCPSInterface_mgr<Type>::inout ()
{
   return *m_ptr;
}

template <class Type> inline Type *& DDS_DCPSInterface_mgr<Type>::out ()
{
   if (m_rel)
   {
      DDS::release (*m_ptr);
   }
   *m_ptr = Type::_nil ();
   return *m_ptr;
}

#undef OS_API
#endif /* SACPP_MAPPING_IFACE_H */
