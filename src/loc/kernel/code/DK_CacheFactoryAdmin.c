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
/*  c includes */
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* OS abstraction layer includes */
#include "os_heap.h"
#include "os_mutex.h"
#include "os_process.h"
#include "os_stdlib.h"

/* user layer includes */
#include "u_entity.h"

/* DLRL utilities includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* Collection includes */
#include "Coll_Compare.h"
#include "Coll_Defs.h"

/* DLRL Kernel includes */
#include "DK_CacheFactoryAdmin.h"
#include "DLRL_Kernel_private.h"

/* global attribute: CacheFactory is a singleton */
static DK_CacheFactoryAdmin factoryAdmin;
static LOC_boolean singletonInitialized = FALSE;

/* define all method caches */
DK_CacheBridge cacheBridge;
DK_CacheAccessBridge cacheAccessBridge;
DK_CollectionBridge collectionBridge;
DK_DCPSUtilityBridge dcpsUtilityBridge;
DK_ObjectBridge objectBridge;
DK_ObjectHomeBridge objectHomeBridge;
DK_ObjectReaderBridge objectReaderBridge;
DK_ObjectRelationReaderBridge objectRelationReaderBridge;
DK_ObjectWriterBridge objectWriterBridge;
DK_SelectionBridge selectionBridge;
DK_UtilityBridge utilityBridge;

#define ENTITY_NAME "DLRL Kernel CacheFactoryAdmin"

DK_CacheAdmin*
DK_CacheFactoryAdmin_ts_createCache(
    DLRL_Exception* exception,
    DLRL_LS_object ls_cache,
    void* userData,
    DK_Usage purpose,
    LOC_const_string cacheName,
    /* operation becomes owner of the participant, and must clean up if an exception occurs*/
    u_participant participant,
    DLRL_LS_object ls_participant)
{
    LOC_string name = NULL;
    DK_CacheAdmin* cache = NULL;
    DK_CacheAdmin* tempCache = NULL;
    long returnCode = COLL_OK;
    u_result result;
    DLRL_Exception exception2;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(purpose < DK_Usage_elements);
    assert(cacheName);
    assert(participant);
    assert(ls_participant);
    /* ls cache may be null */
    /* user data may be null */

    DLRL_Exception_init(&exception2);
    DK_CacheFactoryAdmin_lock();

    /* see if a cache with this name already exists. If a cache can be found,
     * then the already existing exception should be thrown */
    tempCache = (DK_CacheAdmin*) Coll_Map_get(
        &(factoryAdmin.caches),
        (void*)cacheName);
    if(tempCache)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_ALREADY_EXISTING,
            "Unable to create a DLRL Cache within the %s. A cache with the "
            "name %s already exists!",
            ENTITY_NAME,
                DLRL_VALID_NAME(cacheName));
    }

    DLRL_STRDUP(
        name,
        cacheName,
        exception,
        "Unable to create a DLRL Cache within the %s. Allocation error when "
        "trying to allocate memory to store the cache name!",
        ENTITY_NAME);

    cache = DK_CacheAdmin_new(
        exception,
        ls_cache,
        userData,
        purpose,
        participant,
        ls_participant,
        name);
    DLRL_Exception_PROPAGATE(exception);

    /* reference count of create is used */
    returnCode = Coll_Map_add(
        &(factoryAdmin.caches),
        (void*)name,
        (void*)cache,
        NULL);
    if(returnCode != COLL_OK)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to create a DLRL Cache within the %s. Allocation error "
            "when trying to add the created cache to the map of caches!",
            ENTITY_NAME);
    }
    assert(returnCode != COLL_ERROR_ALREADY_EXISTS);

    cache = (DK_CacheAdmin*)DK_Entity_ts_duplicate((DK_Entity*)cache);

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION)
    {
        if(name)
        {
            os_free(name);
        }
        /* exception occured, rollback to a state before entering this
        * operation
        */
        /* cache may be null */
        if(cache)
        {
            DK_CacheAdmin_ts_delete(cache, userData);
            /* decrease the constructor ref count upgrade */
            DK_Entity_ts_release((DK_Entity*)cache);
            if(returnCode == COLL_OK)
            {
                /* decrease the explicit duplicate */
                DK_Entity_ts_release((DK_Entity*)cache);
            }
            cache = NULL;
        } else
        {
            assert(participant);
            result = u_entityFree(u_entity(participant));
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(
                   &exception2,
                   result,
                   "Unable to free the user layer participant!");
               DLRL_REPORT(
                   REPORT_ERROR,
                   "Exception %s occured when attempting to delete the DCPS "
                        "user layer participant\n%s",
                    DLRL_Exception_exceptionIDToString(exception2.exceptionID),
                   exception2.exceptionMessage);
               DLRL_Exception_init(&exception2);
            }
        }
        /* do not need to rollback the action of adding the cache to the map,
         * as that is the last possible place where an exception could have
         * occurred.
         */
    } /*  else do nothing */
    DK_CacheFactoryAdmin_unlock();
    DLRL_INFO(INF_EXIT);
    return cache;
}

DK_CacheAdmin*
DK_CacheFactoryAdmin_ts_findCachebyName(
    LOC_const_string cacheName)
{
    DK_CacheAdmin* cache = NULL;

    DLRL_INFO(INF_ENTER);

    assert(cacheName);
    assert(singletonInitialized);

    DK_CacheFactoryAdmin_lock();
    cache = (DK_CacheAdmin*) Coll_Map_get(
        &(factoryAdmin.caches),
        (void*)cacheName);
    if(cache)
    {
        cache = (DK_CacheAdmin*)DK_Entity_ts_duplicate((DK_Entity*)cache);
    }
    DK_CacheFactoryAdmin_unlock();

    DLRL_INFO(INF_EXIT);
    return cache;
}

void
DK_CacheFactoryAdmin_ts_deleteCache(
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAdmin* cache)
{
    void* storedKey = NULL;

    DLRL_INFO(INF_ENTER);

    assert(cache);
    assert(exception);
    /* userData may be null */
    assert(singletonInitialized);

    DK_CacheFactoryAdmin_lock();
    /* lock the cache to ensure the cache->name parameter is still valid, if we
     * are dealing with an already deleted cache then it this name would be
     * invalid! We only need to lock the during the alive check, as caches may
     * only be deleted by the cache factory and we already have claimed that
     * lock. So we know this to be save, there is only one cache  factory anyway
     */
    DK_CacheAdmin_lockAdministrative(cache);
    DK_CacheAdmin_us_checkAlive(cache, exception);
    DK_CacheAdmin_unlockAdministrative(cache);
    DLRL_Exception_PROPAGATE(exception);
    /* get the cache so it can be deleted */
    Coll_Map_remove(&(factoryAdmin.caches), cache->name, &storedKey);
    /*  call the cache's destructor */
    DK_CacheAdmin_ts_delete(cache, userData);
    DK_Entity_ts_release((DK_Entity*)cache);
    /* after cache admin is deleted then delete the storedKey, as it is the
     * same as the cache->name which wasnt freed in the cache admin delete
     * (as the cachefactory is the owner)
     */
    os_free((LOC_string)storedKey);

    DLRL_Exception_EXIT(exception);
    DK_CacheFactoryAdmin_unlock();

    DLRL_INFO(INF_EXIT);
}

/* the operation calling this must ensure this operation is threadsafe */
DK_CacheFactoryAdmin*
DK_CacheFactoryAdmin_ts_getInstance(
    DLRL_Exception* exception)
{
    DK_CacheFactoryAdmin *handle = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);

    if(!singletonInitialized)
    {
        os_mutexAttr mutexAttr;
        os_result result;

        /*  Initialize the Map that is to hold all DLRL Caches. */
        Coll_Map_init(&(factoryAdmin.caches), stringIsLessThen);

        /* Setup mutex: Set scope of mutex to local init the mutex with the
         * specified attributes
         */
        mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        result = os_mutexInit(&(factoryAdmin.mutex), &mutexAttr);
        assert(result == os_resultSuccess);
        singletonInitialized = TRUE;
        factoryAdmin.ls_cacheFactory = NULL;
    }
    handle = &factoryAdmin;
    assert(singletonInitialized);

    DLRL_Exception_EXIT(exception); /* NOT IN DESIGN */
    DLRL_INFO(INF_EXIT);
    return handle;
}

DLRL_LS_object
DK_CacheFactoryAdmin_us_getLSCacheFactory()
{
    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_EXIT);
    return factoryAdmin.ls_cacheFactory;
}

void
DK_CacheFactoryAdmin_ts_registerLSCacheFactory(
    DLRL_LS_object ls_cacheFactory)
{
    DLRL_INFO(INF_ENTER);

    assert(ls_cacheFactory);
    assert(singletonInitialized);

    DK_CacheFactoryAdmin_lock();
    factoryAdmin.ls_cacheFactory = ls_cacheFactory;
    DK_CacheFactoryAdmin_unlock();

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheFactoryAdmin_lock()
{
    DLRL_INFO(INF_ENTER);

    assert(singletonInitialized);

    os_mutexLock(&(factoryAdmin.mutex));

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheFactoryAdmin_unlock()
{
    DLRL_INFO(INF_ENTER);

    assert(singletonInitialized);

    os_mutexUnlock(&(factoryAdmin.mutex));

    DLRL_INFO(INF_EXIT);
}

DK_CacheBridge*
DK_CacheFactoryAdmin_us_getCacheBridge()
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return &cacheBridge;
}

DK_CacheAccessBridge*
DK_CacheFactoryAdmin_us_getCacheAccessBridge()
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return &cacheAccessBridge;
}


DK_CollectionBridge*
DK_CacheFactoryAdmin_us_getCollectionBridge()
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return &collectionBridge;
}

DK_DCPSUtilityBridge*
DK_CacheFactoryAdmin_us_getDcpsUtilityBridge()
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return &dcpsUtilityBridge;
}

DK_ObjectBridge*
DK_CacheFactoryAdmin_us_getObjectBridge()
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return &objectBridge;
}

DK_ObjectHomeBridge*
DK_CacheFactoryAdmin_us_getObjectHomeBridge()
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return &objectHomeBridge;
}

DK_ObjectReaderBridge*
DK_CacheFactoryAdmin_us_getObjectReaderBridge()
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return &objectReaderBridge;
}

DK_ObjectRelationReaderBridge*
DK_CacheFactoryAdmin_us_getObjectRelationReaderBridge()
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return &objectRelationReaderBridge;
}

DK_ObjectWriterBridge*
DK_CacheFactoryAdmin_us_getObjectWriterBridge()
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return &objectWriterBridge;
}

DK_SelectionBridge*
DK_CacheFactoryAdmin_us_getSelectionBridge()
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return &selectionBridge;
}

DK_UtilityBridge*
DK_CacheFactoryAdmin_us_getUtilityBridge()
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return &utilityBridge;
}
