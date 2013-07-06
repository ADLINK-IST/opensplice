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
#include "symtbl.h"
#include "if.h"

static unsigned char * temp;

extern void autodef_file (const char * f)
{
   int i;

   i = strlen (f);
   temp = os_malloc (i + 2 + 1);
   check_os_malloc (temp);
   os_sprintf ((char*) temp, "\"%s\"", f);
   undef ("__FILE__");
   define ("__FILE__", -1, temp, DEF_DEFINE);
}

extern void autodef_line (int l)
{
   defd ("__LINE__", l);
}
