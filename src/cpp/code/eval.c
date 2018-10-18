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
#include "cpp_malloc.h"
#include "cpp_io.h"
#include "if.h"
#include "expr.h"

extern void do_eval (void)
{
   char c;
   char temp[64];
   int i;

   if (! in_false_if())
   {
      c = getnonspace();
      if (c != '(')
      {
         err_head();
         fprintf(stderr, "@eval must have ()s\n");
         Push(c);
         return ;
      }
      os_sprintf(temp, "%d", eval_expr(0, 1));
      for (i = strlen(temp) - 1;i >= 0;i--)
      {
         Push(temp[i]);
      }
   }
}
