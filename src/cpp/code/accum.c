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
#include "accum.h"
#include "cpp_malloc.h"
#include "vortex_os.h"

char * init_accum (void)
{
   ACCUM *a;

   a = NEW(ACCUM);
   check_os_malloc(a);
   a->used = 0;
   a->have = 8;
   a->buf = os_malloc(8);
   check_os_malloc(a->buf);
   return ((char *)a);
}


void accum_char (char * A, char c)
{
#define a ((ACCUM *)A)
   while (a->used >= a->have)
   {
      a->buf = os_realloc(a->buf, a->have += 8);
      check_os_malloc(a->buf);
   }
   a->buf[a->used++] = c;
}
#undef a

char accum_regret( char * A)
{
#define a ((ACCUM *)A)
   if (a->used > 0)
   {
      return (a->buf[--a->used]);
   }
   else
   {
      return (0);
   }
}
#undef a

char *accum_result (char * A)
{
#define a ((ACCUM *)A)
   char *cp;

   cp = os_realloc(a->buf, a->used + 1);
   check_os_malloc(cp);
   cp[a->used] = '\0';
   OLD(a);
   return (cp);
}
#undef a

char *accum_sofar (char * A)
{
#define a ((ACCUM *)A)
   char *cp;

   cp = os_malloc(a->used + 1);
   check_os_malloc(cp);
   memcpy(cp, a->buf, a->used);
   cp[a->used] = '\0';
   return (cp);
}
#undef a

char *accum_buf (char * A)
{
#define a ((ACCUM *)A)
   return (a->buf);
}
#undef a

int accum_howfar(char * A)
#define a ((ACCUM *)A)
{
   return (a->used);
}
#undef a
