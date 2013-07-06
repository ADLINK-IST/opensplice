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
#include "io.h"
#include "if.h"
#include "cpp_malloc.h"

extern void do_at (void)
{
   char *w;

   w = read_ctrl();
   if (strcmp(w, "include") == 0)
   {
      do_include(0);
   }
   else if (strcmp(w, "define") == 0)
   {
      do_define(0, 0);
   }
   else if (strcmp(w, "undef") == 0)
   {
      do_undef(0);
   }
   else if (strcmp(w, "redefine") == 0)
   {
      do_define(0, 1);
   }
   else if (strcmp(w, "ifdef") == 0)
   {
      do_ifdef(0);
   }
   else if (strcmp(w, "ifndef") == 0)
   {
      do_ifndef(0);
   }
   else if (strcmp(w, "if") == 0)
   {
      do_if(0);
   }
   else if (strcmp(w, "else") == 0)
   {
      do_else(0);
   }
   else if (strcmp(w, "elif") == 0)
   {
      do_elif(0);
   }
   else if (strcmp(w, "endif") == 0)
   {
      do_endif(0);
   }
   else if (strcmp(w, "set") == 0)
   {
      do_set();
   }
   else if (strcmp(w, "while") == 0)
   {
      do_while();
   }
   else if (strcmp(w, "endwhile") == 0)
   {
      do_endwhile();
   }
   else if (strcmp(w, "dump") == 0)
   {
      do_dump();
   }
   else if (strcmp(w, "debug") == 0)
   {
      do_debug();
   }
   else if (strcmp(w, "eval") == 0)
   {
      do_eval();
   }
   else
   {
      if (! in_false_if())
      {
         err_head();
         fprintf(stderr, "unknown control `%s'\n", w);
      }
   }
   os_free(w);
}
