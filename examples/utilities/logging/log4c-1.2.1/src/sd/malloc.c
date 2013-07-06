/* xmalloc.c -- malloc with out of memory checking
   Copyright (C) 1990,91,92,93,94,95,96,97 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <sd/error.h>

#if defined(__APPLE__)
# include <sys/time.h>
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#endif /* __APPLE__ */

typedef void (*sd_malloc_handler_t)();

static char*  first_break = NULL;
static sd_malloc_handler_t handler  = NULL;

static void *
fixup_null_alloc (n)
    size_t n;
{
    void* p = 0;

#ifdef HAVE_SBRK
    if (n == 0)
	p = malloc ((size_t) 1);

    if (p == 0) {
	extern char **environ;
	size_t allocated;

	if (first_break != NULL)
	    allocated = (char *) sbrk (0) - first_break;
	else
	    allocated = (char *) sbrk (0) - (char *) &environ;
	sd_error("\nCannot allocate %lu bytes after allocating %lu bytes\n",
		 (unsigned long) n, (unsigned long) allocated);
	
	if (handler) 
	    handler();
	else {
	    sd_error("\n\tMemory exhausted !! Aborting.\n");
	    abort();
	}
    }
#endif
    return p;
}

sd_malloc_handler_t
sd_malloc_set_handler(a_handler)
    sd_malloc_handler_t a_handler;
{
    sd_malloc_handler_t previous = handler;

    handler = a_handler;
    return previous;
}

/* Allocate N bytes of memory dynamically, with error checking.  */

void *
sd_malloc (n)
    size_t n;
{
    void *p;

    p = malloc (n);
    if (p == 0)
	p = fixup_null_alloc (n);
    return p;
}

/* Allocate memory for N elements of S bytes, with error checking.  */

void *
sd_calloc (n, s)
    size_t n, s;
{
    void *p;

    p = calloc (n, s);
    if (p == 0)
	p = fixup_null_alloc (n * s);
    return p;
}

/* Change the size of an allocated block of memory P to N bytes,
   with error checking.
   If P is NULL, run sd_malloc.  */

void *
sd_realloc (p, n)
    void *p;
    size_t n;
{
    if (p == 0)
	return sd_malloc (n);
    p = realloc (p, n);
    if (p == 0)
	p = fixup_null_alloc (n);
    return p;
}

/* Return a newly allocated copy of STRING.  */

char *
sd_strdup (string)
     const char *string;
{
  return strcpy (sd_malloc (strlen (string) + 1), string);
}
