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

#ifndef C_MMCACHE_H
#define C_MMCACHE_H

#include "c_mmbase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

typedef struct c_mmCache_s  *c_mmCache;

OS_API c_mmCache c_mmCacheCreate (c_mm mm, c_long size, c_long count);
OS_API void      c_mmCacheDestroy (c_mmCache _this);

OS_API void     *c_mmCacheMalloc (c_mmCache _this);
OS_API void      c_mmCacheFree (c_mmCache _this, void *memory);

#undef OS_API

/**
 * Give the amount of memory allocated for the specified cache (including
 * headers).
 *
 * @param _this a valid cache
 * @return the number of bytes allocated for _this
 */
c_size          c_mmCacheGetAllocated(c_mmCache _this);

/**
 * Gives the amount of memory that is free (but preallocated) in the specified
 * cache.
 *
 * @param _this a valid cache
 * @return the number of free (but preallocated) bytes in _this
 */
c_size          c_mmCacheGetFree(c_mmCache _this);

#if defined (__cplusplus)
}
#endif

#endif
