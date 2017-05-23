/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
