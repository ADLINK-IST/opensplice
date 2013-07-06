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


#include "v_groupCache.h"
#include "os_report.h"

v_cache
v_groupCacheNew (
    v_kernel kernel,
    v_cacheKind kind)
{
    c_type type;
    v_cache cache;

    assert(C_TYPECHECK(kernel,v_kernel));

    type = c_keep(v_kernelType(kernel,K_GROUPCACHEITEM));
    cache = v_cacheNew(kernel,type,kind);
    c_free(type);

    if (!cache) {
        OS_REPORT(OS_ERROR,
                  "v_groupCacheNew",0,
                  "Failed to allocate group cache.");
    }

    assert(C_TYPECHECK(cache, v_cache));

    return cache;
}

v_groupCacheItem
v_groupCacheItemNew (
    v_groupInstance groupInstance,
    v_instance instance)
{
    v_groupCacheItem item;
    v_cache cache;

    assert(groupInstance != NULL);
    assert(instance != NULL);
    assert(C_TYPECHECK(groupInstance,v_groupInstance));
    assert(C_TYPECHECK(instance,v_instance));

    cache = groupInstance->targetCache;
    item = v_groupCacheItem(v_cacheNodeNew(cache));
    if (item) {
    	v_cacheItem(item)->instance = instance;
    	item->groupInstance = groupInstance;
    	item->registrationCount = 1;
        item->pendingResends = 0;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_groupCacheItemNew",0,
                  "Failed to allocate group cache item.");
    }

    assert(C_TYPECHECK(item,v_groupCacheItem));

    return item;
}


