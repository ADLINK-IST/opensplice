static const char version[] = "$Id$";

/* 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sd/sprintf.h>
#include <sd/malloc.h>
#include <sd/sd_xplatform.h>

/******************************************************************************/
extern char* sd_sprintf(const char* a_fmt, ...)
{
    char*	buffer;
    va_list	args;

    va_start(args, a_fmt);
    buffer = sd_vsprintf(a_fmt, args);
    va_end(args);

    return buffer;
}

/******************************************************************************/
extern char* sd_vsprintf(const char* a_fmt, va_list a_args)
{
    int		size	= 1024;
    char*	buffer  = sd_calloc(size, sizeof(char));
    
    while (1) {
	int n = vsnprintf(buffer, size, a_fmt, a_args);
	
	/* If that worked, return */
	if (n > -1 && n < size)
	    return buffer;
	
	/* Else try again with more space. */
	if (n > -1)     /* ISO/IEC 9899:1999 */
	    size = n + 1;    
	else            /* twice the old size */
	    size *= 2;      
	
	buffer = sd_realloc(buffer, size);
    }
}

#if defined(__osf__)
#	ifndef snprintf
#		include "sprintf.osf.c"
#	endif
#endif

