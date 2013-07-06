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
#include "cpp_malloc.h"

DEF **symtbl;
int symtbl_size;
int n_in_table;

static int size_index;
static int sizes[] = { 7, 37, 179, 719, 2003, 8009, 30011, 120011, 0 };

static int hash (unsigned char * s)
{
   register unsigned long int i;

   for (i = 0;*s;s++)
   {
      i = (i >> 8) + (i << 3) + *s;
   }
   i &= ~0x80000000;
   return (i % symtbl_size);
}

extern void init_symtbl (void)
{
   int i;

   symtbl_size = sizes[size_index = 0];
   symtbl = (DEF **) os_malloc(symtbl_size * sizeof(DEF *));
   check_os_malloc(symtbl);
   for (i = 0;i < symtbl_size;i++)
   {
      symtbl[i] = 0;
   }
   n_in_table = 0;
}

static void rehash_up (void)
{
   int osize;
   DEF **otbl;
   int i;
   DEF *d;
   DEF *n;
   int h;

   if ((sizes[size_index] == 0) || (sizes[++size_index] == 0))
   {
      return ;
   }
   osize = symtbl_size;
   otbl = symtbl;
   symtbl_size = sizes[size_index];
   symtbl = (DEF **) os_malloc(symtbl_size * sizeof(DEF *));
   check_os_malloc(symtbl);
   for (i = 0;i < symtbl_size;i++)
   {
      symtbl[i] = 0;
   }
   for (i = 0;i < osize;i++)
   {
      for (d = otbl[i];d;d = n)
      {
         n = d->link;
         h = hash ((unsigned char *) d->name);
         d->link = symtbl[h];
         symtbl[h] = d;
      }
   }
   os_free((char *)otbl);
}

extern void define (const char * name, int nargs, unsigned char * repl, int how)
{
   int h;
   DEF *d;
   char *n;

   n = copyofstr(name);
   h = hash ((unsigned char *) n);
   for (d = symtbl[h];d;d = d->link)
   {
      if (strcmp(d->name, n) == 0)
      {
         break;
      }
   }
   if (d)
   {
      if ( (nargs != d->nargs) || strcmp((const char *) repl, (const char *) d->repl) )
      {
         err_head();
         fprintf(stderr, "%s redefined\n", n);
      }
      os_free((char *)d->repl);
      os_free(d->name);
      d->name = n;
      d->nargs = nargs;
      d->repl = repl;
      d->how = how;
   }
   else
   {
      d = NEW(DEF);
      check_os_malloc(d);
      d->name = n;
      d->nargs = nargs;
      d->repl = repl;
      d->how = how;
      d->link = symtbl[h];
      symtbl[h] = d;
      n_in_table ++;
      if (n_in_table > 2*symtbl_size)
      {
         rehash_up();
      }
   }
   if (strcmp(n, "at_sign_ctrls") == 0)
   {
      extern int do_at_ctrls;
      do_at_ctrls = 1;
   }
}

extern void undef (char * name)
{
   int h;
   DEF **D;
   DEF *d;
   int rv;

   h = hash ((unsigned char *) name);
   for (D = symtbl + h;*D;D = &(*D)->link)
   {
      if (strcmp((*D)->name, name) == 0)
      {
         break;
      }
   }
   rv = 0;
   d = *D;
   if (d)
   {
      *D = d->link;
      os_free(d->name);
      os_free((char *)d->repl);
      OLD(d);
      n_in_table --;
      rv = 1;
   }
   if (strcmp(name, "at_sign_ctrls") == 0)
   {
      extern int do_at_ctrls;
      do_at_ctrls = 0;
   }
}

extern DEF *find_def (char * name)
{
   int h;
   DEF *d;

   h = hash ((unsigned char *) name);
   for (d = symtbl[h];d;d = d->link)
   {
      if (strcmp(d->name, name) == 0)
      {
         break;
      }
   }
   return (d);
}

extern void defd (char * name, int value)
{
   char temp[64];
   char *cp;

   os_sprintf(temp, "%d", value);
   undef(name);
   cp = copyofstr(temp);
   check_os_malloc(cp);
   define (name, -1, (unsigned char *) cp, DEF_DEFINE);
}

extern void undef_predefs (void)
{
   int h;
   DEF **D;
   DEF *d;

   for (h = symtbl_size - 1;h >= 0;h--)
   {
      D = symtbl + h;
      while (*D)
      {
         d = *D;
         if (d->how == DEF_PREDEF)
         {
            os_free(d->name);
            os_free((char *)d->repl);
            *D = d->link;
            OLD(d);
            n_in_table --;
         }
         else
         {
            D = &d->link;
         }
      }
   }
}
