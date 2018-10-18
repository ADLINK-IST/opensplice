/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

