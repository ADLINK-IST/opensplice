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


#ifndef V_WRITERCACHE_H
#define V_WRITERCACHE_H

#include "v_kernel.h"
#include "v_cache.h"

#define v_writerCache(o) (C_CAST((o),v_writerCache))
#define v_writerCacheItem(o) (C_CAST((o),v_writerCacheItem))

v_writerCache
v_writerCacheNew (
    v_kernel kernel,
    v_cacheKind kind);

void
v_writerCacheInsert (
    v_writerCache _this,
    v_writerCacheItem item);

c_bool
v_writerCacheWalk (
    v_writerCache _this,
    v_cacheWalkAction action,
    c_voidp arg);

void
v_writerCacheDeinit (
    v_writerCache _this);

/* Inner class: v_writerCacheItem */
 
v_writerCacheItem
v_writerCacheItemNew (
    v_writerCache cache,
    v_groupInstance instance);

v_groupInstance
v_writerCacheItemObject (
    v_writerCacheItem _this);

void
v_writerCacheItemRemove (
    v_writerCacheItem _this,
    v_cacheKind kind);

#endif

