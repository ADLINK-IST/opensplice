#ifndef _IO_
#define _IO_

#include "stdincs.h"

#define MAX_PUSHBACK 8192
#define MAXFILES 20

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
   extern void init_io (FILE *, char *);
   extern void init_incldir (char *);
   extern void push_new_file (char *, FILE *);
   extern void Push (char);
   extern void out_at (int, char *);
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

#ifdef __cplusplus
}
#endif

#endif
