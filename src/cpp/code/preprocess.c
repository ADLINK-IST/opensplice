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
#include "is.h"
#include "cpp_malloc.h"
#include "accum.h"
#include "io.h"
#include "if.h"
#include "symtbl.h"
#include "include.h"
#include "preprocess.h"

#include <ctype.h>

static char quote;

int keep_comments;
int no_line_lines;
int incldep;
char *incldep_o;
int do_at_ctrls;
extern char *predefs[];

void init_preprocess(void)
{
   int i;
   char *cp;

   init_symtbl();
   for (i = 0; predefs[i]; i++)
   {
      cp = copyofstr("1");
      check_os_malloc(cp);
      define(predefs[i], -1, (unsigned char *) cp, DEF_PREDEF);
   }

   init_include();
   keep_comments = 0;
   no_line_lines = 0;
   do_at_ctrls = 0;
   incldep = 0;
   willbefirst = 1;
   quote = 0;
   ifstack = 0;
   n_skipped_ifs = 0;
}

void preprocess(FILE *infile, const char *infilename)
{
   char *cp;

   init_io(infile, infilename);

   cp = strrchr(infilename, CPP_FILESEPCHAR_1);
   if (!cp) {
      cp = strrchr(infilename, CPP_FILESEPCHAR_2);
   }
   if (cp)
   {
      char save = *cp;
      *cp = '\0';
      init_incldir(infilename);
      *cp = save;
   }
   else
   {
      init_incldir(".");
   }

   autodef_file(infilename);
   autodef_line(1);
   out_at(1, infilename);
}

int preprocess_getc(void)
{
   int result = 0;
   int backslash = 0;

   do
   {
      int c;
      int haddigit;

      result = read_char();

      if (result != 0)
      {
         break;
      }

      c = Get();
      if (c == -1)
      {
         break;
      }
      if (backslash)
      {
         maybe_print(c);
         backslash = 0;
         continue;
      }
      if (!incldep && (isdigit((int) c) || (c == '.')))
      {
         haddigit = 0;
         while (isdigit((int) c) || (c == '.'))
         {
            haddigit |= isdigit((int) c);
            maybe_print(c);
            c = Get();
            if (c == -1)
            {
               return 0;
            }
         }
         if (haddigit && ((c == 'e') || (c == 'E')))
         {
            maybe_print(c);
            c = Get();
            if (c == -1)
            {
               return 0;
            }
            while (isdigit((int) c))
            {
               maybe_print(c);
               c = Get();
               if (c == -1)
               {
                  return 0;
               }
            }
         }
         Push(c);
         continue;
      }
      if (quote)
      {
         if (c == '\\')
         {
            maybe_print(c);
            backslash = 1;
            continue;
         }
         else if ((c == quote) || (c == '\n'))
         {
            maybe_print(c);
            quote = 0;
            continue;
         }
         else
         {
            maybe_print(c);
            continue;
         }
      }
      if (c == '\\') /* this weirdness is Reiser semantics.... */
      {
         backslash = 1;
         maybe_print(c);
         continue;
      }
      if ((c == '\'') || (c == '"'))
      {
         quote = c;
         maybe_print(c);
      }
      else if (linefirst && (c == '#'))
      {
         do_sharp();
      }
      else if (do_at_ctrls && (c == '@'))
      {
         do_at();
      }
      else if (! incldep)
      {
         if (isbsymchar(c) && !in_false_if())
         {
            char *cp;
            DEF *d;
            cp = init_accum();
            while (issymchar(c))
            {
               accum_char(cp, c);
               c = Get();
               if (c == -1)
               {
                  return 0;
               }
            }
            Push(c);
            cp = accum_result(cp);
#ifdef DEBUG_MAIN

            if (debugging)
            {
               outputs("<word:");
               outputs(cp);
               outputs(">");
            }
#endif
            d = find_def(cp);
            if (d)
            {
               expand_def(d);
            }
            else
            {
               for (;*cp;cp++)
               {
                  maybe_print(*cp);
               }
            }
         }
         else if (c == '/')
         {
            int d;
            d = Get();
            if (d == -1)
            {
               return 0;
            }
            if (d == '*')
            {
               d = '\0';
               if (keep_comments)
               {
                  maybe_print('/');
                  maybe_print('*');
               }
               do
               {
                  c = d;
                  d = Get();
                  if (d == -1)
                  {
                     return 0;
                  }
                  if ((d == '\n') || keep_comments)
                  {
                     maybe_print(d);
                  }
               }
               while ((c != '*') || (d != '/'));
            }
            else if (d == '/')
            {
               if (keep_comments)
               {
                  maybe_print('/');
                  maybe_print('/');
               }
               do
               {
                  c = Get();
                  if (c == -1)
                  {
                     return 0;
                  }
                  if ((c == '\n') || keep_comments)
                  {
                     maybe_print(c);
                  }
               }
               while (c != '\n');
            }
            else
            {
               Push(d);
               maybe_print(c);
            }
         }
         else
         {
            maybe_print(c);
         }
      }
   } while (result == 0);

   return result;
}
