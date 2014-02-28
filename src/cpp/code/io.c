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
#include <stdio.h>
#include <ctype.h>
#include "expr.h"
#include "cpp_malloc.h"
#include "symtbl.h"
#include "is.h"
#include "accum.h"
#include "io.h"
#include "unctrl.h"
#include "if.h"

#if defined(DEBUG_IO) || defined(DEBUG_INCL)
extern int debugging;
#endif

extern int no_line_lines;

int linefirst;
int willbefirst;
int lineno[MAXFILES];
FILE *fstack[MAXFILES];
int fstackdepth;
char *fn[MAXFILES];
int npushed[MAXFILES];
char pushback[MAXFILES][MAX_PUSHBACK];
char *incldir[MAXFILES];
FILE *outfile;

char *cur_pushback;
int cur_npushed;
FILE *cur_fstack;
char *cur_fn;
char *cur_incldir;
int cur_lineno;

typedef struct _mark
{
   struct _mark *link;
   char *acc;
   int startdepth;
   int incs[MAXFILES];
   int nincs;
   int nignore;
}
MARK;

static MARK *marks;
static int atline;
static char *atfile;
static int done_line;
static int nnls;

static char* output_accum;

/* XXX these definitions assume no input chars are 81, 82, 83 */
#define NOEXPAND ((char)0x81)
#define MARKLINE ((char)0x82)
#define MARKLEND ((char)0x83)

static void mark_push_here (MARK *, int, char *);

static void flush_final_nl (void)
{
   if (nnls > 0)
   {
      accum_char(output_accum, '\n');
   }
}

extern void init_io (FILE * f, const char * fnname)
{
   register int i;

   fstackdepth = 0;
   cur_pushback = pushback[0];
   cur_npushed = 0;
   cur_fstack = f;
   cur_fn = copyofstr(fnname);
   check_os_malloc(cur_fn);
   cur_lineno = 1;
   nnls = 0;
   atfile = copyofstr("");
   check_os_malloc(atfile);
   for (i = 0;i < MAXFILES;i++)
   {
      npushed[i] = 0;
   }
   marks = 0;

   output_accum = init_accum();
}

extern void init_incldir (const char * d)
{
   cur_incldir = copyofstr(d);
   check_os_malloc(cur_incldir);
}

static void change_fstackdepth (int delta)
{
   npushed[fstackdepth] = cur_npushed;
   fstack[fstackdepth] = cur_fstack;
   fn[fstackdepth] = cur_fn;
   incldir[fstackdepth] = cur_incldir;
   lineno[fstackdepth] = cur_lineno;
   fstackdepth += delta;
   if (fstackdepth < MAXFILES) {
      cur_pushback = pushback[fstackdepth];
      cur_npushed = npushed[fstackdepth];
      cur_fstack = fstack[fstackdepth];
      cur_fn = fn[fstackdepth];
      cur_incldir = incldir[fstackdepth];
      cur_lineno = lineno[fstackdepth];
    } else {
       fprintf(stderr, "fatal error: include recursion too deep, only %d allowed\n",
          MAXFILES);
       exit(0);
    }
}

#define GET() (cur_pushback[--cur_npushed])
#define PUSH() (cur_pushback[cur_npushed++])

int Get()
{
   char c;

   linefirst = willbefirst;
   willbefirst = 0;
   while (1)
   {
      if (cur_npushed > 0)
      {
         do
         {
            c = GET();
         }
         while (c == NOEXPAND);
         if (c == MARKLINE)
         {
            mark_get_line();
            continue;
         }
         mark_got_from_pushback(c);
#ifdef DEBUG_IO

         if (debugging)
         {
            outputc('(');
            outputs(unctrl(c));
            outputc(')');
         }
#endif
         break;
      }
      else
      {
         c = getc(cur_fstack);
         if (feof(cur_fstack))
         {
            if (fstackdepth > 0)
            {
#ifdef DEBUG_INCL
               if (debugging)
               {
                  outputs("--eof:");
                  outputs(cur_fn);
                  outputs("--");
               }
#endif
               fclose(cur_fstack);
               os_free(cur_fn);
               os_free(cur_incldir);
               change_fstackdepth( -1);
               mark_file_ending();
               out_at(cur_lineno, cur_fn);
               autodef_file(curfile());
               autodef_line(curline());
               Push('\n');
               continue;
            }
            else
            {
               flush_final_nl();
               return ( -1);
            }
         }
         mark_got_from_file(c);
#ifdef DEBUG_IO

         if (debugging)
         {
            outputc('<');
            outputs(unctrl(c));
            outputc('>');
         }
#endif
         break;
      }
   }
   if (c == '\n')
   {
      cur_lineno ++;
      autodef_line(curline());
      willbefirst = 1;
   }
   return (c);
}

extern void push_new_file (char * name, FILE * f)
{
   change_fstackdepth(1);
   cur_fn = copyofstr(name);
   check_os_malloc(cur_fn);
   cur_lineno = 1;
   cur_fstack = f;
   cur_npushed = 0;
#ifdef DEBUG_INCL

   if (debugging)
   {
      outputs("--newfile:");
      outputs(name);
      outputs("--");
   }
#endif
   mark_file_beginning();
}

extern void Push (char c)
{
   if (cur_npushed > MAX_PUSHBACK)
   {
      fprintf(stderr, "too much pushback\n");
      cur_npushed = 0;
   }
   PUSH() = c;
   mark_charpushed();
   willbefirst = 0;
   if (c == '\n')
   {
      cur_lineno --;
      autodef_line(curline());
   }
#ifdef DEBUG_IO
   if (debugging)
   {
      outputc('[');
      outputs(unctrl(c));
      outputc(']');
   }
#endif
}

char *curfile()
{
   return (cur_fn);
}

char **Curfile()
{
   return (&cur_fn);
}

int curline()
{
   return (cur_lineno);
}

int *Curline()
{
   return (&cur_lineno);
}

char *curdir()
{
   return (cur_incldir);
}

char **Curdir()
{
   return (&cur_incldir);
}

char getnonspace ()
{
   char c;

   do
   {
      c = Get ();
   }
   while (isspace ((int) c));

   return (c);
}

char getnonhspace()
{
   char c;

   while (ishspace(c = Get()))
      ;
   return (c);
}

char getnhsexpand()
{
   char c;

   while (1)
   {
      c = getexpand();
      if (ishspace(c))
      {
         continue;
      }
      if (expr_sharp && (c == '\\'))
      {
         c = Get();
         if (c == '\n')
         {
            continue;
         }
         else
         {
            Push(c);
            c = '\\';
         }
      }
      break;
   }
   return (c);
}

char getexpand()
{
   char c;
   char *str;
   DEF *d;

   while (1)
   {
      c = Get();
      if (c == '/')
      {
         char d;
         d = Get();
         if (d == '*')
         {
            d = '\0';
            do
            {
               c = d;
               d = Get();
            }
            while ((c != '*') || (d != '/'));
            continue;
         }
         else
         {
            Push(d);
            return ('/');
         }
      }
      else if (c == NOEXPAND)
      {
         c = Get();
         if (issymchar(c))
         {
            Push(NOEXPAND);
         }
         return (c);
      }
      else if (! isbsymchar(c))
      {
         return (c);
      }
      str = init_accum();
      do
      {
         accum_char(str, c);
      }
      while (issymchar(c = Get()));
      Push(c);
      str = accum_result(str);
      d = find_def(str);
      if (d)
      {
         expand_def(d);
         os_free(str);
      }
      else
      {
         int i;
         for (i = strlen(str) - 1;i > 0;i--)
         {
            Push(str[i]);
         }
         Push(NOEXPAND);
         c = str[0];
         os_free(str);
         return (c);
      }
   }
}

char *read_ctrl()
{
   char c;
   char *acc;

   acc = init_accum();
   while (ishspace(c = Get()))
      ;
   Push(c);
   while (1)
   {
      c = Get();
      if (islower ((int) c))
      {
         accum_char(acc, c);
      }
      else
      {
         Push(c);
         return (accum_result(acc));
      }
   }
}

char *read_ident()
{
   char *acc;
   char c;

   c = getnonspace();
   if (! isbsymchar(c))
   {
      Push(c);
      return (0);
   }
   acc = init_accum();
   do
   {
      accum_char(acc, c);
   }
   while (issymchar(c = Get()));
   Push(c);
   return (accum_result(acc));
}

/* OSPL-2307: Bug emerged after fixing inclusion of recursively resolved
   header files. cpp would forget about previous include files if no actual idl
   was located between recursive includes. */
extern void output_line_and_file (void)
{
    atline += nnls;
    nnls = 0;

    if (no_line_lines)
    {
       accum_char(output_accum, '\n');
    }
    else
    {
       char temp[1024];
       char *s;

       os_sprintf(temp, "\n# %d \"%s\"\n", atline, atfile);
       for (s = temp; *s; s++)
       {
          accum_char(output_accum, *s);
       }
    }
    done_line = 1;
}

extern void out_at (int line, const char * file)
{
   if ((line - nnls != atline) || strcmp(file, atfile))
   {
      if (!done_line)
      {
         output_line_and_file();
      }
      os_free(atfile);
      atline = line - nnls;
      atfile = copyofstr(file);
      check_os_malloc(atfile);
      done_line = 0;
   }
}

extern void outputc (char c)
{
   extern int incldep;
   static char *white = 0;

   if (incldep)
   {
      return ;
   }
   if (c == '\n')
   {
      nnls ++;
      if (white)
      {
         os_free(accum_result(white));
         white = 0;
      }
   }
   else if ((nnls > 0) && ((c == ' ') || (c == '\t')))
   {
      if (! white)
      {
         white = init_accum();
      }
      accum_char(white, c);
   }
   else
   {
      if ((nnls > 2) || !done_line)
      {
         output_line_and_file ();
      }
      for (;nnls;nnls--)
      {
         atline ++;
         accum_char(output_accum, '\n');
      }
      if (white)
      {
         char *ptr;

         ptr = white = accum_result(white);

         while (*ptr)
         {
            accum_char(output_accum, *ptr++);
         }

         os_free(white);
         white = 0;
      }

      accum_char(output_accum, c);
   }
}

extern void outputs (char * s)
{
   for (;*s;s++)
   {
      outputc(*s);
   }
}

extern void outputd (int n)
{
   char temp[64];

   os_sprintf(temp, "%d", n);
   outputs(temp);
}

extern void input_mark (void)
{
   MARK *m;

   m = NEW(MARK);
   check_os_malloc(m);
   m->link = marks;
   marks = m;
   m->startdepth = fstackdepth;
   m->acc = init_accum();
   m->nignore = 0;
   m->nincs = 0;
   mark_push_here(m, curline(), curfile());
#ifdef DEBUG_IO

   if (debugging)
   {
      outputs("~MARK~");
   }
#endif
}

extern void input_unmark (void)
{
   MARK *m;

   if (marks)
   {
      m = marks;
      marks = m->link;
      os_free(accum_result(m->acc));
      OLD(m);
#ifdef DEBUG_IO

      if (debugging)
      {
         outputs("~UNMARK~");
      }
#endif

   }
}

extern void input_recover (void)
{
   register MARK *m;
   register char *txt;
   register int i;
   register int l;
   char c;

   if (marks)
   {
      m = marks;
      marks = m->link;
#ifdef DEBUG_IO

      if (debugging)
      {
         outputs("~RECOVER~");
      }
#endif
      for (;m->nignore > 0;m->nignore--)
      {
         accum_regret(m->acc);
      }
      txt = accum_result(m->acc);
      i = strlen(txt) - 1;
      while (m->nincs > 0)
      {
         l = m->incs[--m->nincs];
         for (;i >= l;i--)
         {
            Push(txt[i]);
         }
         Push(MARKLEND);
         c = txt[i];
         for (i--;txt[i] != c;i--)
         {
            Push(txt[i]);
         }
         Push('A');
         Push('@');
         Push('@');
         Push('@');
         Push('@');
         Push('@');
         Push('@');
         Push('@');
         for (;(txt[i] != '#') && (txt[i] != '@');i--)
            ;
         Push(MARKLINE);
         i --;
      }
      for (;i >= 0;i--)
      {
         Push(txt[i]);
      }
      os_free(txt);
      OLD(m);
   }
}

extern void mark_file_beginning (void)
{
   register MARK *m;

   for (m = marks;m;m = m->link)
   {
      m->incs[m->nincs++] = accum_howfar(m->acc);
   }
}

extern void mark_file_ending (void)
{
   register MARK *m;
   register int i;
   register int to;
   register char *acc;

   for (m = marks;m;m = m->link)
   {
      if (m->startdepth > fstackdepth)
      {
         m->startdepth = fstackdepth;
         mark_push_here(m, curline(), curfile());
      }
      else if (m->nincs <= 0)
      {
         fprintf(stderr, "INTERNAL BUG: nincs<0 in mark_file_ending\n");
         abort();
      }
      else
      {
         to = m->incs[--m->nincs];
         acc = m->acc;
         for (i = accum_howfar(acc);i > to;i--)
         {
            accum_regret(acc);
         }
      }
   }
}

extern void mark_charpushed (void)
{
   register MARK *m;

   for (m = marks;m;m = m->link)
   {
      m->nignore ++;
   }
}

extern void mark_got_from_pushback (char c)
{
   mark_got(c);
}

extern void mark_got_from_file (char c)
{
   mark_got(c);
}

extern void mark_got (char c)
{
   register MARK *m;

   for (m = marks;m;m = m->link)
   {
      if (m->nignore > 0)
      {
         m->nignore --;
      }
      else
      {
         accum_char(m->acc, c);
      }
   }
}

static void mark_push_here (MARK * m, int l, char * f)
{
   int i;
   char c;

   switch (c = accum_regret(m->acc))
   {
      case 0:
         break;
      case MARKLEND:
         while (accum_regret(m->acc) != MARKLINE)
            ;
         break;
      default:
         accum_char(m->acc, c);
         break;
   }
   accum_char(m->acc, MARKLINE);
   for (i = 28;i >= 0;i -= 4)
   {
      char c = '@' + ((l >> i) & 0xf);
      accum_char(m->acc, c);
   }
   for (;*f;f++)
   {
      accum_char(m->acc, *f);
   }
   accum_char(m->acc, MARKLEND);
}

extern void mark_get_line (void)
{
   int l;
   char *f;
   int i;
   char c;
   MARK *m;

   l = 0;
   for (i = 0;i < 8;i++)
   {
      l = (l << 4) + (GET() & 0xf);
   }
   f = init_accum();
   while ((c = GET()) != MARKLEND)
   {
      accum_char(f, c);
   }
   f = accum_result(f);
   out_at(l, f);
   for (m = marks;m;m = m->link)
   {
      mark_push_here(m, l, f);
   }
   os_free(f);
}

extern int read_char ()
{
   static char *buffer = 0;
   static char *read_ptr = 0;
   int result = 0;

   if (!buffer)
   {
      if (accum_howfar(output_accum) > 0)
      {
         read_ptr = buffer = accum_result(output_accum);
      }
   }

   if (buffer)
   {
      if (*read_ptr)
      {
         result = *read_ptr++;
      }
      else
      {
         os_free(buffer);
         buffer = 0;
         output_accum = init_accum();
      }
   }

   return result;
}
