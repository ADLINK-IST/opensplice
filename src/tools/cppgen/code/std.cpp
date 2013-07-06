/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "Std.h"
#include "sacpp_DDS_DCPS.h"
#include "StdList.h"
#include "StdString.h"

char * string_a (DDS::ULong len)
{
   return new char [len + 1];
}

void string_f (char * str)
{
   delete [] str;
}
 
// -----------------------------------------
// DDS::StdList implementation
// -----------------------------------------

// -----------------------------------------
// hash_str is used by the generated
//  dispatcher
// -----------------------------------------

DDS::ULong Std::hash_str (const char* str)
{
   const long p = 1073741827L;  // prime
   int n = strlen(str);
   long h = 0;
   long retval = 0;

   for (int i = 0; i < n; ++i, ++str)
   {
      h = (h << 2) + *str;
   }

   retval = ((h >= 0) ? (h % p) : (-h % p));

   return retval;
}


// -----------------------------------------
// DDS_StdString implementation
// -----------------------------------------
DDS_StdString::DDS_StdString ()
   : m_rep (new stringrep ())
{
   m_rep->m_string = 0;
   m_rep->m_refs = 1;
}

DDS_StdString::DDS_StdString (const char * str)
   : m_rep (new stringrep ())
{
   if (str)
   {
      m_rep->m_string = string_a (strlen (str));
      os_strcpy (m_rep->m_string, str);
   }
   else
   {
      m_rep->m_string = 0;
   }

   m_rep->m_refs = 1;
}


DDS_StdString::DDS_StdString (const DDS_StdString& that)
      :
      m_rep (that.m_rep)
{
   that.m_rep->m_refs++;
}


DDS_StdString::DDS_StdString (stringrep * rep)
      :
      m_rep (rep)
{
   m_rep->m_refs++;
}


DDS_StdString& DDS_StdString::operator=(const char * str)
{
   if (m_rep->m_refs-- == 0)
   {
      string_f (m_rep->m_string);
   }
   else
   {
      m_rep = new stringrep;
   }

   if (str)
   {
      m_rep->m_string = string_a (strlen (str));
      os_strcpy (m_rep->m_string, str);
   }
   else
   {
      m_rep->m_string = 0;
   }

   m_rep->m_refs = 1;

   return *this;
}


DDS_StdString& DDS_StdString::operator=(const DDS_StdString & str)
{
   if (m_rep != str.m_rep)
   {
      if (m_rep->m_refs-- == 0)
      {
         string_f (m_rep->m_string);
         delete m_rep;
      }

      m_rep = str.m_rep;
      m_rep->m_refs++;
   }

   return *this;
}


DDS_StdString& DDS_StdString::operator+=(const DDS_StdString & str)
{
   if (length() == 0)
   {
      str.m_rep->m_refs++;

      if (m_rep->m_refs-- == 0)
      {
         string_f (m_rep->m_string);
         delete m_rep;
      }

      m_rep = str.m_rep;
      return *this;
   }
   else if (str.length () == 0)
   {
      return *this;
   }
   else
   {
      stringrep * newrep;
      int newlen;

      newlen = length() + str.length();
      newrep = new stringrep;
      newrep->m_string = string_a (newlen);
      os_strcpy(newrep->m_string, m_rep->m_string);
      os_strcat(newrep->m_string, str.m_rep->m_string);
      newrep->m_refs = 1;

      if (m_rep->m_refs-- == 0)
      {
         string_f (m_rep->m_string);
         delete m_rep;
      }

      m_rep = newrep;

      return *this;
   }
}


char& DDS_StdString::operator[](unsigned index) const
{
   unsigned long len;

   if ((len = length ()) > index)
   {
      return m_rep->m_string[index];
   }
   else
   {
      return m_rep->m_string[len];
   }
}


DDS_StdString operator+(const DDS_StdString & str1, const char * str2)
{
   if (str1.m_rep->m_string && str2)
   {
      DDS_StdString::stringrep * rep = new DDS_StdString::stringrep;
      unsigned len;

      len = str1.length () + strlen (str2);

      if (len)
      {
         rep->m_string = string_a (len);
         os_strcpy (rep->m_string, str1.m_rep->m_string);
         os_strcat(rep->m_string, str2);
      }
      else
      {
         rep->m_string = 0;
      }

      return DDS_StdString (rep);
   }
   else if (str1.m_rep->m_string)
   {
      return DDS_StdString (str1);
   }
   else if (str2)
   {
      return DDS_StdString (str2);
   }
   else
   {
      return DDS_StdString ();
   }
}


DDS_StdString operator+(const DDS_StdString& str1, const DDS_StdString & str2)
{

   if (str1.m_rep->m_string && str2.m_rep->m_string)
   {
      DDS_StdString::stringrep * rep = new DDS_StdString::stringrep;
      unsigned len;

      len = str1.length () + str2.length ();

      if (len)
      {
         rep->m_string = string_a (len);
         os_strcpy (rep->m_string, str1.m_rep->m_string);
         os_strcat (rep->m_string, str2.m_rep->m_string);
      }
      else
      {
         rep->m_string = 0;
      }

      return DDS_StdString (rep);
   }
   else if (str1.m_rep->m_string)
   {
      return DDS_StdString (str1);
   }
   else if (str2.m_rep->m_string)
   {
      return DDS_StdString (str2);
   }
   else
   {
      return DDS_StdString ();
   }
}


DDS_StdString::~DDS_StdString()
{
   if (--m_rep->m_refs == 0)
   {
      string_f (m_rep->m_string);
      delete m_rep;
   }
}


// -----------------------------------------
// StdList::node implementation
// -----------------------------------------
const size_t StdList::node::m_chunkSize = 500;
StdList::node * StdList::node::m_freeList = 0;
