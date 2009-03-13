
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

#if defined (__cplusplus)
}
#endif

#endif
