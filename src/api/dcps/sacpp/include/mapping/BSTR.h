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
#ifndef SACPP_MAPPING_BSTR_H
#define SACPP_MAPPING_BSTR_H

#include "mapping/String.h"

// Bounded string sequence
template <DDS::ULong max> class DDS_DCPSBStrSeq
{
public:

   typedef DDS::String_for_seq _subscript_type;
   typedef const char * _const_subscript_type;

   // CORBA-Standard APIs

   static char ** allocbuf (DDS::ULong nelems);
   static void freebuf (char ** buffer);

   DDS_DCPSBStrSeq ();
   DDS_DCPSBStrSeq (DDS::ULong, char**, DDS::Boolean = FALSE);
   DDS_DCPSBStrSeq (const DDS_DCPSBStrSeq<max>&);
   ~DDS_DCPSBStrSeq ();

   DDS_DCPSBStrSeq<max>& operator = (const DDS_DCPSBStrSeq<max>&);

   DDS::ULong maximum () const;

   void length (DDS::ULong);
   DDS::ULong length () const;

   DDS::String_for_seq operator [] (DDS::ULong index);
   const char * operator [] (DDS::ULong index) const;

   DDS::Boolean release () const;
   void replace
   (
      DDS::ULong length,
      char ** data,
      DDS::Boolean rel = FALSE
   );

   char ** get_buffer (DDS::Boolean orphan = FALSE);
   char * const * get_buffer () const;

private:

   DDS::ULong m_length;
   DDS::Boolean m_release;
   char ** m_buffer;
};

template <DDS::ULong max> inline char **
   DDS_DCPSBStrSeq<max>::allocbuf (DDS::ULong nelems)
{
   char ** buffer = (char**) DDS_DCPS_Memory::_vec_alloc (sizeof (char*), nelems);

   for (DDS::ULong i = 0; i < nelems; i++)
   {
      buffer[i] = 0;
   }

   return buffer;
}

template <DDS::ULong max> inline void 
   DDS_DCPSBStrSeq<max>::freebuf (char** buffer)
{
   if (buffer)
   {
      DDS::ULong nelems = DDS_DCPS_Memory::_vec_size (buffer);

      for (DDS::ULong i = 0; i < nelems; i++)
      {
         DDS::string_free (buffer[i]);
      }
      DDS_DCPS_Memory::_vec_dealloc (buffer);
   }
}

template <DDS::ULong max> inline DDS_DCPSBStrSeq<max>::DDS_DCPSBStrSeq ()
:
   m_length (0),
   m_release (TRUE)
{
   m_buffer = allocbuf (max);
}

template <DDS::ULong max> inline DDS_DCPSBStrSeq<max>::DDS_DCPSBStrSeq
(
   DDS::ULong len,
   char ** data,
   DDS::Boolean rel
)
:
   m_length (len),
   m_release (rel),
   m_buffer (data)
{
   assert (m_length <= max);
}

template <DDS::ULong max> inline DDS_DCPSBStrSeq<max>::DDS_DCPSBStrSeq
(
   const DDS_DCPSBStrSeq<max>& that
)
:
   m_length (0),
   m_release (FALSE),
   m_buffer (0)
{
   *this = that;
}

template <DDS::ULong max> inline DDS_DCPSBStrSeq<max>::~DDS_DCPSBStrSeq ()
{
   if (m_release)
   {
      freebuf (m_buffer);
   }
}

template <DDS::ULong max> inline DDS_DCPSBStrSeq<max>& DDS_DCPSBStrSeq<max>::operator=
(
   const DDS_DCPSBStrSeq<max>& that
)
{
   if (this != &that)
   {
      if (m_release)
      {
         freebuf (m_buffer);
      }

      m_length = that.m_length;
      m_release = TRUE;
      m_buffer = allocbuf (max);

      for (DDS::ULong i = 0; i < m_length; i++)
      {
         m_buffer[i] = DDS::string_dup (that.m_buffer[i]);
      }
   }

   return *this;
}

template <DDS::ULong max> inline void DDS_DCPSBStrSeq<max>::length
   (DDS::ULong nelems)
{
   assert (nelems <= max);
   m_length = nelems;
}

template <DDS::ULong max> inline DDS::ULong DDS_DCPSBStrSeq<max>::length () const
{
   return m_length;
}

template <DDS::ULong max> inline DDS::ULong DDS_DCPSBStrSeq<max>::maximum () const
{
   return max;
}

template <DDS::ULong max> inline DDS::Boolean DDS_DCPSBStrSeq<max>::release () const
{
   return m_release;
}

template <DDS::ULong max> inline DDS::String_for_seq
   DDS_DCPSBStrSeq<max>::operator[] (DDS::ULong index)
{
   assert (index < m_length);
   return DDS::String_for_seq (m_buffer[index], m_release);
}

template <DDS::ULong max> inline const char*
   DDS_DCPSBStrSeq<max>::operator[] (DDS::ULong index) const
{
   assert (index < m_length);
   return m_buffer[index];
}

template <DDS::ULong max> inline char **
   DDS_DCPSBStrSeq<max>::get_buffer (DDS::Boolean orphan)
{
   char ** ret = 0;

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

template <DDS::ULong max> inline char * const *
   DDS_DCPSBStrSeq<max>::get_buffer () const
{
   return m_buffer;
}

template <DDS::ULong max> inline void DDS_DCPSBStrSeq<max>::replace
(
   DDS::ULong length,
   char ** data,
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

   assert (m_length <= max);
}

#endif
