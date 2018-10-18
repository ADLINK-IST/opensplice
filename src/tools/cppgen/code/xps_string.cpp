/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
   char buf[21];

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
