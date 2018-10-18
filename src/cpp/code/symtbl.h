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
#include "dds_cpp.h"

#ifndef _SYMTBL_
#define _SYMTBL_

#define DEF_PREDEF  0
#define DEF_CMDLINE 1
#define DEF_DEFINE  2

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _def
{
  struct _def *link;
  char *name;
  int nargs;
  unsigned char *repl;
  int how;
} DEF;

extern DEF ** symtbl;
extern int symtbl_size;
extern int n_in_table;

extern DEF * find_def (char *);
extern void init_symtbl (void);
extern void define (const char *, int, unsigned char *, int);
extern void undef (const char *);
extern void defd (char *, int);
extern void undef_predefs (void);
extern void expand_def (DEF *);

#ifdef __cplusplus
}
#endif

#endif
