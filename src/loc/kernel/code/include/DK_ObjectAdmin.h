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
#ifndef DLRL_KERNEL_OBJECT_ADMIN_H
#define DLRL_KERNEL_OBJECT_ADMIN_H

/* DLRL util includes */
#include "DLRL_Types.h"

/* Collection includes */
#include "Coll_List.h"
#include "Coll_Set.h"

/* DLRL Kernel includes */
#include "DK_Entity.h"
#include "DK_ObjectHolder.h"
#include "DK_Types.h"
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

struct DK_ObjectAdmin_s
{
    DK_Entity entity;
    LOC_boolean alive;
    /* admin and update mutex of the home need to protect this, unless this is
     * an object admin living in a CacheAccess in which case the admin mutex
     * is enough
     */
    DLRL_LS_object ls_object;
    /* NOT IN DESIGN DLRL_LS_object oidStructure; */
    DK_ObjectHomeAdmin* home;
    u_instanceHandle handle;
    LOC_long noWritersCount;
    LOC_long disposedCount;
    /* shouldnt reset readState to an invalid value (IE DK_ObjectState_elements or higher) */
    /* if one does do this, then you'll have to check the code for where this will cause problems  */
    /* (is_modified of object admin for example as of 07-07-2006) */
    DK_ObjectState readState;
    DK_ObjectState writeState;
    /* array to line up with meta model relations to avoid string compares.  */
    DK_ObjectHolder** relations;
    /* length of the relations array */
    LOC_unsigned_long relationsSize;
    /* this is an array of collection pointers */
    DK_Collection** collections;
    /* length of the collection array */
    LOC_unsigned_long collectionsSize;
    /* store key array. This key array is used for:
     *  - unresolved elements list (as index of each element and to search the list)
     *  - ObjectHolder -> when not pointing to a valid object the array is stored
     *  - CollectionWriter -> to fill in owner or target keys based on this specific array
     *  - Validating relation changes -> when setting a relation in a cache access object when the relation is based on
     *                                                                                                  shared keys
     */
    void* keyArray;
    LOC_unsigned_long keyArraySize;
    Coll_Set isRelatedFrom;
    DK_CacheAccessAdmin* access;/* NOT IN DESIGN?? */
    LOC_boolean isRegistered;/* NOT IN DESIGN */
    /* indicates if the topic (not collections) has changed. In the future this will also be an array (or bitmask) */
    /* as support for multiple topic is integrated...  */
    LOC_boolean topicHasChanged;
    /* booleans which indicate which collections have changed. Their indexes in the array coincide with the collections */
    /* on meta level. */
    LOC_boolean* changedCollections;
};

/* caller must release the pointer */
/* NOT IN DESIGN - params added */
DK_ObjectAdmin*
DK_ObjectAdmin_new(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    u_instanceHandle handle,
    void* keyArray,
    LOC_unsigned_long keyArraysize,
    DK_ObjectState readState,
    DK_ObjectState writeState,
    DK_CacheAccessAdmin* access,
    LOC_boolean isRegistered);

LOC_long
DK_ObjectAdmin_us_getNoWritersCount(
    DK_ObjectAdmin* _this);

void
DK_ObjectAdmin_us_setNoWritersCount(
    DK_ObjectAdmin* _this,
    LOC_long noWritersCount);

LOC_long
DK_ObjectAdmin_us_getDisposedCount(
    DK_ObjectAdmin* _this);

void
DK_ObjectAdmin_us_setDisposedCount(
    DK_ObjectAdmin* _this,
    LOC_long disposedCount);

void
DK_ObjectAdmin_us_delete(
    DK_ObjectAdmin* _this,
    void* userData);

/* NOT IN DESIGN */
void
DK_ObjectAdmin_us_setIsRegistered(
    DK_ObjectAdmin* _this,
    void* userData,
    LOC_boolean isRegistered);

/* NOT IN DESIGN */
LOC_boolean
DK_ObjectAdmin_us_getIsRegistered(
    DK_ObjectAdmin* _this);

/**********************************************************************************************************************
************************* Kernel Thread unsafe calls of the ObjectAdmin (Internal kernel API) *************************
**********************************************************************************************************************/
/* dont have to release the list or the contents */
/* NOT IN DESIGNDK_ObjectHolder** DK_ObjectAdmin_us_getRelations(DK_ObjectAdmin* _this); */
/* NOT IN DESIGN */
DK_ObjectHolder*
DK_ObjectAdmin_us_getSingleRelation(
    DK_ObjectAdmin* _this,
    LOC_unsigned_long index);

void*
DK_ObjectAdmin_us_getKeyValueArray(
    DK_ObjectAdmin* _this);

LOC_unsigned_long
DK_ObjectAdmin_us_getKeyValueArraySize(
    DK_ObjectAdmin* _this);

void
DK_ObjectAdmin_us_addCollection(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    DK_Collection* collection,
    LOC_unsigned_long collectionIndex);

/* NOT IN DESIGNu_instanceHandle DK_ObjectAdmin_us_getHandle(DK_ObjectAdmin* _this); */
DK_ObjectState
DK_ObjectAdmin_us_getReadState(
    DK_ObjectAdmin* _this);

void
DK_ObjectAdmin_us_setHandle(
    DK_ObjectAdmin* _this,
    u_instanceHandle handle);

void
DK_ObjectAdmin_us_setReadState(
    DK_ObjectAdmin* _this,
    DK_ObjectState readState);

void
DK_ObjectAdmin_us_setWriteState(
    DK_ObjectAdmin* _this,
    void* userData,
    DK_ObjectState writeState);

/* destroys any previously set OID structure */
/* NOT IN DESIGN - removed void DK_ObjectAdmin_us_setOID(DK_ObjectAdmin* _this, void* userData, DLRL_LS_object oidStructure); */
/* NOT IN DESIGN */
void
DK_ObjectAdmin_us_addSingleRelation(
    DK_ObjectAdmin* _this,
    DK_ObjectHolder* holder,
    LOC_unsigned_long index);

/* NOT IN DESIGN - userData param */
void
DK_ObjectAdmin_us_setLSObject(
    DK_ObjectAdmin* _this,
    void* userData,
    DLRL_LS_object ls_object);

/* no duplicate done */
DK_Collection*
DK_ObjectAdmin_us_getCollection(
    DK_ObjectAdmin* _this,
    LOC_unsigned_long collectionIndex);

void
DK_ObjectAdmin_us_resetModificationInfoOnCollections(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN */
int
isObjectAdminCacheAccessLessThen(
    void *left,
    void *right);
/* NOT IN DESIGN - param */
void
DK_ObjectAdmin_us_setKeyValueArray(
    DK_ObjectAdmin* _this,
    void* keyArray,
    LOC_unsigned_long keysSize);

void
DK_ObjectAdmin_us_registerIsRelatedFrom(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHolder* holder);

void
DK_ObjectAdmin_us_unregisterIsRelatedFrom(
    DK_ObjectAdmin* _this,
    DK_ObjectHolder* holder);

/* NOT IN DESIGN */
DK_CacheAccessAdmin*
DK_ObjectAdmin_us_getAccess(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN */
LOC_boolean
DK_ObjectAdmin_us_hasInvalidRelations(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN */
void
DK_ObjectAdmin_us_collectionHasChanged(
    DK_ObjectAdmin* _this,
    LOC_unsigned_long collectionIndex);

/* NOT IN DESIGN */
LOC_boolean
DK_ObjectAdmin_us_hasTopicChanged(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN */
void
DK_ObjectAdmin_us_relationHasChanged(
    DK_ObjectAdmin* _this,
    LOC_unsigned_long relationIndex);

/* NOT IN DESIGN */
DK_Collection**
DK_ObjectAdmin_us_getCollections(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN */
LOC_boolean*
DK_ObjectAdmin_us_getChangedCollections(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN */
void
DK_ObjectAdmin_us_resetChangedFlags(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN */
LOC_unsigned_long
DK_ObjectAdmin_us_getCollectionsSize(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN */
void
    DK_ObjectAdmin_us_createCollections(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* NOT IN DESIGN */
void
DK_ObjectAdmin_us_getObjectID(
    DK_ObjectAdmin* _this,
    DK_ObjectID* oid);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_OBJECT_ADMIN_H */
