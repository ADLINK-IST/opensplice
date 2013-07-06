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
#include "symtbl.h"

extern void err_head (void);

extern void do_undef (int sharp)
{
   char *mac;

   if (! in_false_if())
   {
      mac = read_ident();
      if (! mac)
      {
         err_head();
         fprintf(stderr, "missing/illegal macro name\n");
      }
      else
      {
         undef (mac);
      }
   }
   if (sharp)
   {
      flush_sharp_line();
   }
}
