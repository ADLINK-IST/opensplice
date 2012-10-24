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
#ifndef SACPP_MAPPING_UOBJ_H
#define SACPP_MAPPING_UOBJ_H

#include "sacpp_if.h"

// Unbounded object reference sequence

template <class T, typename X> class DDS_DCPSUObjSeq
{
public:

   typedef DDS_DCPSInterface_mgr<T> _subscript_type;
   typedef const DDS_DCPSInterface_mgr<T> _const_subscript_type;

   static T ** allocbuf (DDS::ULong);
   static void freebuf (T**);

   DDS_DCPSUObjSeq ();
   DDS_DCPSUObjSeq (DDS::ULong);
   DDS_DCPSUObjSeq
   (
      DDS::ULong max,
      DDS::ULong len,
      T** data,
      DDS::Boolean rel = FALSE
   );
   DDS_DCPSUObjSeq (const DDS_DCPSUObjSeq<T, X>&);
   ~DDS_DCPSUObjSeq ();

   DDS_DCPSUObjSeq<T, X>& operator = (const DDS_DCPSUObjSeq<T, X>&);

   DDS::ULong maximum () const;

   void length (DDS::ULong);
   DDS::ULong length () const;

   DDS_DCPSInterface_mgr<T> operator [] (DDS::ULong index) const;
   DDS_DCPSInterface_mgr<T> operator [] (DDS::ULong index);

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

template <class T, typename X>
inline T ** DDS_DCPSUObjSeq<T, X>::allocbuf (DDS::ULong nelems)
{
   T ** buffer = (T**) DDS_DCPS_Memory::_vec_alloc (sizeof (T*), nelems);

   for (DDS::ULong i = 0; i < nelems; i++)
   {
      buffer[i] = 0;
   }

   return buffer;
}

template <class T, typename X> inline void DDS_DCPSUObjSeq<T, X>::freebuf (T ** buffer)
{
   if (buffer)
   {
      DDS::ULong nelems = DDS_DCPS_Memory::_vec_size (buffer);

      for (DDS::ULong i = 0; i < nelems; i++)
      {
         DDS::release (buffer[i]);
      }

      DDS_DCPS_Memory::_vec_dealloc (buffer);
   }
}

template <class T, typename X> inline DDS_DCPSUObjSeq<T, X>::DDS_DCPSUObjSeq ()
:
   m_max (0),
   m_length (0),
   m_buffer (0),
   m_release (FALSE)
{}

template <class T, typename X>
inline DDS_DCPSUObjSeq<T, X>::DDS_DCPSUObjSeq (DDS::ULong nelems)
:
   m_max (nelems),
   m_length (0),
   m_buffer (allocbuf (m_max)),
   m_release (TRUE)
{}

template <class T, typename X> inline DDS_DCPSUObjSeq<T, X>::DDS_DCPSUObjSeq
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

template <class T, typename X>
inline DDS_DCPSUObjSeq<T, X>::DDS_DCPSUObjSeq (const DDS_DCPSUObjSeq<T, X>& that)
:
   m_max (0),
   m_length (0),
   m_buffer (0),
   m_release (FALSE)
{
   *this = that;
}

template <class T, typename X> inline DDS_DCPSUObjSeq<T, X>::~DDS_DCPSUObjSeq ()
{
   if (m_release)
   {
      freebuf (m_buffer);
   }
}

template <class T, typename X>
inline DDS_DCPSUObjSeq<T, X>& DDS_DCPSUObjSeq<T, X>::operator =
(
   const DDS_DCPSUObjSeq<T, X>& that
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
         m_buffer[i] = T::_duplicate (that.m_buffer[i]);
      }
   }

   return *this;
}

template <class T, typename X>
inline DDS::ULong DDS_DCPSUObjSeq<T, X>::maximum () const
{
   return m_max;
}

template <class T, typename X>
inline void DDS_DCPSUObjSeq<T, X>::length (DDS::ULong nelems)
{
   if (nelems > m_max)
   {
      T** oldBuf = m_buffer;

      m_max = nelems;
      m_buffer = allocbuf (m_max);

      for (DDS::ULong i = 0; i < m_length; i++)
      {
         m_buffer[i] = T::_duplicate (oldBuf[i]);
      }

      if (m_release)
      {
         freebuf (oldBuf);
      }

      m_release = TRUE;
   }

   m_length = nelems;
}

template <class T, typename X>
inline DDS::ULong DDS_DCPSUObjSeq<T, X>::length () const
{
   return m_length;
}

template <class T, typename X>
inline DDS::Boolean DDS_DCPSUObjSeq<T, X>::release () const
{
   return m_release;
}

template <class T, typename X> inline DDS_DCPSInterface_mgr<T>
DDS_DCPSUObjSeq<T, X>::operator [] (DDS::ULong index) const
{
   assert (index < m_length);
   return DDS_DCPSInterface_mgr<T> (m_buffer[index], FALSE);
}

template <class T, typename X> inline DDS_DCPSInterface_mgr<T>
DDS_DCPSUObjSeq<T, X>::operator [] (DDS::ULong index)
{
   assert (index < m_length);
   return DDS_DCPSInterface_mgr<T> (m_buffer[index], m_release);
}

template <class T, typename X> inline T**
DDS_DCPSUObjSeq<T, X>::get_buffer (DDS::Boolean orphan)
{
   T** ret = NULL;

   if (orphan)
   {
      if (m_release)
      {
         m_length = 0;
         m_release = TRUE;
         ret = m_buffer;
         m_buffer = NULL;
      }
   }
   else
   {
      ret = m_buffer;
   }

   return ret;
}

template <class T, typename X>
inline T* const * DDS_DCPSUObjSeq<T, X>::get_buffer () const
{
   return m_buffer;
}

template <class T, typename X> inline void DDS_DCPSUObjSeq<T, X>::replace
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
#endif /* SACPP_MAPPING_UOBJ_H */
