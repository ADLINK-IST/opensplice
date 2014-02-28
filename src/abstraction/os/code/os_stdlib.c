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

#include "os_stdlib.h"

#include "code/os_stdlib_defs.c"
#include "code/os_stdlib_mkdir.c"
#include "code/os_stdlib.c"

size_t
os_strnlen(const char *ptr, size_t maxlen)
{
   size_t len;
   for ( len = 0; len < maxlen && ptr[len] != '\0'; len++ )
   {
   }
   return(len);
}

os_result
os_setenv(const char *name, const char *value)
{
    assert(name != NULL);
    assert(value != NULL);

#ifdef WINCE
    return os_putenv_reg(name, value);
#elif _WIN32
    return ((_putenv_s(name, value) == 0) ? os_resultSuccess : os_resultFail);
#elif defined (OS_SOLARIS_DEFS_H) &&  (OS_SOLARIS_VER < 10) || defined ( _WRS_KERNEL )
    {
       /* setenv didn't exist on Solaris until 5.10. Fallback to putenv... */
       char* new_string = os_malloc(strlen(name) + sizeof("=") + strlen(value));
       if (new_string)
       {
           os_sprintf(new_string, "%s%c%s", name, '=', value);
           return os_putenv(new_string);
       }
    }
    return os_resultFail;
#else
    return ((setenv(name, value, 1) == 0) ? os_resultSuccess : os_resultFail);
#endif
}
