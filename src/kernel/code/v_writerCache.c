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
#include "os_report.h"

static c_type _v_writerCacheItem = NULL;

c_type
v_writerCacheItem_t (
    c_base base)
{
    if (_v_writerCacheItem == NULL) {
        _v_writerCacheItem = c_resolve(base,"kernelModule::v_writerCacheItem");
    }
    return c_keep(_v_writerCacheItem);
}

v_cache
v_writerCacheNew (
    v_kernel kernel,
    v_cacheKind kind)
{
    c_base base;
    c_type type;
    v_cache cache;

    assert(C_TYPECHECK(kernel,v_kernel));

    base = c_getBase(kernel);
    type = v_writerCacheItem_t(base);
    cache = v_cacheNew(type,kind);
    c_free(type);

    if (!cache) {
        OS_REPORT(OS_ERROR,
                  "v_writerCacheNew",0,
                  "Failed to allocate cache.");
    }

    assert(C_TYPECHECK(cache,v_cache));

    return cache;
}

v_writerCacheItem
v_writerCacheItemNew (
    v_cache cache,
    v_groupInstance instance)
{
    v_writerCacheItem item;

    assert(C_TYPECHECK(cache,v_cache));
    assert(C_TYPECHECK(instance,v_groupInstance));

    item = v_writerCacheItem(v_cacheNodeNew(cache));

    if (item) {
        item->instance = instance;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_writerCacheNew",0,
                  "Failed to allocate cache item.");
    }
    assert(C_TYPECHECK(item,v_writerCacheItem));

    return item;
}

