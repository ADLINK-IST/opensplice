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
#ifndef _DDS_STDSTRING_H_
#define _DDS_STDSTRING_H_

#include "cppgen_string.h"
#include "cppgen_iostream.h"
#include "cppgen_counter.h"
#include "sacpp_dds_basic_types.h"

class DDS_StdString
{
public:

   DDS_StdString ();
   DDS_StdString (const char * str);
   DDS_StdString (const DDS_StdString & str);
   ~DDS_StdString ();

   friend DDS_StdString operator + (const DDS_StdString & head, const char * tail);
   friend DDS_StdString operator + (const DDS_StdString & head, const DDS_StdString & tail);

   friend bool operator == (const DDS_StdString & str1, const char * str2);
   friend bool operator == (const DDS_StdString & str1, const DDS_StdString & str2);
   friend bool operator != (const DDS_StdString & str1, const char * str2);
   friend bool operator != (const DDS_StdString & str1, const DDS_StdString & str2);
   friend bool operator == (const char * str1, const DDS_StdString & str2);
   friend bool operator != (const char * str1, const DDS_StdString & str2);

   DDS_StdString & operator= (const char * str);
   DDS_StdString & operator= (const DDS_StdString & str);
   DDS_StdString & operator+= (const DDS_StdString & str);

   char & operator[](unsigned) const;

   operator char * ();
   operator const char * () const;

   unsigned long length () const;

public:

   struct stringrep
   {
      char * m_string;
      Counter m_refs;
   };

 private:

   DDS_StdString (stringrep * rep);

   stringrep * m_rep;
};


inline bool operator == (const DDS_StdString & str1, const char * str2)
{
   if (str1.m_rep->m_string && str2)
   {
      return strcmp (str1.m_rep->m_string, str2) == 0;
   }
   else
   {
      return (str1.m_rep->m_string == str2);
   }
}

inline bool operator == (const DDS_StdString & str1, const DDS_StdString & str2)
{
   if (str1.m_rep->m_string && str2.m_rep->m_string)
   {
      return strcmp (str1.m_rep->m_string, str2.m_rep->m_string) == 0;
   }
   else
   {
      return (str1.m_rep->m_string == str2.m_rep->m_string);
   }
}

inline bool operator != (const DDS_StdString & str1, const char * str2)
{
   if (str1.m_rep->m_string && str2)
   {
      return strcmp(str1.m_rep->m_string, str2) != 0;
   }
   else
   {
      return (str1.m_rep->m_string != str2);
   }
}

inline bool operator != (const DDS_StdString & str1, const DDS_StdString & str2)
{
   if (str1.m_rep->m_string && str2.m_rep->m_string)
   {
      return (strcmp (str1.m_rep->m_string, str2.m_rep->m_string) != 0);
   }
   else
   {
      return (str1.m_rep->m_string != str2.m_rep->m_string);
   }
}

inline bool operator == (const char * str1, const DDS_StdString & str2)
{
   if (str1 && str2.m_rep->m_string)
   {
      return strcmp (str1, str2.m_rep->m_string) == 0;
   }
   else
   {
      return (str1 == str2.m_rep->m_string);
   }
}

inline bool operator != (const char * str1, const DDS_StdString & str2)
{
   if (str1 && str2.m_rep->m_string)
   {
      return strcmp (str1, str2.m_rep->m_string) != 0;
   }
   else
   {
      return (str1 != str2.m_rep->m_string);
   }
}

inline DDS_StdString::operator char * ()
{
   return m_rep->m_string;
}

inline DDS_StdString::operator const char * () const
{
   return m_rep->m_string;
}

inline unsigned long DDS_StdString::length () const
{
   return (m_rep->m_string ? strlen(m_rep->m_string) : 0);
}

inline ostream & operator << (ostream & os, DDS_StdString & stdString)
{
   char *str = (char *) stdString;
   return (str != 0) ? (os << str) : os;
}

inline ostream & operator << (ostream & os, const DDS_StdString & stdString)
{
   const char *str = (const char *) stdString;
   return (str != 0) ? (os << str) : os;
}

#endif
