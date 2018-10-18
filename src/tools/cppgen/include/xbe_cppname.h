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
#ifndef _XBE_CPPNAME_H
#define _XBE_CPPNAME_H

#include "xps_string.h"

// A be_CppName is a name (an identifier) in C++.
//
// A be_CppName is always guaranteed to not be a C++ keyword.  If you try
// to create a be_CppName that is a C++ keyword, it automatically prepends
// "_cxx_".  This is how the DDS spec requires that IDL identifiers that
// are C++ keywords be represented in C++.

class be_CppName
{

public:
   be_CppName() : m_name()
   { }

   be_CppName(const char* s) : m_name(ConvertToCpp(s))
   { }

   be_CppName(const DDSString& str) : m_name(ConvertToCpp(str))
   { }

   be_CppName(const be_CppName& that) : m_name(that.m_name)
   { }

   be_CppName(const DDS_StdString& stdstr)
         : m_name(ConvertToCpp((DDSString)stdstr))
   { }

   // CONVERSIONS

   inline operator const DDSString&() const
   {
      return m_name;
   }

   // ASSIGNMENT OPERATORS

   inline be_CppName& operator=(const be_CppName& that)
   {
      m_name = that.m_name;
      return *this;
   }

   inline be_CppName& operator=(const char* s)
   {
      m_name = ConvertToCpp(s);
      return *this;
   }

   inline be_CppName& operator=(const DDSString& str)
   {
      m_name = ConvertToCpp(str);
      return *this;
   }

   // COMPARISON OPERATORS

   inline bool operator<(const be_CppName& that) const
   {
      return strcmp(m_name, DDSString(that)) < 0;
   }

private:
   DDSString m_name;

   // PRIVATE HELPERS

   static const char* m_CppKeywords[];

   static bool IsKeyword(const DDSString& idlName);

   static DDSString ConvertToCpp(const DDSString& idlName);
};

// COMPARISON OPERATORS

inline bool operator==(const be_CppName& c1, const be_CppName& c2)
{
   return (DDSString)c1 == (DDSString)c2;
}

inline bool operator!=(const be_CppName& c1, const be_CppName& c2)
{
   return (DDSString)c1 != (DDSString)c2;
}

inline bool operator==(const be_CppName& c, const DDSString& str)
{
   return (DDSString)c == str;
}

inline bool operator!=(const be_CppName& c, const DDSString& str)
{
   return (DDSString)c != str;
}

inline bool operator==(const DDSString& str, const be_CppName& c)
{
   return (DDSString)c == str;
}

inline bool operator!=(const DDSString& str, const be_CppName& c)
{
   return (DDSString)c != str;
}

inline bool operator==(const be_CppName& c, const char* s)
{
   return (DDSString)c == s;
}

inline bool operator!=(const be_CppName& c, const char* s)
{
   return (DDSString)c != s;
}

inline bool operator==(const char* s, const be_CppName& c)
{
   return (DDSString)c == s;
}

inline bool operator!=(const char* s, const be_CppName& c)
{
   return (DDSString)c != s;
}

// STREAMING

inline ostream& operator<<(ostream& os, const be_CppName& cppName)
{
   os << DDSString(cppName);
   return os;
}

#endif // _XBE_CPPNAME_H
