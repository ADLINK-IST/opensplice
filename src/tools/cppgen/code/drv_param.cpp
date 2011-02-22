/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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
   char tmpstr[1024];
   char *ret = NULL;
   long i;

   if (arc > 0)
   {
      os_sprintf (tmpstr, "%s", av[0]);

      // all drv args are -<string> with no params

      for (i = 1;i < arc; i++)
      {
         if (strcmp("-store", av[i]) != 0 && (av[i][0] == '-'))
         {
            os_sprintf(tmpstr, "%s %s", tmpstr, av[i]);
         }
      }
   }

   ret = new char[strlen(tmpstr) + 1];
   os_strcpy(ret, tmpstr);
   return (ret);
}
