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
#include "xbe_cppname.h"
#include "xbe_utils.h"

bool be_CppName::IsKeyword (const DDSString & idlName)
{
   for (const char** keyword = m_CppKeywords; *keyword; keyword++)
   {
      if (str_eq(idlName, *keyword))
      {
         return true;
      }
   }

   return false;
}

DDSString be_CppName::ConvertToCpp (const DDSString & idlName)
{
   DDSString result = idlName;

   // This next bit of code initializes the string for
   // iostreams that require it.

   if (result.length() == 0)
   {
      result = "";
   }

   if (IsKeyword(idlName))
   {
      result = "_cxx_" + idlName;
   }

   return result;
}

const char* be_CppName::m_CppKeywords[] =
{
   "and",
   "and_eq",
   "asm",
   "auto",
   "bitand",
   "bitor",
   "bool",
   "break",
   "case",
   "catch",
   "char",
   "class",
   "compl",
   "const",
   "const_cast",
   "continue",
   "default",
   "delete",
   "do",
   "double",
   "dynamic_cast",
   "else",
   "enum",
   "extern",
   "false",
   "float",
   "for",
   "friend",
   "goto",
   "if",
   "inline",
   "int",
   "long",
   "mutable",
   "namespace",
   "new",
   "not",
   "not_eq",
   "operator",
   "or",
   "or_eq",
   "private",
   "protected",
   "public",
   "register",
   "reinterpret_cast",
   "return",
   "short",
   "signed",
   "sizeof",
   "static",
   "static_cast",
   "struct",
   "switch",
   "template",
   "this",
   "throw",
   "true",
   "try",
   "typedef",
   "typeid",
   "union",
   "unsigned",
   "using",
   "virtual",
   "void",
   "volatile",
   "wchar_t",
   "while",
   "xor",
   "xor_eq",
   NULL
};
