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


#ifndef V_WRITERCACHE_H
#define V_WRITERCACHE_H

#include "v_kernel.h"
#include "v_cache.h"

#define v_writerCache(o) (C_CAST((o),v_writerCache))
#define v_writerCacheItem(o) (C_CAST((o),v_writerCacheItem))

v_cache
v_writerCacheNew (
    v_kernel kernel,
    v_cacheKind kind);

#define v_writerCacheInsert(_this,item) \
        v_cacheInsert(v_cache(_this),v_cacheNode(item))

#define v_writerCacheWalk(_this,action,arg) \
        v_cacheWalk(v_cache(_this),action,arg)

#define v_writerCacheDeinit(_this) \
        v_cacheDeinit(v_cache(_this))

#define v_writerCacheRemove(_this,item) \
        v_cacheRemove(v_cache(_this),v_cacheNode(item))

/* Inner class: v_writerCacheItem */
 
v_writerCacheItem
v_writerCacheItemNew (
    v_cache cache,
    v_groupInstance instance);

#define v_writerCacheItemInstance(_this) \
        c_keep(v_writerCacheItem(_this)-instance)

#define v_writerCacheItemRemove(_this,kind) \
        v_cacheNodeRemove(v_cacheNode(_this),kind)

#endif

