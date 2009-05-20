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


#include "v_writerCache.h"
#include "v_cache.h"

static c_type _v_writerCache = NULL;
static c_type _v_writerCacheItem = NULL;

c_type
v_writerCache_t (
    c_base base)
{
    if (_v_writerCache == NULL) {
        _v_writerCache = c_resolve(base,"kernelModule::v_writerCache");
    }
    return c_keep(_v_writerCache);
}

c_type
v_writerCacheItem_t (
    c_base base)
{
    if (_v_writerCacheItem == NULL) {
        _v_writerCacheItem = c_resolve(base,"kernelModule::v_writerCacheItem");
    }
    return c_keep(_v_writerCacheItem);
}

v_writerCache
v_writerCacheNew (
    v_kernel kernel,
    v_cacheKind kind)
{
    c_base base;
    c_type type;
    v_writerCache cache;

    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(kernel);
    type = v_writerCache_t(base);
    cache = c_new(type);
    cache->itemType = v_writerCacheItem_t(base);
    v_cacheInit(v_cache(cache),kind);

    assert(C_TYPECHECK(cache,v_writerCache));

    return cache;
}

void
v_writerCacheInsert (
    v_writerCache cache,
    v_writerCacheItem item)
{
    assert(C_TYPECHECK(cache,v_writerCache));
    assert(C_TYPECHECK(item,v_writerCacheItem));

    v_cacheInsert(v_cache(cache),v_cacheNode(item));
}

c_bool
v_writerCacheWalk (
    v_writerCache cache,
    v_cacheWalkAction action,
    c_voidp arg)
{
    assert(C_TYPECHECK(cache,v_writerCache));

    return v_cacheWalk(v_cache(cache),action,arg);
}

void
v_writerCacheDeinit (
    v_writerCache cache)
{
    assert(C_TYPECHECK(cache,v_writerCache));

    v_cacheDeinit(v_cache(cache));
}

v_writerCacheItem
v_writerCacheItemNew (
    v_writerCache cache,
    v_groupInstance instance)
{
    v_writerCacheItem item;

    assert(C_TYPECHECK(cache,v_writerCache));
    assert(C_TYPECHECK(instance,v_groupInstance));

    item = c_new(cache->itemType);
    item->instance = c_keep(instance);
    v_cacheNodeInit(v_cacheNode(item));

    assert(C_TYPECHECK(item,v_writerCacheItem));

    return item;
}

void
v_writerCacheItemRemove (
    v_writerCacheItem item,
    v_cacheKind kind)
{
    assert(C_TYPECHECK(item,v_writerCacheItem));

    v_cacheNodeRemove(v_cacheNode(item),kind);
}

v_groupInstance
v_writerCacheItemInstance (
    v_writerCacheItem item)
{
    assert(C_TYPECHECK(item,v_writerCacheItem));
    assert(C_TYPECHECK(item->instance,v_groupInstance));

    return c_keep(item->instance);
}



