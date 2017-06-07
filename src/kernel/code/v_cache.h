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

