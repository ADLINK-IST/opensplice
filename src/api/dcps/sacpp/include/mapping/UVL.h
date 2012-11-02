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
#ifndef _SACPP_MAPPING_UVL_H_
#define _SACPP_MAPPING_UVL_H_

#include "sacpp_if.h"
// Unbounded variable-length sequence

template <class T, typename X> class DDS_DCPSUVLSeq
{
public:

   typedef T& _subscript_type;
   typedef const T& _const_subscript_type;

   static T * allocbuf (DDS::ULong nelems);
   static void freebuf (T * buffer);

   DDS_DCPSUVLSeq ();
   DDS_DCPSUVLSeq (DDS::ULong);
   DDS_DCPSUVLSeq
   (
      DDS::ULong max,
      DDS::ULong len,
      T* data,
      DDS::Boolean rel = FALSE
   );
   DDS_DCPSUVLSeq (const DDS_DCPSUVLSeq<T, X>&);
   ~DDS_DCPSUVLSeq ();

   DDS_DCPSUVLSeq<T, X> & operator = (const DDS_DCPSUVLSeq<T, X>&);

   DDS::ULong maximum () const;

   void length (DDS::ULong);
   DDS::ULong length () const;

   T & operator [] (DDS::ULong index);
   const T & operator [] (DDS::ULong index) const;

   DDS::Boolean release () const;
   void replace
   (
      DDS::ULong max,
      DDS::ULong len,
      T * data,
      DDS::Boolean rel = FALSE
   );

   T * get_buffer (DDS::Boolean orphan = FALSE);
   const T * get_buffer () const;

private:

   DDS::ULong m_max;
   DDS::ULong m_length;
   DDS::Boolean m_release;
   T * m_buffer;
};


template <class T, typename X> void DDS_DCPSUVLSeq<T, X>::replace
(
   DDS::ULong max,
   DDS::ULong len,
   T * data,
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

template <class T, typename X>
inline T * DDS_DCPSUVLSeq<T, X>::allocbuf (DDS::ULong nelems)
{
   return new T [nelems];
}

template <class T, typename X>
inline void DDS_DCPSUVLSeq<T, X>::freebuf (T * buffer)
{
   if (buffer)
   {
      delete [] buffer;
   }
}

template <class T, typename X> inline DDS_DCPSUVLSeq<T, X>::DDS_DCPSUVLSeq ()
:
   m_max (0),
   m_length (0),
   m_release (FALSE),
   m_buffer (0)
{}

template <class T, typename X>
inline DDS_DCPSUVLSeq<T, X>::DDS_DCPSUVLSeq (DDS::ULong max)
:
   m_max (max),
   m_length (0),
   m_release (TRUE),
   m_buffer (allocbuf (max))
{}

template <class T, typename X> inline DDS_DCPSUVLSeq<T, X>::DDS_DCPSUVLSeq
(
   DDS::ULong max,
   DDS::ULong len,
   T* data,
   DDS::Boolean rel
)
:
   m_max (max),
   m_length (len),
   m_release (rel),
   m_buffer (data)
{
   assert (m_length <= m_max);
}

template <class T, typename X>
inline DDS_DCPSUVLSeq<T, X>::DDS_DCPSUVLSeq (const DDS_DCPSUVLSeq<T, X>& that)
:
   m_max (0),
   m_length (0),
   m_release (FALSE),
   m_buffer (0)
{
   *this = that;
}

template <class T, typename X> inline DDS_DCPSUVLSeq<T, X>::~DDS_DCPSUVLSeq ()
{
   if (m_release)
   {
      freebuf (m_buffer);
   }
}

template <class T, typename X>
inline DDS_DCPSUVLSeq<T, X> & DDS_DCPSUVLSeq<T, X>::operator =
(
   const DDS_DCPSUVLSeq<T, X>& that
)
{
   if (this != &that)
   {
      if (that.m_max > m_max)
      {
         if (m_release)
         {
            freebuf (m_buffer);
         }

         m_max = that.m_max;
         m_length = that.m_length;
         m_buffer = allocbuf (m_max);
         m_release = TRUE;
      }
      else
      {
         m_length = that.m_length;
      }

      for (DDS::ULong i = 0; i < m_length; i++)
      {
         m_buffer[i] = that.m_buffer[i];
      }
   }

   return *this;
}

template <class T, typename X>
inline DDS::ULong DDS_DCPSUVLSeq<T, X>::maximum () const
{
   return m_max;
}

template <class T, typename X>
inline void DDS_DCPSUVLSeq<T, X>::length (DDS::ULong nelems)
{
   if (nelems > m_max)
   {
      T * oldBuf = m_buffer;

      m_max = nelems;
      m_buffer = allocbuf (m_max);

      for (DDS::ULong i = 0; i < m_length; i++)
      {
         m_buffer[i] = oldBuf[i];
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
inline DDS::ULong DDS_DCPSUVLSeq<T, X>::length () const
{
   return m_length;
}

template <class T, typename X>
inline DDS::Boolean DDS_DCPSUVLSeq<T, X>::release () const
{
   return m_release;
}

template <class T, typename X>
inline T& DDS_DCPSUVLSeq<T, X>::operator [] (DDS::ULong index)
{
   assert (index < m_length);
   return m_buffer[index];
}

template <class T, typename X> inline const T& DDS_DCPSUVLSeq<T, X>::operator []
(
   DDS::ULong index
) const
{
   assert (index < m_length);
   return m_buffer[index];
}

template <class T, typename X>
inline T* DDS_DCPSUVLSeq<T, X>::get_buffer (DDS::Boolean orphan)
{
   T * ret = 0;

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

template <class T, typename X>
inline const T* DDS_DCPSUVLSeq<T, X>::get_buffer () const
{
   return m_buffer;
}

#undef SACPP_API
#endif
