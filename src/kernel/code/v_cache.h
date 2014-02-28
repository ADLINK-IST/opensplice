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
#ifndef V_CACHE_H
#define V_CACHE_H

#include "v_kernel.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define v_cache(o)     (C_CAST((o),v_cache))
#define v_cacheNode(o) (C_CAST((o),v_cacheNode))
#define v_cacheItem(o) (C_CAST((o),v_cacheItem))

OS_API typedef c_bool
(*v_cacheWalkAction)(
    v_cacheNode node,
    c_voidp arg);

OS_API v_cacheNode
v_cacheNodeNew (
    v_cache cache);

OS_API v_cache
v_cacheNew (
    v_kernel kernel,
    c_type nodeType,
    v_cacheKind kind);

OS_API void
v_cacheDeinit (
    v_cache cache);

OS_API void
v_cacheInsert (
    v_cache cache,
    v_cacheNode node);

OS_API c_bool
v_cacheWalk (
    v_cache cache,
    v_cacheWalkAction action,
    c_voidp arg);

OS_API void
v_cacheNodeRemove (
    v_cacheNode node,
    v_cacheKind kind);

OS_API c_bool
v_cacheEmpty (
    v_cache cache);

#undef OS_API

#endif

