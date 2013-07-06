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

char *copyofstr (const char * str)
{
   char * cp;

   if (str == 0)
   {
      return (0);
   }
   cp = os_malloc (strlen (str) + 1);
   if (cp == 0)
   {
      return (0);
   }
   os_strcpy(cp, str);
   return (cp);
}

char *copyofblk (const char * blk, int len)
{
   char *cp;

   if (blk == 0)
   {
      return (0);
   }
   cp = os_malloc(len);
   if (cp == 0)
   {
      return (0);
   }
   memcpy(cp, blk, len);
   return (cp);
}
