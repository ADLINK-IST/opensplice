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
#include "xps_string.h"

DDSString::DDSString(const DDSString& that)
   : m_rep (that.m_rep)
{
   that.m_rep->m_refs++;
}

DDSString::~DDSString()
{
   AbandonRep();
}

DDSString::stringrep*
DDSString::IntToString(int i)
{
   char buf[20];

   os_sprintf(buf, "%d", i);
   return new stringrep(buf);
}

DDSString::stringrep *
DDSString::ULongToString (DDS::ULong i)
{
   char buf[20];

   os_sprintf (buf, "%du", i);
   return new stringrep(buf);
}

DDSString&
DDSString::operator=(const char* s)
{
   RemoveReference();

   if ((int)m_rep->m_refs == 0)
   {
      m_rep->SetNewString(s);
   }
   else
   {
      m_rep = new stringrep(s);
   }

   return *this;
}

DDSString&
DDSString::operator=(const DDSString& str)
{
   if (m_rep != str.m_rep)
   {
      AbandonRep();
      CopyRep(str.m_rep);
   }

   return *this;
}

DDSString&
DDSString::operator+=(const DDSString& str)
{
   if (length() == 0)
   {
      str.m_rep->m_refs++;

      if (m_rep->m_refs-- == 0)
      {
         delete[] m_rep->m_string;
         delete m_rep;
      }

      m_rep = str.m_rep;
      return *this;
   }
   else if (str.length() == 0)
   {
      return *this;
   }
   else
   {
      stringrep* newrep = new stringrep(m_rep->m_string, str.m_rep->m_string);

      if (m_rep->m_refs-- == 0)
      {
         delete[] m_rep->m_string;
         delete m_rep;
      }

      m_rep = newrep;

      return *this;
   }
}

char&
DDSString::operator[](int index) const
{
   int len;

   if ((len = length()) > index)
   {
      return m_rep->m_string[index];
   }
   else
   {
      return m_rep->m_string[len];
   }
}

DDSString
operator+(const DDSString& str1, const DDSString& str2)
{
   return DDSString(str1.m_rep->m_string, str2.m_rep->m_string);
}
