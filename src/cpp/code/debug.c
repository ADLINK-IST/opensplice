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
#include "io.h"
#include "if.h"

char buf[BUFSIZ];

int debugging = 0;

extern void do_debug (void)
{
   char c;

   /* fflush(outfile);*/
   c = getnonspace();
   switch (c)
   {
      case '1':
      case 'y':
      case 'Y':
         debugging = 1;
         break;
      default:
         debugging = 0;
         break;
   }
}
