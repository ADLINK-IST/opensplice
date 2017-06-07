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
#ifndef _IF_
#define _IF_

#ifdef __cplusplus
extern "C"
{
#endif

   typedef struct _if
   {
      struct _if *next;
      int condstate;
   }
   IF;

#define IFSTATE_TRUE 0
#define IFSTATE_FALSE 1
#define IFSTATE_STAYFALSE 2

   IF *ifstack;
   int n_skipped_ifs;

   extern void do_sharp (void);
   extern void do_if (int);
   extern void do_ifdef (int);
   extern void do_ifndef (int);
   extern void do_elif (int);
   extern void do_else (int);
   extern void do_endif (int);
   extern void do_set (void);
   extern void do_dump (void);
   extern void do_line (void);
   extern void do_while (void);
   extern void do_endwhile (void);
   extern void do_pragma (void);
   extern void do_include (int);
   extern void do_define (int, int);
   extern void do_undef (int);
   extern void do_at (void);
   extern void do_eval (void);
   extern void do_debug (void);

   extern int in_false_if (void);
   extern void maybe_print (char);
   extern void autodef_file (const char *);
   extern void autodef_line (int);
   extern void flush_sharp_line (void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
}
#endif

#endif
