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
