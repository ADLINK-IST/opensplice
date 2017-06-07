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
#include "cpp_malloc.h"
#include "is.h"
#include "if.h"
#include "cpp_io.h"
#include "accum.h"
#include "symtbl.h"
#include "include.h"

int nIfiles;
char **Ifiles;

static void read_include_file (char *, int, int);

void init_include (void)
{
   nIfiles = 0;
   /* The following malloc will be realloced later! */
   Ifiles = (char **) os_malloc(1);
   check_os_malloc(Ifiles);
}

void Ifile (const char * f)
{
   Ifiles = (char **) os_realloc((char *)Ifiles, (nIfiles + 1) * sizeof(char *));
   check_os_malloc(Ifiles);
   Ifiles[nIfiles] = copyofstr(f);
   check_os_malloc(Ifiles[nIfiles]);
   nIfiles ++;
}

extern void do_include (int sharp)
{
   char c;
   char *acc;

   if (in_false_if())
   {
      if (sharp)
      {
         flush_sharp_line();
      }
      return ;
   }
   while (1)
   {
      c = getnonspace();
      if (isbsymchar(c))
      {
         char *cp;
         DEF *d;
         cp = init_accum();
         while (issymchar(c))
         {
            accum_char(cp, c);
            c = Get();
         }
         Push(c);
         cp = accum_result(cp);
         d = find_def(cp);
         if (d)
         {
            expand_def(d);
         }
         else
         {
            char *dp;
            for (dp = cp + strlen(cp);dp > cp;dp--)
               Push(*dp);
         }
      }
      else
      {
         break;
      }
   }
   acc = init_accum();
   if (c == '"')
   {
      while (1)
      {
         c = Get();
         if (c == '\n')
         {
            Push('\n');
            c = '"';
            err_head();
            fprintf(stderr, "warning: unterminated %cinclude filename\n",
                    sharp ? '#' : '@');
         }
         if (c == '"')
         {
            break;
         }
         accum_char(acc, c);
      }
      if (sharp)
      {
         flush_sharp_line();
      }
      read_include_file(accum_result(acc), 1, sharp);
   }
   else if (c == '<')
   {
      while (1)
      {
         c = Get();
         if (c == '\n')
         {
            Push('\n');
            c = '>';
            err_head();
            fprintf(stderr, "warning: unterminated %cinclude filename\n",
                    sharp ? '#' : '@');
         }
         if (c == '>')
         {
            break;
         }
         accum_char(acc, c);
      }
      if (sharp)
      {
         flush_sharp_line();
      }
      read_include_file(accum_result(acc), 0, sharp);
   }
   else
   {
      os_free(accum_result(acc));
      err_head();
      fprintf(stderr, "illegal %cinclude filename delimiter\n", sharp ? '#' : '@');
   }
}

static void read_include_file (char * name, int dohere, int sharp)
{
   FILE *f;
   char *n;
   char * temp = os_malloc (1024);
   char *cp;
   extern int incldep;
   extern char *incldep_o;

   f = NULL;
   if (dohere)
   {
      if ((strcmp(curdir(), ".") != 0) &&
          ((name[0] != CPP_FILESEPCHAR_1) && (name[0] != CPP_FILESEPCHAR_2)))
      {
         os_sprintf(temp, "%s%c%s", curdir(), CPP_FILESEPCHAR_DEF, name);
         n = temp;
      }
      else
      {
         n = name;
      }
      f = fopen(n, "r");
   }
   if (f == NULL)
   {
      if ((name[0] == CPP_FILESEPCHAR_1) ||
          (name[0] == CPP_FILESEPCHAR_2))
      {
         f = fopen(name, "r");
         n = name;
      }
      else
      {
         int i;
         n = temp;
         for (i = 0;i < nIfiles;i++)
         {
            os_sprintf(temp, "%s%c%s", Ifiles[i], CPP_FILESEPCHAR_DEF, name);
            f = fopen(temp, "r");
            if (f != NULL)
            {
               break;
            }
         }
      }
   }
   if (f == NULL)
   {
      err_head();
      fprintf(stderr, "can't find include file %s\n", name);
      goto done;
   }
   if (incldep)
   {
      printf("%s: %s\n", incldep_o, n);
   }
   out_at(!sharp, n);
   autodef_file(n);
   autodef_line(1);
   Push('\n');
   Get();
   push_new_file(n, f);
   cp = strrchr(n, CPP_FILESEPCHAR_1);
   if (!cp) {
       cp = strrchr(n, CPP_FILESEPCHAR_2);
   }
   if (cp)
   {
      *cp = '\0';
      cp = copyofstr(n);
   }
   else
   {
      cp = copyofstr(".");
   }
   check_os_malloc(cp);
   *Curdir() = cp;

done:

   os_free (name);
   os_free (temp);
}
