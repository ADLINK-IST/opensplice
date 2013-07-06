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

#include "idl.h"
#include "idl_extern.h"
#include "drv_private.h"
#include "drv_link.h"
#include <stdio.h>

char * DRV_param_copy (long arc, char **av)
{
   char * ret = NULL;
   long i;
   int tot_size = 0;

   /* determine total string size */
   for (i = 0;i < arc; i++) {
       tot_size += strlen (av[i]) + 1; /* +1 is for the space to match %s %s */
   }

   ret = new char [tot_size + 1];
   ret[0] = 0;

   if (arc > 0)
   {
      os_sprintf (ret, "%s", av[0]);

      // all drv args are -<string> with no params

      for (i = 1;i < arc; i++)
      {
         if (strcmp("-store", av[i]) != 0 && (av[i][0] == '-'))
         {
            os_sprintf(ret, "%s %s", ret, av[i]);
         }
      }
   }
   return (ret);
}
