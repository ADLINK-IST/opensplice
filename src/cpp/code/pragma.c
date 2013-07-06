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

extern void do_pragma (void)
{
   char c;

   if (in_false_if())
   {
      while ((c = Get()) != '\n')
         ;
   }
   else
   {
      outputc('#');
      outputc('p');
      outputc('r');
      outputc('a');
      outputc('g');
      outputc('m');
      outputc('a');
      outputc(' ');
      c = Get();
      while ((c != '\n') && (c != -1))
      {
         outputc(c);
         c = Get();
      }
   }
}
