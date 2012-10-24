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

#ifndef SACPP_MAPPING_BFL_H
#define SACPP_MAPPING_BFL_H

#include "sacpp_if.h"
// Bounded fixed-length sequence

template <class T, class X, DDS::ULong max> class DDS_DCPSBFLSeq
{
public:

   typedef T& _subscript_type;
   typedef const T& _const_subscript_type;

   // DDS-Standard
   static T * allocbuf (DDS::ULong n);
   static void freebuf (T* p);

   DDS_DCPSBFLSeq ();
   DDS_DCPSBFLSeq (DDS::ULong, T*, DDS::Boolean = FALSE);
   DDS_DCPSBFLSeq (const DDS_DCPSBFLSeq<T, X, max>&);
   ~DDS_DCPSBFLSeq ();

   DDS_DCPSBFLSeq<T, X, max> & operator= (const DDS_DCPSBFLSeq<T, X, max>&);

   DDS::ULong maximum () const;

   void length (DDS::ULong);
   DDS::ULong length () const;

   T & operator[] (DDS::ULong index);
   const T & operator[] (DDS::ULong index) const;

   DDS::Boolean release () const;
   void replace
   (
      DDS::ULong length,
      T * data,
      DDS::Boolean rel = FALSE
   );

   T * get_buffer (DDS::Boolean orphan = FALSE);
   const T * get_buffer () const;

private:

   DDS::ULong m_length;
   DDS::Boolean m_release;
   T * m_buffer;
};

template <class T, class X, DDS::ULong max> inline T*
   DDS_DCPSBFLSeq<T, X, max>::allocbuf (DDS::ULong nelems)
{
   return new T [nelems];
}

template <class T, class X, DDS::ULong max> inline void
DDS_DCPSBFLSeq<T, X, max>::freebuf (T* buffer)
{
   delete [] buffer;
}

template <class T, class X, DDS::ULong max> inline
   DDS_DCPSBFLSeq<T, X, max>::DDS_DCPSBFLSeq ()
:
   m_length (0),
   m_release (TRUE),
   m_buffer (allocbuf(max))
{}

template <class T, class X, DDS::ULong max> inline
   DDS_DCPSBFLSeq<T, X, max>::DDS_DCPSBFLSeq (DDS::ULong len, T* data, DDS::Boolean rel)
:
   m_length (len),
   m_release (rel),
   m_buffer (data)
{
   assert (len <= max);
}

template <class T, class X, DDS::ULong max> inline
   DDS_DCPSBFLSeq<T, X, max>::DDS_DCPSBFLSeq (const DDS_DCPSBFLSeq<T, X, max>& that)
:
   m_length (0),
   m_release (TRUE),
   m_buffer (allocbuf (max))
{
   *this = that;
}

template <class T, class X, DDS::ULong max> inline
   DDS_DCPSBFLSeq<T, X, max>::~DDS_DCPSBFLSeq ()
{
   if (m_release)
   {
      freebuf (m_buffer);
   }
}

template <class T, class X, DDS::ULong max> inline DDS_DCPSBFLSeq<T, X, max> &
DDS_DCPSBFLSeq<T, X, max>::operator= (const DDS_DCPSBFLSeq<T, X, max>& that)
{
   if (this != &that)
   {
      m_length = that.m_length;

      if (m_length)
      {
         memcpy (m_buffer, that.m_buffer, m_length * sizeof (T));
      }
   }

   return *this;
}

template <class T, class X, DDS::ULong max> inline void
DDS_DCPSBFLSeq<T, X, max>::length (DDS::ULong nelems)
{
   assert (nelems <= max);
   m_length = nelems;
}

template <class T, class X, DDS::ULong max> inline DDS::ULong
DDS_DCPSBFLSeq<T, X, max>::length () const
{
   return m_length;
}

template <class T, class X, DDS::ULong max> inline DDS::Boolean
DDS_DCPSBFLSeq<T, X, max>::release () const
{
   return m_release;
}

template <class T, class X, DDS::ULong max> inline DDS::ULong
DDS_DCPSBFLSeq<T, X, max>::maximum () const
{
   return max;
}

template <class T, class X, DDS::ULong max> inline T&
DDS_DCPSBFLSeq<T, X, max>::operator[] (DDS::ULong index)
{
   assert (index < m_length);
   return m_buffer[index];
}

template <class T, class X, DDS::ULong max> inline const T&
DDS_DCPSBFLSeq<T, X, max>::operator[] (DDS::ULong index) const
{
   assert (index < m_length);
   return m_buffer[index];
}

template <class T, class X, DDS::ULong max> inline T*
DDS_DCPSBFLSeq<T, X, max>::get_buffer (DDS::Boolean orphan)
{
   T * ret = 0;

   if (orphan)
   {
      if (m_release)
      {
         m_length = 0;
         m_release = TRUE;
         ret = m_buffer;
         m_buffer = allocbuf (max);
      }
   }
   else
   {
      ret = m_buffer;
   }

   return ret;
}

template <class T, class X, DDS::ULong max> inline const T*
DDS_DCPSBFLSeq<T, X, max>::get_buffer () const
{
   return m_buffer;
}

template <class T, class X, DDS::ULong max> inline void
DDS_DCPSBFLSeq<T, X, max>::replace
(
   DDS::ULong length,
   T * data,
   DDS::Boolean rel
)
{
   if (m_release)
   {
      freebuf (m_buffer);
   }

   m_length = length;
   m_buffer = data;
   m_release = rel;
}

#undef SACPP_API
#endif
