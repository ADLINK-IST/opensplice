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


#ifndef V_GROUPCACHE_H
#define V_GROUPCACHE_H

#include "v_kernel.h"
#include "v_cache.h"

#define v_groupCache(o) (C_CAST((o),v_groupCache))
#define v_groupCacheEmpty(_this) v_cacheEmpty(v_cache(_this))

v_cache
v_groupCacheNew (
    v_kernel kernel, 
    v_cacheKind kind);

#define v_groupCacheInsert(_this,item) \
        v_cacheInsert(v_cache(_this),v_cacheNode(item))

#define v_groupCacheWalk(_this,_action,_arg) \
        v_cacheWalk(v_cache(_this),_action,_arg)

#define v_groupCacheDeinit(_this) \
        v_cacheDeinit(v_cache(_this))

/* inner class: v_groupCacheItem */

#define v_groupCacheItem(o) (C_CAST((o),v_groupCacheItem))

#define v_groupCacheRemove(_this,item) \
        v_cacheRemove(v_cache(_this),v_cacheNode(item))

#define v_groupCacheItemRemove(_this,kind) \
        v_cacheNodeRemove(v_cacheNode(_this),kind)

#define v_groupCacheItemInstance(_this) \
        c_keep(v_cacheNode(_this)->instance)

v_groupCacheItem
v_groupCacheItemNew (
    v_groupInstance groupInstance, 
    v_instance instance);

#endif

