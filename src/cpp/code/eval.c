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
#include "io.h"
#include "if.h"
#include "expr.h"

extern void do_eval (void)
{
   char c;
   char temp[64];
   int i;

   if (! in_false_if())
   {
      c = getnonspace();
      if (c != '(')
      {
         err_head();
         fprintf(stderr, "@eval must have ()s\n");
         Push(c);
         return ;
      }
      os_sprintf(temp, "%d", eval_expr(0, 1));
      for (i = strlen(temp) - 1;i >= 0;i--)
      {
         Push(temp[i]);
      }
   }
}
