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
#include "cpp_io.h"
#include "if.h"
#include "cpp_malloc.h"

extern void do_sharp (void)
{
   char *w;
   char c;

   w = read_ctrl();
   if (strcmp(w, "ifdef") == 0)
   {
      do_ifdef(1);
   }
   else if (strcmp(w, "ifndef") == 0)
   {
      do_ifndef(1);
   }
   else if (strcmp(w, "if") == 0)
   {
      do_if(1);
   }
   else if (strcmp(w, "else") == 0)
   {
      do_else(1);
   }
   else if (strcmp(w, "elif") == 0)
   {
      do_elif(1);
   }
   else if (strcmp(w, "endif") == 0)
   {
      do_endif(1);
   }
   else if (strcmp(w, "include") == 0)
   {
      do_include(1);
   }
   else if (strcmp(w, "define") == 0)
   {
      do_define(1, 0);
   }
   else if (strcmp(w, "undef") == 0)
   {
      do_undef(1);
   }
   else if (strcmp(w, "line") == 0)
   {
      do_line();
   }
   else if (strcmp(w, "pragma") == 0)
   {
      do_pragma();
   }
   else if (strcmp(w, "") == 0)
   {}
   else
   {
      int isnull;
      isnull = 0;
      if (strcmp(w, "") == 0)
      {
         c = Get();
         if (c != '\n')
         {
            Push(c);
         }
         else
         {
            isnull = 1;
         }
      }
      if (!isnull && !in_false_if())
      {
         err_head();
         fprintf(stderr, "unknown control `%s'\n", w);
         flush_sharp_line();
      }
   }
   maybe_print('\n');
   os_free(w);
}

extern void flush_sharp_line (void)
{
   int quote;
   int backslash;
   int comment;
   int lastc;
   int c;

   quote = 0;
   backslash = 0;
   comment = 0;
   c = 'x';
   while (1)
   {
      lastc = c;
      c = Get();
      if (backslash)
      {
         backslash = 0;
         continue;
      }
      if (comment)
      {
         if (c == '\\')
         {
            backslash = 1;
         }
         else if ((c == '/') && (lastc == '*'))
         {
            comment = 0;
         }
         continue;
      }
      if (quote)
      {
         if (c == '\\')
         {
            backslash = 1;
         }
         else if (c == quote)
         {
            quote = 0;
         }
      }
      if (EOF)
      {
         break;
      }
      switch (c)
      {
         case '\\':
            backslash = 1;
            continue;
            break;
         case '"':
         case '\'':
            quote = c;
            continue;
            break;
         case '*':
            comment = (lastc == '/');
            continue;
            break;
         default:
            continue;
            break;
         case '\n':
            break;
      }
      break;
   }
}
