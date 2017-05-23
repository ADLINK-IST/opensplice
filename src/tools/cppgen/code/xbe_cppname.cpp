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
