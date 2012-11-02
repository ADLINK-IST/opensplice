#ifndef _DDS_XPS_STRING_H_
#define _DDS_XPS_STRING_H_

#include "cppgen_iostream.h"
#include "sacpp_DDS_DCPS.h"
#include "StdString.h"

class be_CppName;

// An DDSString is the same as an DDS_StdString, but with additional
// tailorings for IDLC.
//
// Also, the name is easier to type.

class DDSString
{

public:
   DDSString() : m_rep(new stringrep(NULL))
   { }

   DDSString(const char* s) : m_rep(new stringrep(s))
   { }

   DDSString(const char* s, const char* t) : m_rep(new stringrep(s, t))
   { }

   DDSString(const DDSString& str);
   /*****
    DDSString(int i) : m_rep(IntToString(i)) { }
    DDSString(DDS::ULong n) : m_rep(ULongToString(n)) { }
   *****/

   virtual ~DDSString();

   // CONVERSION CONSTRUCTORS

   DDSString(const DDS_StdString& stdstr)
         : m_rep(new stringrep((const char*)stdstr))
   { }


   // ACCESS FUNCTIONS

   char& operator[](int) const;

   inline unsigned long length() const
   {
      return stringrep::Length(m_rep->m_string);
   }

   // CONVERSION OPERATORS

   inline operator const char*() const
   {
      if (!m_rep->m_string)
         return "";
      else
         return m_rep->m_string;
   }

   inline operator DDS_StdString() const
   {
      return DDS_StdString(m_rep->m_string);
   }

   // CONCATENATION OPERATORS

   friend DDSString operator+(const DDSString& head, const DDSString& tail);

   // NUMERIC CONVERSION OPERATORS

   inline DDSString operator+(int i)
   {
      return *this += i;
   }

   inline DDSString operator+(DDS::ULong i)
   {
      return *this += i;
   }

   // ASSIGNMENT OPERATORS

   DDSString& operator=(const char* str);
   DDSString& operator=(const DDSString& str);

   inline DDSString& operator=(const DDS_StdString& stdstr)
   {
      return operator=((const char*)stdstr);
   }

   inline DDSString& operator=(const be_CppName& cppName)
   {
      return operator=((DDSString&)cppName);
   }

   inline DDSString& operator=(int i)
   {
      return * this = DDSString(IntToString(i));
   }

   inline DDSString& operator=(DDS::ULong i)
   {
      return * this = DDSString(ULongToString(i));
   }

   // += OPERATORS

   DDSString& operator+=(const DDSString& str);

   inline DDSString& operator+=(int i)
   {
      return * this += DDSString(IntToString(i));
   }

   inline DDSString& operator+=(DDS::ULong i)
   {
      return * this += DDSString(ULongToString(i));
   }

   // EQUALITY OPERATORS

   friend bool operator==(const DDSString& str1, const char* str2);
   friend bool operator==(const DDSString& str1, const DDSString& str2);
   friend bool operator!=(const DDSString& str1, const char* str2);
   friend bool operator!=(const DDSString& str1, const DDSString& str2);
   friend bool operator==(const char* str1, const DDSString& str2);
   friend bool operator!=(const char* str1, const DDSString& str2);

   friend ostream& operator<<(ostream& os, const DDSString& str);

private:

   struct stringrep
   {
      char* m_string;
      Counter m_refs;

      stringrep(const char* s) : m_string(NewString(s)), m_refs(1)
      { }

      stringrep(const char* s, const char* t)  // concatenate
            :
            m_string(NewString(s, t)), m_refs(1)
      { }

      inline static char* NewString(const char* s)
      {
         char* result = NULL;
         int len = Length(s);

         if (len)
         {
            result = new char[len + 1];
            os_strcpy(result, s);
         }

         return result;
      }

      inline static char* NewString(const char* s, const char* t) // concatenate
      {
         int len = Length(s, t);

         if (len == 0)
         {
            return NULL;
         }

         char* result = new char[len + 1];

         if (s)
         {
            os_strcpy(result, s);

            if (t)
            {
               os_strcat(result, t);
            }
         }
         else
         {
            assert (t != NULL);
            os_strcpy(result, t);
         }

         return result;
      }

      inline void SetNewString(const char* s)
      {
         assert ((int)m_refs == 0);
         m_string = NewString(s);
         m_refs = 1;
      }

      // result of Length() functions does not include terminating null

      inline static int Length(const char* s)  // zero length if NULL
      {
         return s ? strlen(s) : 0;
      }

      inline static int Length(const char* s, const char* t)
      {
         return Length(s) + Length(t);
      }
   };

   stringrep* m_rep;

   // PRIVATE HELPERS

   DDSString(stringrep* rep) : m_rep(rep)
   { }

   inline void RemoveReference()
   {
      if (m_rep->m_refs-- == 0)
      {
         delete[] m_rep->m_string;
      }
   }

   inline void AbandonRep()
   {
      if (m_rep->m_refs-- == 0)
      {
         delete[] m_rep->m_string;
         delete m_rep;
      }
   }

   inline void CopyRep(stringrep* rep)
   {
      m_rep = rep;
      m_rep->m_refs++;
   }

   static inline bool Streq(const char* s, const char* t)
   {
      if (s && t)
      {
         return strcmp(s, t) == 0;
      }
      else
      {
         return s == t;
      }
   }

   static stringrep* IntToString(int i);
   static stringrep* ULongToString(DDS::ULong i);
};

inline bool
operator==(const DDSString& str, const char* s)
{
   return DDSString::Streq(str.m_rep->m_string, s);
}

inline bool
operator==(const DDSString& str1, const DDSString& str2)
{
   return DDSString::Streq(str1.m_rep->m_string, str2.m_rep->m_string);
}

inline bool
operator!=(const DDSString& str, const char* s)
{
   return !DDSString::Streq(str.m_rep->m_string, s);
}

inline bool
operator!=(const DDSString& str1, const DDSString& str2)
{
   return !DDSString::Streq(str1.m_rep->m_string, str2.m_rep->m_string);
}

inline bool
operator==(const char* s, const DDSString& str)
{
   return DDSString::Streq(str.m_rep->m_string, s);
}

inline bool
operator!=(const char* s, const DDSString& str)
{
   return !DDSString::Streq(str.m_rep->m_string, s);
}

// COMPARISON WITH DDS_StdString

inline bool
operator==(const DDS_StdString& stdstr, const DDSString& str)
{
   return (const char*)stdstr == str;
}

inline bool
operator!=(const DDS_StdString& stdstr, const DDSString& str)
{
   return (const char*)stdstr != str;
}

inline bool
operator==(const DDSString& str, const DDS_StdString& stdstr)
{
   return (const char*)stdstr == str;
}

inline bool
operator!=(const DDSString& str, const DDS_StdString& stdstr)
{
   return (const char*)stdstr != str;
}

// STREAMING

inline ostream& operator<<(ostream& os, const DDSString & str)
{
   os << (const char*)str;
   return os;
}

#endif  // _XPS_STRING_H
