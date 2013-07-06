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
#include "if.h"
#include "accum.h"
#include "io.h"
#include "cpp_malloc.h"
#include "symtbl.h"

static int nargs;
static int nfound;
static char **argnames;

static void read_formals (void)
{
   char *arg;
   char c;

   nargs = 0;
   argnames = (char **) os_malloc(0);
   check_os_malloc(argnames);
   while (1)
   {
      arg = read_ident();
      if (arg)
      {
         argnames = (char **) os_realloc((char *)argnames, (nargs + 1) * sizeof(char *));
         check_os_malloc(argnames);
         argnames[nargs] = arg;
         nargs ++;
         c = getnonspace();
         if (c == ')')
         {
            return ;
         }
         else if (c != ',')
         {
            err_head();
            fprintf(stderr, "invalid macro parameter delimiter\n");
         }
      }
      else
      {
         c = Get();
         if ((c == ')') && (nargs == 0))
         {
            return ;
         }
         else
         {
            Push(c);
         }
         err_head();
         fprintf(stderr, "missing/illegal macro parameter\n");
         while (1)
         {
            c = Get();
            if (c == ')')
            {
               return ;
            }
            if (isbsymchar(c))
            {
               Push(c);
               break;
            }
         }
      }
   }
}

extern void do_define (int sharp, int redef)
{
   char *mac;
   char c;
   char *acc;
   unsigned char *repl;
   int quoted;
   int incomment;
   unsigned char *f;
   unsigned char *t;
   unsigned char *g;
   int i;

   if (in_false_if())
   {
      char e = '\0';
      if (sharp)
      {
         while (1)
         {
            c = e;
            e = Get();
            if (e == '\n')
            {
               if (c == '\\')
               {
                  maybe_print('\n');
               }
               else
               {
                  break;
               }
            }
         }
      }
      else
      {
         do
         {
            c = e;
            e = Get();
         }
         while ((c != '@') || (e == '@'));
         Push(e);
      }
      return ;
   }
   mac = read_ident();
   if (! mac)
   {
      err_head();
      fprintf(stderr, "missing/illegal macro name\n");
      flush_sharp_line();
      return ;
   }
   c = Get();
   if (c == '(')
   {
      read_formals();
      nfound = nargs;
      if (nargs > 128)
      {
         err_head();
         fprintf(stderr, "too many macro formals, more than 128 ignored\n");
         nargs = 128;
      }
   }
   else
   {
      argnames = 0;
      nargs = -1;
      nfound = -1;
      if ((c != ' ') && (c != '\t'))
      {
         Push(c);
      }
   }
   quoted = 0;
   incomment = 0;
   acc = init_accum();
   if (sharp)
   {
      while (1)
      {
         c = Get();
         if (quoted && (c == '\n'))
         {
            quoted = 0;
            maybe_print('\n');
         }
         else if (quoted)
         {
            accum_char(acc, '\\');
            accum_char(acc, c);
            quoted = 0;
         }
         else if (c == '/')
         {
            char d = Get();
            accum_char(acc, '/');
            if (d == '*')
            {
               accum_char(acc, '*');
               incomment = 1;
            }
            else
            {
               Push(d);
            }
         }
         else if (incomment)
         {
            accum_char(acc, c);
            if (c == '*')
            {
               char d = Get();
               if (d == '/')
               {
                  accum_char(acc, '/');
                  incomment = 0;
               }
               else
               {
                  Push(d);
               }
            }
            else if (c == '\n')
            {
               maybe_print('\n');
            }
         }
         else if (c == '\\')
         {
            quoted = 1;
         }
         else if (c == '\n')
         {
            break;
         }
         else
         {
            accum_char(acc, c);
         }
      }
   }
   else
   {
      while (1)
      {
         c = Get();
         if (quoted && (c == '@'))
         {
            accum_char(acc, '@');
            quoted = 0;
         }
         else if (quoted)
         {
            Push(c);
            break;
         }
         else if (c == '/')
         {
            char d = Get();
            accum_char(acc, '/');
            if (d == '*')
            {
               accum_char(acc, '*');
               incomment = 1;
            }
            else
            {
               Push(d);
            }
         }
         else if (incomment)
         {
            accum_char(acc, c);
            if (c == '*')
            {
               char d = Get();
               if (d == '/')
               {
                  accum_char(acc, '/');
                  incomment = 0;
               }
               else
               {
                  Push(d);
               }
            }
            else if (c == '\n')
            {
               maybe_print('\n');
            }
         }
         else if (c == '@')
         {
            quoted = 1;
         }
         else
         {
            if (c == '\n')
            {
               maybe_print('\n');
            }
            accum_char(acc, c);
         }
      }
   }
   repl = (unsigned char *) accum_result(acc);
   f = repl;
   t = repl;
   while (*f)
   {
      if (isbsymchar(*f) && !incomment)
      {
         for (g = f;issymchar(*g);g++)
            ;
         c = *g;
         *g = '\0';
         for (i = 0;i < nargs;i++)
         {
            if (strcmp((const char *) f, (const char *) argnames[i]) == 0)
            {
               break;
            }
         }
         if (i < nargs)
         {
            *t++ = 0x80 | i;
            f = g;
         }
         else
         {
            while ((*t++ = *f++))
               ;
            f --;
            t --;
         }
         *g = c;
      }
      else if ((f[0] == '/') && (f[1] == '*'))
      {
         f += 2;
         *t++ = '/';
         *t++ = '*';
         incomment = 1;
      }
      else if (incomment && (f[0] == '*') && (f[1] == '/'))
      {
         f += 2;
         *t++ = '*';
         *t++ = '/';
         incomment = 0;
      }
      else
      {
         *t++ = *f++;
      }
   }
   *t++ = '\0';
   repl = (unsigned char *) os_realloc((char *)repl, t - repl);
   check_os_malloc(repl);
   if (redef)
   {
      undef(mac);
   }
   define(mac, nargs, repl, DEF_DEFINE);
   if (argnames)
   {
      for (i = 0;i < nfound;i++)
      {
         os_free(argnames[i]);
      }
      os_free((char *)argnames);
   }
   os_free(mac);
}
