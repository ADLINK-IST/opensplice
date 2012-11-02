/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
*/

#ifndef _SACPP_MAPPING_VALUE_H_
#define _SACPP_MAPPING_VALUE_H_

#include "sacpp_if.h"
// Template classes used to implement valuetype _var and _out types.

template <class Type> class DDSValueBase_var;

template <class Type> class DDSValueBase_mgr
{
   public:
      DDSValueBase_mgr (Type *& p,  DDS::Boolean rel);

      DDSValueBase_mgr (const DDSValueBase_mgr<Type> & v);

      DDSValueBase_mgr<Type> & operator = (Type * p);

      DDSValueBase_mgr<Type> & operator = (const DDSValueBase_mgr<Type> & p);

      DDSValueBase_mgr<Type> & operator = (const DDSValueBase_var<Type> & p);

      operator Type *& ();

      Type * operator -> () const;

      Type * in () const;

      Type *& inout ();

      Type *& out ();

      Type * _retn ();

      Type ** m_ptr;
      DDS::Boolean m_rel;
};

template <class Type> class DDSValueBase_var
{
   public:
      DDSValueBase_var (Type *p = 0);

      DDSValueBase_var (const DDSValueBase_var<Type> & v);

      DDSValueBase_var (const DDSValueBase_mgr<Type> & m);

      ~DDSValueBase_var ();

      DDSValueBase_var<Type> & operator = (Type * p);

      DDSValueBase_var<Type> & operator = (const DDSValueBase_var<Type> & p);

      DDSValueBase_var<Type> & operator = (const DDSValueBase_mgr<Type> & p);

      operator Type *& ();

      Type * operator -> () const;

      Type * in () const;

      Type *& inout ();

      Type *& out ();

      Type * _retn ();

      Type * m_ptr;
};

template <class Type> class DDSValueBase_out
{
   public:
      DDSValueBase_out (Type *& p);

      DDSValueBase_out (DDSValueBase_mgr<Type> & v);

      DDSValueBase_out<Type> & operator = (Type * p);

      Type * operator -> ();

      operator Type * () const;

      Type *& m_ptr;
};

template <class Type> inline
DDSValueBase_mgr<Type>::DDSValueBase_mgr(Type *& p, DDS::Boolean rel)
   : m_ptr (&p),
     m_rel (rel)
{
}

template <class Type> inline
DDSValueBase_mgr<Type>::DDSValueBase_mgr(const DDSValueBase_mgr<Type> & m)
   : m_ptr (m.m_ptr),
     m_rel (m.m_rel)
{
}

template <class Type> inline DDSValueBase_mgr<Type>&
DDSValueBase_mgr<Type>::operator = (Type * p)
{
   if (m_rel)
   {
      DDS::remove_ref (*m_ptr);
   }

   *m_ptr = p;
   m_rel = TRUE;

   return *this;
}

template <class Type> inline DDSValueBase_mgr<Type>&
DDSValueBase_mgr<Type>::operator = (const DDSValueBase_mgr<Type> & p)
{
   if (m_rel)
   {
      DDS::remove_ref (*m_ptr);
   }

   m_ptr = p.m_ptr;
   m_rel = p.m_rel;

   return *this;
}

template <class Type> inline DDSValueBase_mgr<Type>&
DDSValueBase_mgr<Type>::operator = (const DDSValueBase_var<Type> & p)
{
   if (m_rel)
   {
      DDS::remove_ref (*m_ptr);
   }

   *m_ptr = p.in ();
   m_rel = TRUE;
   DDS::add_ref (*m_ptr);
   return *this;
}

template <class Type> inline DDSValueBase_mgr<Type>::operator Type *& ()
{
   return *m_ptr;
}

template <class Type> inline Type *
DDSValueBase_mgr<Type>::operator -> () const
{
   return *m_ptr;
}

template <class Type> inline Type * DDSValueBase_mgr<Type>::in () const
{
   return *m_ptr;
}

template <class Type> inline Type *& DDSValueBase_mgr<Type>::inout ()
{
   return *m_ptr;
}

template <class Type> inline Type *& DDSValueBase_mgr<Type>::out ()
{
   if (m_rel)
   {
      DDS::remove_ref (*m_ptr);
   }

   *m_ptr = NULL;
   return *m_ptr;
}

template <class Type> inline Type * DDSValueBase_mgr<Type>::_retn ()
{
   Type * ret = *m_ptr;
   *m_ptr = NULL;
   return ret;
}

template <class Type> inline
DDSValueBase_var<Type>::DDSValueBase_var(Type * p)
   : m_ptr (p)
{
}

template <class Type> inline
DDSValueBase_var<Type>::DDSValueBase_var(const DDSValueBase_var<Type> & v)
   : m_ptr (v.m_ptr)
{
   DDS::add_ref (m_ptr);
}

template <class Type> inline
DDSValueBase_var<Type>::DDSValueBase_var(const DDSValueBase_mgr<Type> & m)
  : m_ptr (m.in ())
{
   DDS::add_ref (m_ptr);
}

template <class Type> inline DDSValueBase_var<Type>::~DDSValueBase_var ()
{
   DDS::remove_ref (this->m_ptr);
}

template <class Type> inline DDSValueBase_var<Type>&
DDSValueBase_var<Type>::operator = (Type * p)
{
   DDS::remove_ref (m_ptr);
   m_ptr = p;

   return *this;
}

template <class Type> inline DDSValueBase_var<Type>&
DDSValueBase_var<Type>::operator = (const DDSValueBase_var<Type> & p)
{
   DDS::remove_ref (m_ptr);
   m_ptr = p.m_ptr;
   DDS::add_ref (m_ptr);
   return *this;
}

template <class Type> inline DDSValueBase_var<Type>&
DDSValueBase_var<Type>::operator = (const DDSValueBase_mgr<Type> & p)
{
   DDS::remove_ref (m_ptr);
   m_ptr = p.in ();
   DDS::add_ref (m_ptr);
   return *this;
}

template <class Type> inline DDSValueBase_var<Type>::operator Type *& ()
{
   return m_ptr;
}

template <class Type> inline Type *
DDSValueBase_var<Type>::operator -> () const
{
   return m_ptr;
}

template <class Type> inline Type * DDSValueBase_var<Type>::in () const
{
   return m_ptr;
}

template <class Type> inline Type *& DDSValueBase_var<Type>::inout ()
{
   return m_ptr;
}

template <class Type> inline Type *& DDSValueBase_var<Type>::out ()
{
   DDS::remove_ref (m_ptr);
   m_ptr = NULL;
   return m_ptr;
}

template <class Type> inline Type * DDSValueBase_var<Type>::_retn ()
{
   Type * ret = m_ptr;
   m_ptr = NULL;
   return ret;
}

template <class Type> inline
DDSValueBase_out<Type>::DDSValueBase_out(Type *& p)
   : m_ptr (p)
{
}

template <class Type> inline
DDSValueBase_out<Type>::DDSValueBase_out(DDSValueBase_mgr<Type> & v)
   : m_ptr (v.m_ptr)
{
   DDS::remove_ref (m_ptr);
}

template <class Type> inline DDSValueBase_out<Type> &
DDSValueBase_out<Type>::operator = (Type * p)
{
   m_ptr = p;
   return *this;
}

template <class Type> inline Type * DDSValueBase_out<Type>::operator -> ()
{
   return m_ptr;
}

template <class Type> inline DDSValueBase_out<Type>::operator Type * () const
{
   return m_ptr;
}

#undef SACPP_API
#endif
