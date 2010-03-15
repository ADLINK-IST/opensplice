/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */


#include "v_groupCache.h"
#include "v_cache.h"
#include "os_report.h"

static c_type _v_groupCache_t = NULL;
static c_type _v_groupCacheItem_t = NULL;

c_type
v_groupCache_t (
    c_base base)
{
    if (_v_groupCache_t == NULL) {
        _v_groupCache_t = c_resolve(base,"kernelModule::v_groupCache");
    }
    return c_keep(_v_groupCache_t);
}

c_type
v_groupCacheItem_t (
    c_base base)
{
    if (_v_groupCacheItem_t == NULL) {
        _v_groupCacheItem_t = c_resolve(base,"kernelModule::v_groupCacheItem");
    }
    return c_keep(_v_groupCacheItem_t);
}

v_groupCache
v_groupCacheNew (
    v_kernel kernel,
    v_cacheKind kind)
{
    c_base base;
    c_type type;
    v_groupCache cache;

    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(kernel);
    type = v_groupCache_t(base);
    cache = c_new(type);
    c_free(type);
    if (cache) {
        cache->itemType = v_groupCacheItem_t(base);
        v_cacheInit(v_cache(cache),kind);
    } else {
        OS_REPORT(OS_ERROR,
                  "v_groupCacheNew",0,
                  "Failed to allocate group cache.");
    }

    assert(C_TYPECHECK(cache,v_groupCache));

    return cache;
}

void
v_groupCacheInsert (
    v_groupCache cache,
    v_groupCacheItem item)
{
    assert(cache != NULL);
    assert(item != NULL);
    assert(C_TYPECHECK(cache,v_groupCache));
    assert(C_TYPECHECK(item,v_groupCacheItem));

    v_cacheInsert(v_cache(cache),v_cacheNode(item));
}

void
v_groupCacheDeinit (
    v_groupCache cache)
{
    assert(cache != NULL);
    assert(C_TYPECHECK(cache,v_groupCache));

    v_cacheDeinit(v_cache(cache));
    /* the cache should now be empty */
    assert(v_cacheNode(cache)->owner.next == NULL);
    assert(v_cacheNode(cache)->owner.prev == NULL);
    assert(v_cacheNode(cache)->instance.next == NULL);
    assert(v_cacheNode(cache)->instance.prev == NULL);
}

v_groupCacheItem
v_groupCacheItemNew (
    v_groupInstance groupInstance,
    v_instance instance)
{
    v_groupCacheItem item;
    v_groupCache cache;

    assert(groupInstance != NULL);
    assert(instance != NULL);
    assert(C_TYPECHECK(groupInstance,v_groupInstance));
    assert(C_TYPECHECK(instance,v_instance));

    cache = groupInstance->readerInstanceCache;
    item = c_new(cache->itemType);
    if (item) {
        item->instance = c_keep(instance);
        item->groupInstance = groupInstance;
        item->registrationCount = 1;
        item->pendingResends = 0;
        v_cacheNodeInit(v_cacheNode(item));
    } else {
        OS_REPORT(OS_ERROR,
                  "v_groupCacheItemNew",0,
                  "Failed to allocate group cache item.");
    }

    assert(C_TYPECHECK(item,v_groupCacheItem));

    return item;
}

void
v_groupCacheItemRemove (
    v_groupCacheItem item,
    v_cacheKind kind)
{
    assert(C_TYPECHECK(item,v_groupCacheItem));

    v_cacheNodeRemove(v_cacheNode(item),kind);
}

v_instance
v_groupCacheItemInstance (
    v_groupCacheItem item)
{
    assert(C_TYPECHECK(item,v_groupCacheItem));
    assert(C_TYPECHECK(item->instance,v_instance));

    return c_keep(item->instance);
}



