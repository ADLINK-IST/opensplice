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
#include "symtbl.h"

extern void err_head (void);

extern void do_undef (int sharp)
{
   char *mac;

   if (! in_false_if())
   {
      mac = read_ident();
      if (! mac)
      {
         err_head();
         fprintf(stderr, "missing/illegal macro name\n");
      }
      else
      {
         undef (mac);
      }
   }
   if (sharp)
   {
      flush_sharp_line();
   }
}
