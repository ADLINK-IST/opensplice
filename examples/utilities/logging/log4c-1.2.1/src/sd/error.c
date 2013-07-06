static const char version[] = "$Id$";

/* 
 * See the COPYING file for the terms of usage and distribution.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_STDARG_H
#  include <stdarg.h>
#else
#  ifdef HAVE_VARARGS_H
#    include <varargs.h>
#  endif
#endif

#include <sd/error.h>
#include <sd_xplatform.h>

int sd_debug(const char *fmt, ...)
{
    va_list args;
    int r;

    if (!getenv("SD_DEBUG"))
	return 0;

    r = fprintf(stderr, "[DEBUG] ");
    va_start(args, fmt);
    r += vfprintf(stderr, fmt, args);
    va_end(args);
    r += fprintf(stderr, "\n");

    return r;
}

int sd_error(const char *fmt, ...)
{
    va_list args;
    int r;

    if (!getenv("SD_ERROR"))
	return 0;

    r = fprintf(stderr, "[ERROR] ");
    va_start(args, fmt);
    r += vfprintf(stderr, fmt, args);
    va_end(args);
    r += fprintf(stderr, "\n");

    return r;
}
