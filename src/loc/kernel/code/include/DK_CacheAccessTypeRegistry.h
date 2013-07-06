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
/* NOT IN DESIGN - entire class */
#ifndef DLRL_KERNEL_CACHE_ACCESS_TYPE_REGISTRY_H
#define DLRL_KERNEL_CACHE_ACCESS_TYPE_REGISTRY_H

/* DLRL Kernel includes */
#include "DK_Types.h"
#include "DK_Entity.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct DK_CacheAccessTypeRegistry_s
{
    LOC_unsigned_long containedTypesIndex;
    Coll_Set unresolvedElements;
    Coll_Set objects;/* unique */
    Coll_Set changedObjects;/* unique, not ref counted */
    Coll_Set unregisteredObjects;/* unique */
    Coll_List newObjects;
    Coll_List modifiedObjects;
    /* TODO when clearing this list (deletedObjects) it might mean that the registry should be removed from the
     * cache access because this registry is empty, so check for that when implementing the clear code!!
     * This only applies when implementing read/write as thats when this 'deletedObjects' list will start to be used!
     */
    Coll_List deletedObjects;
    DK_ObjectHomeAdmin* home;/* ref counted */
};

void
DK_CacheAccessTypeRegistry_us_resolveElements(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* objectAdmin);

void
DK_CacheAccessTypeRegistry_us_markObjectAsChanged(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* changedAdmin);

/* deletes & frees the memory */
void
DK_CacheAccessTypeRegistry_ts_destroy(
    DK_CacheAccessTypeRegistry* _this,
    void* userData);

void
DK_CacheAccessTypeRegistry_us_registerUnregisteredObject(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin);

void
DK_CacheAccessTypeRegistry_us_registerObject(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin);

void
DK_CacheAccessTypeRegistry_us_unregisterRegisteredObject(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin);

/* expects an object admin which has already been ref counted */
void
DK_CacheAccessTypeRegistry_us_addUnregisteredObject(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin);

void
DK_CacheAccessTypeRegistry_us_removeUnregisteredObject(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin);

DK_CacheAccessTypeRegistry*
DK_CacheAccessTypeRegistry_new(
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home,
    LOC_unsigned_long containedTypesIndex);

LOC_unsigned_long
DK_CacheAccessTypeRegistry_us_getContainedTypesStorageLocation(
    DK_CacheAccessTypeRegistry* _this);

void
DK_CacheAccessTypeRegistry_us_setContainedTypesStorageLocation(
    DK_CacheAccessTypeRegistry* _this,
    LOC_unsigned_long containedTypesIndex);

void
DK_CacheAccessTypeRegistry_lock(
    DK_CacheAccessTypeRegistry* _this);

void
DK_CacheAccessTypeRegistry_unlock(
    DK_CacheAccessTypeRegistry* _this);

void
DK_CacheAccessTypeRegistry_us_unregisterUnresolvedElement(
    DK_CacheAccessTypeRegistry* _this,
    void* userData,
    DK_ObjectHolder* holder);

void
DK_CacheAccessTypeRegistry_us_commitChanges(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    void* userData);

DK_ObjectHomeAdmin*
DK_CacheAccessTypeRegistry_us_getHome(
    DK_CacheAccessTypeRegistry* _this);

LOC_unsigned_long
DK_CacheAccessTypeRegistry_us_getNrOfObjects(
    DK_CacheAccessTypeRegistry* _this);

Coll_Set*
DK_CacheAccessTypeRegistry_us_getObjects(
    DK_CacheAccessTypeRegistry* _this);

LOC_boolean
DK_CacheAccessTypeRegistry_us_canBeDestroyed(
    DK_CacheAccessTypeRegistry* _this);

void
DK_CacheAccessTypeRegistry_us_destroyEmptyRegistry(
    DK_CacheAccessTypeRegistry* _this,
    void* userData);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_CACHE_ACCESS_TYPE_REGISTRY_H */
