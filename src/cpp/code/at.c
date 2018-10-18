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
#include "cpp_io.h"
#include "if.h"
#include "cpp_malloc.h"

extern void do_at (void)
{
   char *w;

   w = read_ctrl();
   if (strcmp(w, "include") == 0)
   {
      do_include(0);
   }
   else if (strcmp(w, "define") == 0)
   {
      do_define(0, 0);
   }
   else if (strcmp(w, "undef") == 0)
   {
      do_undef(0);
   }
   else if (strcmp(w, "redefine") == 0)
   {
      do_define(0, 1);
   }
   else if (strcmp(w, "ifdef") == 0)
   {
      do_ifdef(0);
   }
   else if (strcmp(w, "ifndef") == 0)
   {
      do_ifndef(0);
   }
   else if (strcmp(w, "if") == 0)
   {
      do_if(0);
   }
   else if (strcmp(w, "else") == 0)
   {
      do_else(0);
   }
   else if (strcmp(w, "elif") == 0)
   {
      do_elif(0);
   }
   else if (strcmp(w, "endif") == 0)
   {
      do_endif(0);
   }
   else if (strcmp(w, "set") == 0)
   {
      do_set();
   }
   else if (strcmp(w, "while") == 0)
   {
      do_while();
   }
   else if (strcmp(w, "endwhile") == 0)
   {
      do_endwhile();
   }
   else if (strcmp(w, "dump") == 0)
   {
      do_dump();
   }
   else if (strcmp(w, "debug") == 0)
   {
      do_debug();
   }
   else if (strcmp(w, "eval") == 0)
   {
      do_eval();
   }
   else
   {
      if (! in_false_if())
      {
         err_head();
         fprintf(stderr, "unknown control `%s'\n", w);
      }
   }
   os_free(w);
}
