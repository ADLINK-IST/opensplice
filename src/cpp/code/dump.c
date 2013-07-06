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
#include "symtbl.h"
#include "if.h"
#include "io.h"

static void dump_single (DEF *);

extern void do_dump (void)
{
   int i;
   DEF *d;
   extern char *cur_incldir;
   extern char *incldir[];
   extern int fstackdepth;

   fprintf(stderr,
           "\n\n\tDump of definition table (%d entries in %d buckets):\n\n",
           n_in_table, symtbl_size);
   for (i = 0;i < symtbl_size;i++)
   {
      fprintf(stderr, "Bucket %d:\n", i);
      for (d = symtbl[i];d;d = d->link)
      {
         dump_single(d);
         putc('\n', stderr);
      }
   }
   fprintf(stderr, "\n\tInclude directory stack:\n\n");
   for (i = 0;i < fstackdepth;i++)
   {
      fprintf(stderr, "\t\t%s\n", incldir[i]);
   }
   fprintf(stderr, "\t\t%s\n", cur_incldir);
}

static void dump_single (DEF * d)
{
   unsigned char *cp;

   fprintf(stderr, "\t%s", d->name);
   if (d->nargs == 0)
   {
      fprintf(stderr, "()");
   }
   else if (d->nargs > 0)
   {
      int i;
      for (i = 0;i < d->nargs;i++)
      {
         fprintf(stderr, "%c#%d", i ? ',' : '(', i);
      }
      putc(')', stderr);
   }
   putc(' ', stderr);
   for (cp = d->repl;*cp;cp++)
   {
      if (*cp & 0x80)
      {
         fprintf(stderr, "#%d", (*cp)&~0x80);
      }
      else
      {
         putc(*cp, stderr);
      }
   }
}
