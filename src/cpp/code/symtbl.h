/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef _SYMTBL_
#define _SYMTBL_

#define DEF_PREDEF  0
#define DEF_CMDLINE 1
#define DEF_DEFINE  2

#ifdef __cplusplus
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CPP
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

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
OS_API extern void define (const char *, int, unsigned char *, int);
extern void undef (char *);
extern void defd (char *, int);
extern void undef_predefs (void);
extern void expand_def (DEF *);

#undef OS_API

#ifdef __cplusplus
}
#endif

#endif
