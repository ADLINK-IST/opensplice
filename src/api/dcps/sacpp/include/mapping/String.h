#ifndef SACPP_MAPPING_STRING_H
#define SACPP_MAPPING_STRING_H

#include "sacpp_if.h"
#include "mapping/Memory.h"

class SACPP_API DDS_DCPS::String_var
{
public:

   inline String_var ();
   inline String_var (char * p);
   inline String_var (const char * p);
   inline String_var (const String_var & s);
   inline virtual ~String_var ();

   inline String_var & operator = (const char * p);
   inline String_var & operator = (char * p);
   inline String_var & operator = (const String_var & s);

   inline operator char * ();
   inline operator const char * () const;

   inline char & operator [] (DDS_DCPS::ULong index);
   inline char operator [] (DDS_DCPS::ULong index) const;

   inline const char * in () const;
   inline DDS_DCPS::String & inout ();
   inline DDS_DCPS::String_out out ();
   inline DDS_DCPS::String _retn ();
   inline DDS_DCPS::String & val ();

   char * m_ptr;
   DDS_DCPS::Boolean m_rel;

protected:

   inline void reset (char * p);
};


/**
 * This class is used as the type of string members of structs
 * and arrays so the element can be initialized to the empty
 * string.
 */

class SACPP_API DDS_DCPS::String_mgr : public DDS_DCPS::String_var
{
public:

   inline String_mgr () { m_ptr = (char*) ""; m_rel = 0; };
   inline String_mgr (char * s) : String_var (s) {};
   inline String_mgr (const char * s) : String_var (s) {};
   inline String_mgr (const String_var & s)
      : String_var (s) {};
   inline String_mgr (const String_mgr & m)
      : String_var ((const String_var&) m) {};

   inline DDS_DCPS::String_var & operator = (const char * p);
   inline DDS_DCPS::String_var & operator = (char * p);
   inline DDS_DCPS::String_var & operator = (const String_var & s);
};


class SACPP_API DDS_DCPS::String_out
{
public:

   inline String_out (char *& p);
   inline String_out (String_var & s);
   inline String_out (const String_out & s);

   inline String_out& operator = (char * s);

   inline operator char *& ();
   inline char *& ptr ();

   char *& m_ptr;

private:

   // assignment from String_var disallowed

   inline void operator = (const String_var &) {}
};


class SACPP_API DDS_DCPS::String_for_seq
{
public:

   inline String_for_seq (DDS_DCPS::String & p, DDS_DCPS::Boolean rel)
      : m_pptr (&p), m_rel (rel) {};
   inline ~String_for_seq () {};

   inline DDS_DCPS::String_for_seq & operator = (const char * p);
   inline DDS_DCPS::String_for_seq & operator = (DDS_DCPS::String p);
   inline DDS_DCPS::String_for_seq & operator = (const DDS_DCPS::String_var &v);
   inline DDS_DCPS::String_for_seq & operator = (const DDS_DCPS::String_for_seq &s);

   inline operator char *& ();

   inline char & operator [] (DDS_DCPS::ULong index);
   inline char operator [] (DDS_DCPS::ULong index) const;

   inline const char * in () const;
   inline DDS_DCPS::String & inout ();
   inline DDS_DCPS::String_out out ();
   inline DDS_DCPS::String _retn ();

private:

   char ** m_pptr;
   DDS_DCPS::Boolean m_rel;
};


// ------------------------------------------------------------
//  inline implementations
// ------------------------------------------------------------

#include "eOrb/os/eorbiostream.h"

inline void DDS_DCPS::String_var::reset (char * p)
{
   if (m_rel && m_ptr != NULL)
   {
      DDS_DCPS::string_free (m_ptr);
   }
   m_rel = 1;
   m_ptr = p;
}

inline DDS_DCPS::String_var::String_var ()
   : m_ptr (nil), m_rel (1)
{}

inline DDS_DCPS::String_var::String_var (char * p)
   : m_ptr (p), m_rel (1)
{}

inline DDS_DCPS::String_var::String_var (const char * p)
   : m_ptr (DDS_DCPS::string_dup (p)), m_rel (1)
{}

inline DDS_DCPS::String_var::String_var (const DDS_DCPS::String_var & s)
   : m_rel (1)
{
   if (s.m_ptr != NULL)
   {
      m_ptr = DDS_DCPS::string_dup (s.m_ptr);
   }
   else
   {
      m_ptr = s.m_ptr;
   }
}

inline DDS_DCPS::String_var::~String_var ()
{
   if (m_rel && m_ptr != NULL)
   {
      DDS_DCPS::string_free (m_ptr);
   }
}

inline DDS_DCPS::String_var & DDS_DCPS::String_var::operator = (char * p)
{
   if (p != m_ptr)
   {
      reset (p);
   }

   return *this;
}

inline DDS_DCPS::String_var & DDS_DCPS::String_var::operator = (const char * p)
{
   if (p != m_ptr)
   {
      reset (DDS_DCPS::string_dup (p));
   }

   return *this;
}

inline DDS_DCPS::String_var & DDS_DCPS::String_var::operator =
   (const DDS_DCPS::String_var & s)
{
   if (s.m_ptr == NULL)
   {
      reset (s.m_ptr);
   }
   else
   {
      reset (DDS_DCPS::string_dup (s.m_ptr));
   }
   return *this;
}

inline DDS_DCPS::String_var::operator char * ()
{
   return m_ptr;
}

inline DDS_DCPS::String_var::operator const char* () const
{
   return m_ptr;
}

inline char& DDS_DCPS::String_var::operator[] (DDS_DCPS::ULong index)
{
   return m_ptr[index];
}

inline char DDS_DCPS::String_var::operator[] (DDS_DCPS::ULong index) const
{
   return m_ptr[index];
}

inline const char * DDS_DCPS::String_var::in () const
{
   return m_ptr;
}

inline DDS_DCPS::String & DDS_DCPS::String_var::inout ()
{
   return m_ptr;
}

inline DDS_DCPS::String_out DDS_DCPS::String_var::out ()
{
   return m_ptr;
}

inline DDS_DCPS::String DDS_DCPS::String_var::_retn ()
{
   DDS_DCPS::String ret = m_ptr;
   m_ptr = nil;
   return ret;
}

inline DDS_DCPS::String & DDS_DCPS::String_var::val ()
{
   return m_ptr;
}

inline DDS_DCPS::String_var & DDS_DCPS::String_mgr::operator = (const char * p)
{
   return DDS_DCPS::String_var::operator= (p);
}

inline DDS_DCPS::String_var & DDS_DCPS::String_mgr::operator = (char * p)
{
   return DDS_DCPS::String_var::operator= (p);
}

inline DDS_DCPS::String_var & DDS_DCPS::String_mgr::operator = (const DDS_DCPS::String_var & s)
{
   return DDS_DCPS::String_var::operator= (s);
}

inline DDS_DCPS::String_out::String_out (char*& p)
   : m_ptr (p)
{}

inline DDS_DCPS::String_out::String_out (DDS_DCPS::String_var& s)
   : m_ptr (s.m_ptr)
{
   DDS_DCPS::string_free (m_ptr);
   m_ptr = nil;
}

inline DDS_DCPS::String_out::String_out (const DDS_DCPS::String_out & s)
   : m_ptr (s.m_ptr)
{}

inline DDS_DCPS::String_out& DDS_DCPS::String_out::operator = (char * s)
{
   m_ptr = s;
   return *this;
}

inline DDS_DCPS::String_out::operator char *& ()
{
   return m_ptr;
}

inline char*& DDS_DCPS::String_out::ptr ()
{
   return m_ptr;
}

// Unbounded string sequence

class DDS_DCPSUStrSeq
{
public:

   inline static char ** allocbuf (DDS_DCPS::ULong);
   inline static void freebuf (char**);

   inline DDS_DCPSUStrSeq ();
   inline DDS_DCPSUStrSeq (DDS_DCPS::ULong);
   inline DDS_DCPSUStrSeq
   (
      DDS_DCPS::ULong max,
      DDS_DCPS::ULong len,
      char ** data,
      DDS_DCPS::Boolean rel = FALSE
   );
   inline DDS_DCPSUStrSeq (const DDS_DCPSUStrSeq&);
   inline ~DDS_DCPSUStrSeq ();

   DDS_DCPSUStrSeq & operator = (const DDS_DCPSUStrSeq&);

   inline DDS_DCPS::ULong maximum () const;

   void length (DDS_DCPS::ULong);
   inline DDS_DCPS::ULong length() const;

   inline DDS_DCPS::String_for_seq operator [] (DDS_DCPS::ULong index);
   inline const char * operator [] (DDS_DCPS::ULong index) const;

   inline DDS_DCPS::Boolean release () const;

   void replace
   (
      DDS_DCPS::ULong max,
      DDS_DCPS::ULong len,
      char ** data,
      DDS_DCPS::Boolean rel = FALSE
   );

   char ** get_buffer (DDS_DCPS::Boolean orphan = FALSE);
   char * const * get_buffer () const;

private:

   DDS_DCPS::ULong m_max;
   DDS_DCPS::ULong m_length;
   DDS_DCPS::Boolean m_release;
   char ** m_buffer;
};

template <typename X> class DDS_DCPSUStrSeqT: public DDS_DCPSUStrSeq
{
   public:
   inline DDS_DCPSUStrSeqT ()
   {
   }

   inline DDS_DCPSUStrSeqT (DDS_DCPS::ULong len)
      : DDS_DCPSUStrSeq (len)
   {
   }

   inline DDS_DCPSUStrSeqT
   (
      DDS_DCPS::ULong max,
      DDS_DCPS::ULong len,
      char ** data,
      DDS_DCPS::Boolean rel = FALSE
   )
      : DDS_DCPSUStrSeq (max, len, data, rel)
   {
   }

   inline DDS_DCPSUStrSeqT (const DDS_DCPSUStrSeq& that)
      : DDS_DCPSUStrSeq (that)
   {
   }
};

inline char ** DDS_DCPSUStrSeq::allocbuf (DDS_DCPS::ULong nelems)
{
   return (char**) DDS_DCPS_Memory::_vec_alloc (sizeof (char*), nelems);
}

inline void DDS_DCPSUStrSeq::freebuf (char ** buffer)
{
   if (buffer)
   {
      DDS_DCPS::ULong nelems = DDS_DCPS_Memory::_vec_size (buffer);

      for (DDS_DCPS::ULong i = 0; i < nelems; i++)
      {
         if (buffer[i])
         {
            DDS_DCPS::string_free (buffer[i]);
         }
      }

      DDS_DCPS_Memory::_vec_dealloc (buffer);
   }
}

inline DDS_DCPSUStrSeq::DDS_DCPSUStrSeq ()
:
   m_max (0),
   m_length (0),
   m_release (TRUE),
   m_buffer (0)
{}

inline DDS_DCPSUStrSeq::DDS_DCPSUStrSeq (DDS_DCPS::ULong nelems)
:
   m_max (nelems),
   m_length (0),
   m_release (TRUE),
   m_buffer (nelems ? allocbuf (nelems) : 0)
{
   for (DDS_DCPS::ULong i = 0; i < nelems; i++)
   {
      m_buffer[i] = DDS_DCPS::string_dup ("");
   }
}

inline DDS_DCPSUStrSeq::DDS_DCPSUStrSeq
(
   DDS_DCPS::ULong max,
   DDS_DCPS::ULong len,
   char ** data,
   DDS_DCPS::Boolean rel
)
:
   m_max (max),
   m_length (len),
   m_release (rel),
   m_buffer (data)
{
   assert (m_length <= m_max);
}

inline DDS_DCPSUStrSeq::DDS_DCPSUStrSeq (const DDS_DCPSUStrSeq & that)
:
   m_max (0),
   m_length (0),
   m_release (FALSE),
   m_buffer (0)
{
   *this = that;
}

inline DDS_DCPSUStrSeq::~DDS_DCPSUStrSeq ()
{
   if (m_release)
   {
      freebuf (m_buffer);
   }
}

inline DDS_DCPS::ULong DDS_DCPSUStrSeq::maximum () const
{
   return m_max;
}

inline DDS_DCPS::ULong DDS_DCPSUStrSeq::length () const
{
   return m_length;
}

inline DDS_DCPS::Boolean DDS_DCPSUStrSeq::release () const
{
   return m_release;
}

inline DDS_DCPS::String_for_seq DDS_DCPSUStrSeq::operator [] (DDS_DCPS::ULong index)
{
   assert (index < m_length);
   return DDS_DCPS::String_for_seq (m_buffer[index], m_release);
}

inline const char * DDS_DCPSUStrSeq::operator [] (DDS_DCPS::ULong index) const
{
   assert (index < m_length);
   return m_buffer[index];
}

inline char * const * DDS_DCPSUStrSeq::get_buffer () const
{
   return m_buffer;
}

template <class T> class DDS_DCPSUStrSeq_var
{

public:

   DDS_DCPSUStrSeq_var () : m_ptr (nil) {}

   DDS_DCPSUStrSeq_var (T * p) : m_ptr (p) {}

   DDS_DCPSUStrSeq_var (const DDS_DCPSUStrSeq_var<T> & that)
   {
      m_ptr = new T (*that.m_ptr);
   }

   ~DDS_DCPSUStrSeq_var ()
   {
      delete m_ptr;
   }

   DDS_DCPSUStrSeq_var<T>& operator= (T * p)
   {
      delete m_ptr;
      m_ptr = p;
      return *this;
   }

   DDS_DCPSUStrSeq_var<T>& operator= (const DDS_DCPSUStrSeq_var<T>& that)
   {
      *m_ptr = *that.m_ptr;
      return *this;
   }

   DDS_DCPS::String_for_seq operator[] (DDS_DCPS::ULong index)
   {
      return ((*m_ptr)[index]);
   }

   const char * operator[] (DDS_DCPS::ULong index) const
   {
      return (char*&)(*m_ptr)[index];
   }

   DDS_DCPSUStrSeq * operator-> ()
   {
      return m_ptr;
   }

   operator T & ()
   {
      return *m_ptr;
   }

   operator const T & () const
   {
      return *m_ptr;
   }

   operator T *()
   {
      return m_ptr;
   }

   operator const T * () const
   {
      return m_ptr;
   }

   const T & in ()
   {
      return *m_ptr;
   }

   T & inout ()
   {
      return *m_ptr;
   }

   T *& out ()
   {
      return m_ptr;
   }

   T * _retn ()
   {
      T * ret = m_ptr;

      m_ptr = nil;
      return ret;
   }

   T *& val ()
   {
      return m_ptr;
   }

public:

   T * m_ptr;
};

template <class T> class DDS_DCPSUStrSeq_out
{
public:

   DDS_DCPSUStrSeq_out (T *& p) : m_ptr (p) {}

   DDS_DCPSUStrSeq_out (DDS_DCPSUStrSeq_var<T> & v)
      : m_ptr (v.m_ptr)
   {
      delete m_ptr;
   }

   DDS_DCPSUStrSeq_out (const DDS_DCPSUStrSeq_out<T> & v)
      : m_ptr (v.m_ptr)
   {}

   DDS_DCPSUStrSeq_out<T>& operator = (T * p)
   {
      m_ptr = p;
      return *this;
   }

   T * operator -> ()
   {
      return m_ptr;
   }

   T *& out ()
   {
      return m_ptr;
   }

   operator T *& ()
   {
      return m_ptr;
   }

public:

   T *& m_ptr;
};

inline void DDS_DCPSUStrSeq::length (DDS_DCPS::ULong nelems)
{     
   if (nelems > m_max)
   {
      DDS_DCPS::ULong i = 0;
      char ** old = m_buffer;
   
      m_max = nelems;
      m_buffer = allocbuf (m_max);
      
      while (i < m_length)
      {  
         if (m_release)
         {
            m_buffer[i] = old[i];
            old[i] = 0;
         }
         else
         {
            m_buffer[i] = DDS_DCPS::string_dup (old[i]);
         }
         i++;
      }
      while (i < m_max)
      {
         m_buffer[i] = DDS_DCPS::string_dup (""); 
         i++;
      }
   
      if (m_release)
      {
         freebuf (old);
      }
   
      m_release = TRUE;
   }
   
   m_length = nelems;
}  

inline DDS_DCPSUStrSeq & DDS_DCPSUStrSeq::operator = (const DDS_DCPSUStrSeq & that)
{  
   if (this != &that)
   {  
      DDS_DCPS::ULong i = 0;
      if (m_release)
      {  
         freebuf (m_buffer);
      }  
         
      m_max = that.m_max;
      m_length = that.m_length;
      m_release = TRUE;
      m_buffer = allocbuf (m_max);
      
      while (i < m_length)
      {
         m_buffer[i] = DDS_DCPS::string_dup (that.m_buffer[i]);
         i++;
      }
      while (i < m_max)
      {
         m_buffer[i] = DDS_DCPS::string_dup ("");
         i++;
      }
   }
   
   return *this;
}  

inline DDS_DCPS::String_for_seq & DDS_DCPS::String_for_seq::operator = (DDS_DCPS::String p)
{
   assert (m_pptr);

   if (m_rel && (*m_pptr) && (*m_pptr != p))
   {
      DDS_DCPS::string_free (*m_pptr);
   }

   *m_pptr = p;
   m_rel = TRUE;

   return *this;
}

inline DDS_DCPS::String_for_seq & DDS_DCPS::String_for_seq::operator = (const char * p)
{
   assert (m_pptr);

   if (m_rel && (*m_pptr) && (*m_pptr != p))
   {
      DDS_DCPS::string_free (*m_pptr);
   }

   *m_pptr = DDS_DCPS::string_dup (p);
   m_rel = TRUE;

   return *this;
}

inline DDS_DCPS::String_for_seq & DDS_DCPS::String_for_seq::operator = (const DDS_DCPS::String_var &v)
{
   assert (m_pptr);

   if (m_rel && (*m_pptr))
   {
      DDS_DCPS::string_free (*m_pptr);
   }

   *m_pptr = DDS_DCPS::string_dup (v.in ());
   m_rel = TRUE;

   return *this;
}

inline DDS_DCPS::String_for_seq & DDS_DCPS::String_for_seq::operator = (const DDS_DCPS::String_for_seq &s)
{
   if (&s != this)
   {
      assert (m_pptr);

      if (m_rel && (*m_pptr))
      {
         DDS_DCPS::string_free (*m_pptr);
      }

      *m_pptr = DDS_DCPS::string_dup (s.in ());
      m_rel = TRUE;
   }

   return *this;
}

inline DDS_DCPS::String_for_seq::operator char *& ()
{
   return *m_pptr;
}

inline char & DDS_DCPS::String_for_seq::operator [] (DDS_DCPS::ULong index)
{
   return (*m_pptr)[index];
}

inline char DDS_DCPS::String_for_seq::operator [] (DDS_DCPS::ULong index) const
{
   return (*m_pptr)[index];
}

inline const char * DDS_DCPS::String_for_seq::in () const
{
   assert (m_pptr);
   return *m_pptr;
}

inline DDS_DCPS::String & DDS_DCPS::String_for_seq::inout ()
{
   assert (m_pptr);
   return *m_pptr;
}

inline DDS_DCPS::String_out DDS_DCPS::String_for_seq::out ()
{
   assert (m_pptr);

   if (m_rel && (*m_pptr))
   {
      DDS_DCPS::string_free (*m_pptr);
   }

   *m_pptr = nil;
   return *m_pptr;
}

inline DDS_DCPS::String DDS_DCPS::String_for_seq::_retn ()
{
   assert (m_pptr);
   DDS_DCPS::String result = *m_pptr;
   *m_pptr = nil;
   return result;
}

#undef SACPP_API
#endif /* SACPP_MAPPING_STRING_H */
