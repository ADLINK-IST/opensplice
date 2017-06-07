/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */


#include "v_writerCache.h"
#include "os_report.h"

v_cache
v_writerCacheNew (
    v_kernel kernel,
    v_cacheKind kind)
{
    c_type type;
    v_cache cache;

    assert(C_TYPECHECK(kernel,v_kernel));

    type = c_keep(v_kernelType(kernel,K_WRITERCACHEITEM));
    cache = v_cacheNew(kernel, type,kind);
    c_free(type);

    if (!cache) {
        OS_REPORT(OS_ERROR,
                  "v_writerCacheNew",V_RESULT_INTERNAL_ERROR,
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
                  "v_writerCacheNew",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate cache item.");
    }
    assert(C_TYPECHECK(item,v_writerCacheItem));

    return item;
}

