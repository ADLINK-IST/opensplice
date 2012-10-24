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

#ifndef _SACPP_MAPPING_UVAL_H_
#define _SACPP_MAPPING_UVAL_H_

#include "sacpp_if.h"
#include "sacpp_Memory.h"

// Unbounded value type sequence

template <class T> class DDS_DCPSUValSeq
{
public:

   typedef DDSValueBase_mgr<T> _subscript_type;
   typedef const DDSValueBase_mgr<T> _const_subscript_type;

   static T ** allocbuf (DDS::ULong);
   static void freebuf (T**);

   DDS_DCPSUValSeq ();
   DDS_DCPSUValSeq (DDS::ULong);
   DDS_DCPSUValSeq
   (
      DDS::ULong max,
      DDS::ULong len,
      T** data,
      DDS::Boolean rel = FALSE
   );
   DDS_DCPSUValSeq (const DDS_DCPSUValSeq<T>&);
   ~DDS_DCPSUValSeq ();

   DDS_DCPSUValSeq<T>& operator= (const DDS_DCPSUValSeq<T>&);

   DDS::ULong maximum () const;

   void length (DDS::ULong);
   DDS::ULong length() const;

   DDSValueBase_mgr<T> operator [] (DDS::ULong index) const;
   DDSValueBase_mgr<T> operator [] (DDS::ULong index);

   DDS::Boolean release () const;
   void replace
   (
      DDS::ULong max,
      DDS::ULong length,
      T ** data,
      DDS::Boolean rel = FALSE
   );

   T ** get_buffer (DDS::Boolean = FALSE);
   T * const * get_buffer () const;

private:

   DDS::ULong m_max;
   DDS::ULong m_length;
   T ** m_buffer;
   DDS::Boolean m_release;
};

template <class T> inline T ** DDS_DCPSUValSeq<T>::allocbuf (DDS::ULong nelems)
{
   T ** buffer = (T**) DDS_Memory::_vec_alloc (sizeof (T*), nelems);

   for (DDS::ULong i = 0; i < nelems; i++)
   {
      buffer[i] = 0;
   }

   return buffer;
}

template <class T> inline void DDS_DCPSUValSeq<T>::freebuf (T ** buffer)
{
   if (buffer)
   {
      DDS::ULong nelems = DDS_Memory::_vec_size(buffer);

      for (DDS::ULong i = 0; i < nelems; i++)
      {
         DDS::remove_ref (buffer[i]);
      }

      DDS_Memory::_vec_dealloc (buffer);
   }
}

template <class T> inline DDS_DCPSUValSeq<T>::DDS_DCPSUValSeq ()
:
   m_max (0),
   m_length (0),
   m_buffer (0),
   m_release (FALSE)
{}

template <class T> inline DDS_DCPSUValSeq<T>::DDS_DCPSUValSeq (DDS::ULong nelems)
:
   m_max (nelems),
   m_length (0),
   m_buffer (allocbuf (m_max)),
   m_release (TRUE)
{}

template <class T> inline DDS_DCPSUValSeq<T>::DDS_DCPSUValSeq
(
   DDS::ULong max,
   DDS::ULong len,
   T ** data,
   DDS::Boolean rel
)
:
   m_max (max),
   m_length (len),
   m_buffer (data),
   m_release (rel)
{
   assert (m_length <= m_max);
}

template <class T> inline DDS_DCPSUValSeq<T>::DDS_DCPSUValSeq (const DDS_DCPSUValSeq<T>& that)
:
   m_max (0),
   m_length (0),
   m_buffer (0),
   m_release (FALSE)
{
   *this = that;
}

template <class T> inline DDS_DCPSUValSeq<T>::~DDS_DCPSUValSeq ()
{
   if (m_release)
   {
      freebuf (m_buffer);
   }
}

template <class T> inline DDS_DCPSUValSeq<T>& DDS_DCPSUValSeq<T>::operator =
(
   const DDS_DCPSUValSeq<T>& that
)
{
   if (this != &that)
   {
      if (m_release)
      {
         freebuf (m_buffer);
      }

      m_max = that.m_max;
      m_length = that.m_length;
      m_buffer = allocbuf (m_max);
      m_release = TRUE;

      for (DDS::ULong i = 0; i < m_length; i++)
      {
         m_buffer[i] = that.m_buffer[i];
         m_buffer[i]->_add_ref ();
      }
   }

   return *this;
}

template <class T> inline DDS::ULong DDS_DCPSUValSeq<T>::maximum () const
{
   return m_max;
}

template <class T> inline void DDS_DCPSUValSeq<T>::length (DDS::ULong nelems)
{
   if (nelems > m_max)
   {
      T** oldBuf = m_buffer;

      m_max = nelems;
      m_buffer = allocbuf(m_max);

      for (DDS::ULong i = 0; i < m_length; i++)
      {
         m_buffer[i] = oldBuf[i];
         m_buffer[i]->_add_ref ();
      }

      if (m_release)
      {
         freebuf (oldBuf);
      }

      m_release = TRUE;
   }

   m_length = nelems;
}

template <class T> inline DDS::ULong DDS_DCPSUValSeq<T>::length () const
{
   return m_length;
}

template <class T> inline DDS::Boolean DDS_DCPSUValSeq<T>::release () const
{
   return m_release;
}

template <class T> inline DDSValueBase_mgr<T>
DDS_DCPSUValSeq<T>::operator [] (DDS::ULong index) const
{
   assert (index < m_length);
   return DDSValueBase_mgr<T> (m_buffer[index], FALSE);
}

template <class T> inline DDSValueBase_mgr<T>
DDS_DCPSUValSeq<T>::operator [] (DDS::ULong index)
{
   assert (index < m_length);
   return DDSValueBase_mgr<T> (m_buffer[index], m_release);
}

template <class T> inline T**
DDS_DCPSUValSeq<T>::get_buffer (DDS::Boolean orphan)
{
   T ** ret = 0;

   if (orphan)
   {
      if (m_release)
      {
         m_length = 0;
         m_release = TRUE;
         ret = m_buffer;
         m_buffer = 0;
      }
   }
   else
   {
      ret = m_buffer;
   }

   return ret;
}

template <class T> inline T* const * DDS_DCPSUValSeq<T>::get_buffer () const
{
   return m_buffer;
}

template <class T> inline void DDS_DCPSUValSeq<T>::replace
(
   DDS::ULong max,
   DDS::ULong len,
   T ** data,
   DDS::Boolean rel
)
{
   if (m_release)
   {
      freebuf (m_buffer);
   }

   m_max = max;
   m_length = len;
   m_buffer = data;
   m_release = rel;
}

#undef SACPP_API
#endif
