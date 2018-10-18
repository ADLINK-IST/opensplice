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
#include "vortex_os.h"

extern char * fe_cpp_keyword (const char * id);

static const char * fe_cpp_keywords [] =
{
   "asm",
   "auto",
   "bool",
   "break",
   "case",
   "catch",
   "char",
   "class",
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
   "explicit",
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
   "operator",
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
   "typename",
   "union",
   "unsigned",
   "using",
   "virtual",
   "void",
   "volatile",
   "wchar_t",
   "while",
   0
};

char * fe_cpp_keyword (const char * id)
{
   static const char * prefix = "__cxx_";
   static const char escape = '_';

   unsigned long i = 0;
   char * ret = 0;
   const char * test = id;

   if (id)
   {
      /* Remove identifier escaping */

      if (test[0] == escape)
      {
         test++;
      }

      /* Prefix C++ keywords with _cxx_ */

      while (fe_cpp_keywords[i])
      {
         if (strcmp (test, fe_cpp_keywords[i]) == 0)
         {
            ret = (char*) os_malloc (strlen (test) + strlen (prefix) + 1);
            strcpy (ret, prefix);
            strcat (ret, test);
            break;
         }
         i++;
      }

      if (ret == 0)
      {
         ret = (char*) os_malloc (strlen (id) + 1);
         strcpy (ret, id);
      }
   }

   return ret;
}
