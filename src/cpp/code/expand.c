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
#include "cpp_malloc.h"
#include "symtbl.h"
#include "io.h"
#include "is.h"
#include "accum.h"
#include "unctrl.h"

#define DEBUG_EXPAND/**/

#ifdef DEBUG_EXPAND
extern int debugging;
#endif

extern int keep_comments;

static char **actuals;
static int *actlens;

static void read_actuals (DEF * d)
{
   int n;
   int i;
   int pc;
   char c;
   char last;
   char quote;
   int backslash;
   int comment;
   char *acc;

   n = d->nargs;
   actuals = (char **) os_malloc(n * sizeof(char *));
   check_os_malloc(actuals);
   actlens = (int *) os_malloc(n * sizeof(int));
   check_os_malloc(actlens);
   c = getnonspace();
   if (c != '(')
   {
      err_head();
      if (n == 0)
      {
         fprintf(stderr, "missing () on %s\n", d->name);
      }
      else
      {
         fprintf(stderr, "missing argument%s to %s\n", (n == 1) ? "" : "s", d->name);
      }
      for (i = 0;i < n;i++)
      {
         actuals[i] = copyofstr("");
         check_os_malloc(actuals[i]);
         actlens[i] = 0;
      }
      Push(c);
      return ;
   }
   if (n == 0)
   {
      c = getnonspace();
      if (c != ')')
      {
         err_head();
         fprintf(stderr, "unwanted argument to %s\n", d->name);
         Push(c);
      }
      return ;
   }
   i = 0;
   while (1)
   {
      pc = 0;
      quote = 0;
      backslash = 0;
      comment = 0;
      c = 0;
      acc = init_accum();
      while (1)
      {
         last = c;
         c = Get();
         accum_char(acc, c);
         if (comment)
         {
            if ((last == '*') && (c == '/'))
            {
               comment = 0;
            }
         }
         else
         {
            if (backslash)
            {
               backslash = 0;
            }
            else if (quote && (c == quote))
            {
               quote = 0;
            }
            else if (c == '\\')
            {
               backslash = 1;
            }
            else if (quote)
            {}
            else if ((last == '/') && (c == '*'))
            {
               comment = 1;
            }
            else if ((c == '\'') || (c == '"'))
            {
               quote = c;
            }
            else if (c == '(')
            {
               pc ++;
            }
            else if (c == ')')
            {
               if (pc > 0)
               {
                  pc --;
               }
               else
               {
                  accum_regret(acc);
                  break;
               }
            }
            else if ((c == ',') && (pc == 0))
            {
               accum_regret(acc);
               break;
            }
         }
      }
      if (i < n)
      {
         actuals[i] = accum_result(acc);
         actlens[i] = strlen(actuals[i]);
         i ++;
      }
      else if (i == n)
      {
         err_head();
         fprintf(stderr, "too many arguments to %s\n", d->name);
         i ++;
      }
      if (c == ')')
      {
         break;
      }
   }
   if (i < n)
   {
      err_head();
      fprintf(stderr, "too few arguments to %s\n", d->name);
      for (;i < n;i++)
      {
         actuals[i] = copyofstr("");
         check_os_malloc(actuals[i]);
         actlens[i] = 0;
      }
   }
}

extern void expand_def (DEF * d)
{
   unsigned char *cp;
   char *dp;
   char *ep;
   char *result;
   int ok;
   int incomm;
   char last;

   if (d->nargs >= 0)
   {
      read_actuals(d);
   }
#ifdef DEBUG_EXPAND
   if (debugging)
   {
      char *cp;
      outputs("~EXPAND:");
      outputs(d->name);
      if (d->nargs == 0)
      {
         outputs("()");
      }
      else if (d->nargs > 0)
      {
         int i;
         for (i = 0;i < d->nargs;i++)
         {
            char c = i ? ',' : '(';
            outputc(c);
            outputs(actuals[i]);
         }
         outputc(')');
      }
      outputs("-->");
      for (cp = (char *)d->repl;*cp;cp++)
      {
         outputs(unctrl(*cp));
      }
      outputc('~');
   }
#endif
   result = init_accum();
   for (cp = d->repl;*cp;cp++)
   {
      if (*cp & 0x80)
      {
         char *dp;
         int i;
         i = *cp & ~0x80;
         for (dp = actuals[i];*dp;dp++)
         {
            accum_char(result, *dp);
         }
      }
      else
      {
         accum_char(result, *cp);
      }
   }
   dp = accum_result(result);
#ifdef DEBUG_EXPAND

   if (debugging)
   {
      outputs("first:");
      for (ep = dp;*ep;ep++)
      {
         outputs(unctrl(*ep));
      }
      outputc('~');
   }
#endif
   result = init_accum();
   last = '\0';
   ok = 1;
   incomm = 0;
   for (ep = dp;*ep;ep++)
   {
      if (!incomm && (last == '/') && (*ep == '*'))
      {
         incomm = 1;
         if (!keep_comments || (strncmp(ep, "**/", 3) == 0))
         {
            accum_regret(result);
            ok = 0;
         }
      }
      if (ok)
      {
         accum_char(result, *ep);
      }
      if ((last == '*') && (*ep == '/'))
      {
         incomm = 0;
         ok = 1;
      }
      last = *ep;
   }
   os_free(dp);
   result = accum_result(result);
#ifdef DEBUG_EXPAND

   if (debugging)
   {
      outputs("/**/strip:");
      outputs(result);
      outputc('~');
   }
#endif
   for (dp = result + strlen(result) - 1;dp >= result;dp--)
   {
      Push(*dp);
   }
   os_free(result);
   if (d->nargs >= 0)
   {
      int i;
      for (i = 0;i < d->nargs;i++)
      {
         os_free(actuals[i]);
      }
      os_free((char *)actuals);
   }
}
