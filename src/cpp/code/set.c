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
#include "symtbl.h"
#include "cpp_malloc.h"
#include "io.h"
#include "if.h"
#include "expr.h"

extern void do_set (void)
{
   char *mac;
   char c;
   char temp[64];

   mac = read_ident();
   if (! mac)
   {
      err_head();
      fprintf(stderr, "@set: missing/illegal macro name\n");
      return ;
   }
   if (! in_false_if())
   {
      char *cp;
      c = getnonspace();
      if (c != '(')
      {
         err_head();
         fprintf(stderr, "@set must have ()s\n");
         Push(c);
         return ;
      }
      os_sprintf(temp, "%d", eval_expr(0, 1));
      undef(mac);
      cp = copyofstr(temp);
      check_os_malloc(cp);
      define(mac, -1, (unsigned char *) cp, DEF_DEFINE);
   }
   os_free(mac);
}
