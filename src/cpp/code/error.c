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

extern void err_head (void)
{
   fprintf(stderr, "\"%s\", line %d: ", curfile(), curline());
}

extern void Check_malloc (const char * ptr)
{
   if (ptr == 0)
   {
      fprintf(stderr, "out of memory!\n");
      abort();
   }
}
