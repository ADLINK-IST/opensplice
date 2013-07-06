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
