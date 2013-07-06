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
#ifndef DLRL_KERNEL_CACHE_ACCESS_BRIDGE_H
#define DLRL_KERNEL_CACHE_ACCESS_BRIDGE_H

#if defined (__cplusplus)
extern "C" {
#endif

typedef void (*DK_CacheAccessBridge_us_containedTypesAction)(
    DLRL_Exception* exception,
    void* userData,
    LOC_long* indexes,
    LOC_unsigned_long totalSize,
    void** arg);

typedef void (*DK_CacheAccessBridge_us_containedTypeNamesAction)(
    DLRL_Exception* exception,
    void* userData,
    LOC_unsigned_long totalSize,
    LOC_unsigned_long index,
    LOC_string name,
    void** arg);

typedef void (*DK_CacheAccessBridge_us_objectsAction)(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long size,
    LOC_unsigned_long* elementIndex,
    Coll_Set* objects);

typedef void (*DK_CacheAccessBridge_us_invalidObjectsAction)(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    Coll_List* invalidObjects);

typedef struct DK_CacheAccessBridge_s{
    DK_CacheAccessBridge_us_containedTypesAction containedTypesAction;
    DK_CacheAccessBridge_us_containedTypeNamesAction containedTypeNamesAction;
    DK_CacheAccessBridge_us_objectsAction objectsAction;
    DK_CacheAccessBridge_us_invalidObjectsAction invalidObjectsAction;
} DK_CacheAccessBridge;

extern DK_CacheAccessBridge cacheAccessBridge;

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_CACHE_ACCESS_BRIDGE_H */
