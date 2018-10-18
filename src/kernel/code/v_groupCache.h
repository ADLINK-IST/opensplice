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

