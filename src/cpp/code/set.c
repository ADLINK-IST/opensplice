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
#include "symtbl.h"
#include "cpp_malloc.h"
#include "cpp_io.h"
#include "if.h"
#include "expr.h"

extern void do_set (void)
{
   char *mac;
   char c;
   char temp[64];

   mac = read_ident();
   if (! mac)
   {
      err_head();
      fprintf(stderr, "@set: missing/illegal macro name\n");
      return ;
   }
   if (! in_false_if())
   {
      char *cp;
      c = getnonspace();
      if (c != '(')
      {
         err_head();
         fprintf(stderr, "@set must have ()s\n");
         Push(c);
         return ;
      }
      os_sprintf(temp, "%d", eval_expr(0, 1));
      undef(mac);
      cp = copyofstr(temp);
      check_os_malloc(cp);
      define(mac, -1, (unsigned char *) cp, DEF_DEFINE);
   }
   os_free(mac);
}
