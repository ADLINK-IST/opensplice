/* $Id$
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef __sd_malloc_h
#define __sd_malloc_h

#include <stddef.h>
#include <stdlib.h>
#include <sd/defs.h>

/**
 * @file malloc.h
 */

__SD_BEGIN_DECLS

typedef void (*sd_malloc_handler_t)();

extern sd_malloc_handler_t sd_malloc_set_handler(void (*a_handler)());

#ifndef __SD_DEBUG__

extern void *sd_malloc(size_t n);
extern void *sd_calloc(size_t n, size_t s);
extern void *sd_realloc(void *p, size_t n);
extern char *sd_strdup (const char *__str);

#else

#define sd_malloc	malloc
#define sd_calloc	calloc
#define sd_realloc	realloc
#define sd_strdup	strdup

#endif

__SD_END_DECLS

#endif
