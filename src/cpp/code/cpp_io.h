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
#ifndef _IO_
#define _IO_

#include "stdincs.h"
#include "os_stdlib.h"

#define MAX_PUSHBACK 8192
#define MAXFILES 100

#define CPP_FILESEPCHAR_DEF OS_FILESEPCHAR
#define CPP_FILESEPCHAR_1   '/'
#define CPP_FILESEPCHAR_2   '\\'

extern int linefirst;
extern int willbefirst;
/* extern FILE *outfile; */

#ifdef __cplusplus
extern "C"
{
#endif

   extern int Get (void);
   extern char getnonspace (void);
   extern char getnonhspace (void);
   extern char getnhsexpand (void);
   extern char getexpand (void);
   extern char *curfile (void);
   extern char **Curfile (void);
   extern int curline (void);
   extern int *Curline (void);
   extern char *curdir (void);
   extern char **Curdir (void);
   extern char *read_ctrl (void);
   extern char *read_ident (void);
   extern void init_io (FILE *, const char *);
   extern void init_incldir (const char *);
   extern void push_new_file (char *, FILE *);
   extern void Push (char);
   extern void output_line_and_file (void);
   extern void out_at (int, const char *);
   extern void outputc (char);
   extern void outputs (char *);
   extern void outputd (int);
   extern void input_mark (void);
   extern void input_unmark (void);
   extern void input_recover (void);
   extern void mark_file_beginning (void);
   extern void mark_file_ending (void);
   extern void mark_charpushed (void);
   extern void mark_got_from_pushback (char);
   extern void mark_got_from_file (char);
   extern void mark_got (char);
   extern void mark_get_line (void);
   extern int read_char (void);

#ifdef __cplusplus
}
#endif

#endif
