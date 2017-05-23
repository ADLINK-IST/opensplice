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
#include "if.h"
#include "cpp_io.h"

#ifdef DEBUG_WHILE
extern int debugging;
#endif

extern void do_while (void)
{
   input_mark();
   do_if(0);
}

extern void do_endwhile(void)
{
   if (in_false_if())
   {
      do_endif(0);
      input_unmark();
#ifdef DEBUG_WHILE

      if (debugging)
      {
         outputs("//endwhile:");
         outputd(curline());
         outputs(",");
         outputs(curfile());
         outputs("\\\\");
      }
#endif
      out_at(curline(), curfile());
   }
   else
   {
      do_endif(0);
      input_recover();
      input_mark();
      do_if(0);
   }
}
