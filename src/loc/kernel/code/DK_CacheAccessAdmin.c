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
/* C includes */
#include <assert.h>
#include <stdio.h>

/* OS abstraction layer includes */
#include "os_heap.h"

/* DLRL utilities includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* collection includes */
#include "Coll_Compare.h"

/* DLRL kernel includes */
#include "DK_CacheAccessAdmin.h"
#include "DK_CacheAccessTypeRegistry.h"
#include "DK_CacheAdmin.h"
#include "DK_ObjectAdmin.h"
#include "DK_ObjectHomeAdmin.h"
#include "DK_Utility.h"
/* NOT IN DESIGN entire class */

static void
DK_CacheAccessAdmin_us_destroy(
    DK_Entity* _this);

static DK_CacheAccessTypeRegistry*
DK_CacheAccessAdmin_us_getRegistry(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home);

static void
DK_CacheAccessAdmin_us_destroySingleRegistry(
    DK_CacheAccessAdmin* _this,
    void* userData,
    LOC_long homeIndex);

static LOC_boolean
DK_CacheAccessAdmin_us_containsInvalidObjects(
    DK_CacheAccessAdmin* _this);

#define ENTITY_NAME "DLRL Kernel CacheAccessAdmin"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

/* requires admin lock of the cache admin and that the object homes list is in
 *tact
 */
DK_CacheAccessAdmin*
DK_CacheAccessAdmin_new(
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_cacheAccess,
    DK_Usage usage,
    DK_CacheAdmin* cache)
{
    DK_CacheAccessAdmin* _this;
    os_mutexAttr mutexAttr;
    os_result resultInit;
    LOC_unsigned_long nrOfHomes = 0;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    /* userData may be null */
    /* ls_cacheAccess may be null */
    assert(usage < DK_Usage_elements);
    assert(cache);

    DLRL_ALLOC(_this, DK_CacheAccessAdmin, exception,
        "%s within DLRL Cache '%s'", allocError, DLRL_VALID_NAME(cache->name));

    _this->base.alive = TRUE;
    _this->base.usage = usage;
    _this->registry = NULL;
    _this->containedTypes = NULL;
    _this->maxContainedTypes = 0;
    _this->currentNrOfTypes = 0;
    _this->owner = (DK_CacheAdmin*)DK_Entity_ts_duplicate((DK_Entity*)cache);
    _this->base.ls_cacheBase = ls_cacheAccess;
    Coll_List_init(&(_this->contracts));
#if 0
    _this->invalidLinks = 0;
#endif

    mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
    resultInit = os_mutexInit(&(_this->mutex), &mutexAttr);
    if(resultInit != os_resultSuccess)
    {
        DK_Entity_ts_release((DK_Entity*)_this->owner);
        os_free(_this);
        _this = NULL;
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
         "%s '%s' mutex init failed", allocError, DLRL_VALID_NAME(cache->name));
    }

    DK_Entity_us_init(
        &(_this->base.entity),
        DK_CLASS_CACHE_ACCESS_ADMIN,
        DK_CacheAccessAdmin_us_destroy);

    nrOfHomes = Coll_List_getNrOfElements(
        DK_CacheAdmin_us_getHomes(_this->owner));
    assert(nrOfHomes > 0);

    DLRL_ALLOC_WITH_SIZE(
        _this->registry,
        (sizeof(DK_CacheAccessTypeRegistry*)*nrOfHomes),
        exception,
        "Unable to allocate array of CacheAccess type registries!");
    memset(_this->registry, 0, (sizeof(DK_CacheAccessTypeRegistry*)*nrOfHomes));

    DLRL_ALLOC_WITH_SIZE(
        _this->containedTypes,
        (sizeof(LOC_long)*nrOfHomes),
        exception,
        "Unable to allocate array of CacheAccess type registry indexes!");
    _this->containedTypes[0] = -1;
    _this->maxContainedTypes = nrOfHomes;

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        DK_CacheAccessAdmin_us_delete(_this, userData);
        DK_Entity_ts_release((DK_Entity*)_this);
        _this = NULL;
    }

    DLRL_INFO(INF_EXIT);
    return _this;
}

/* no homes may be locked! */
void
DK_CacheAccessAdmin_ts_delete(
    DK_CacheAccessAdmin* _this,
    void* userData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */
    /* userData may be null */

    if(_this)
    {
        DK_CacheAccessAdmin_lock(_this);
        DK_CacheAccessAdmin_us_delete(_this, userData);
        DK_CacheAccessAdmin_unlock(_this);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_us_destroy(
    DK_Entity* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if(_this)
    {
        os_mutexDestroy(&(((DK_CacheAccessAdmin*)_this)->mutex));
        os_free((DK_CacheAccessAdmin*)_this);
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_us_resolveElements(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* objectAdmin)
{
    LOC_long index = 0;
    DK_CacheAccessTypeRegistry* registry = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(home);
    assert(objectAdmin);

    index = DK_ObjectHomeAdmin_us_getRegistrationIndex(home);
    assert(index >= 0);
    registry = _this->registry[index];
    if(registry)
    {
        DK_CacheAccessTypeRegistry_us_resolveElements(
            registry,
            exception,
            userData,
            home,
            objectAdmin);
        DLRL_Exception_PROPAGATE(exception);
        /* do not need to check if we can destroy the registry as when we
         * resolve unresolved elements it means we at least have one object in
         * the objects list (the object which may be the target of the
         * unresolved elements)
         */
    }/* else do nothing */

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* requires admin lock of the cache admin and that the object homes list is in*/
/* tact no homes may be locked! */
void
DK_CacheAccessAdmin_us_delete(
    DK_CacheAccessAdmin* _this,
    void* userData)
{
    LOC_unsigned_long nrOfHomes = 0;
    LOC_unsigned_long count = 0;
    LOC_long index = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* userData may be null */

    if(_this->base.alive)
    {
        if(_this->registry)
        {
            if(_this->containedTypes)
            {
                nrOfHomes = Coll_List_getNrOfElements(
                    DK_CacheAdmin_us_getHomes(_this->owner));
                assert(nrOfHomes > 0);
                for(count = 0; (count < nrOfHomes) && (index != -1); count++)
                {
                    index = _this->containedTypes[count];
                    if(index != -1)
                    {
                        DK_CacheAccessTypeRegistry* registry;

                        registry = _this->registry[index];
                        assert(registry);
                        /* deletes & frees the memory */
                        DK_CacheAccessTypeRegistry_ts_destroy(
                            registry,
                            userData);
                        _this->registry[index] = NULL;
                    }
                }
                _this->currentNrOfTypes = 0;
            }
            os_free(_this->registry);
        }
        if(_this->containedTypes)
        {
            os_free(_this->containedTypes);
        }
        while(Coll_List_getNrOfElements(&(_this->contracts)) > 0)
        {
            DK_Contract* aContract;

            aContract = (DK_Contract*)Coll_List_popBack(&(_this->contracts));
            /* TODO ID: 148 not implemented: DK_Contract_ts_delete(aContract,
             * userData); */
            DK_Entity_ts_release((DK_Entity*)aContract);
        }
        if(_this->base.ls_cacheBase)
        {
            utilityBridge.releaseLSInterfaceObject(
                userData,
                _this->base.ls_cacheBase);
            _this->base.ls_cacheBase = NULL;
        }
        if(_this->owner)
        {
            DK_Entity_ts_release((DK_Entity*)_this->owner);
            _this->owner = NULL;
        }
        _this->base.alive = FALSE;
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_ts_getObjects(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    LOC_long index = 0;
    DK_CacheAccessTypeRegistry* registry = NULL;
    LOC_unsigned_long size = 0;
    LOC_unsigned_long count = 0;
    LOC_unsigned_long elementIndex = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* arg may be NULL */

    DK_CacheAccessAdmin_lock(_this);
    DK_CacheAccessAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    /* step 1: determine the total number of objects in the access. This could
     * be optimized by just counting the number of registers/deletions and
     * such.. */
    for(count = 0; count < _this->currentNrOfTypes; count++)
    {
        index = (LOC_long)_this->containedTypes[count];
        assert(index != -1);
        registry = _this->registry[index];
        size += DK_CacheAccessTypeRegistry_us_getNrOfObjects(registry);

    }
    /* now we can visit each object and call the action routine... */
    elementIndex = 0;
    for(count = 0; count < _this->currentNrOfTypes; count++)
    {
        index = (LOC_long)_this->containedTypes[count];
        assert(index != -1);
        registry = _this->registry[index];

        /* if the cache access is alive, all homes must be as well.
         * Unlocked use of this is safe */
        assert(registry->home->alive);
        DK_ObjectHomeAdmin_lockAdmin(registry->home);
        cacheAccessBridge.objectsAction(
            exception,
            userData,
            arg,
            size,
            &elementIndex,
            DK_CacheAccessTypeRegistry_us_getObjects(registry));
        DK_ObjectHomeAdmin_unlockAdmin(registry->home);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAccessAdmin_unlock(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_ts_visitAllObjectsForHome(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    void (*action)( DLRL_Exception*,
                    void*,
                    DK_ObjectAdmin*,
                    LOC_unsigned_long,
                    LOC_unsigned_long,
                    void**),
    void** arg)
{
    LOC_long homeIndex = -1;
    DK_CacheAccessTypeRegistry* registry = NULL;
    LOC_unsigned_long size = 0;
    LOC_unsigned_long elementIndex = 0;
    Coll_Iter* iterator = NULL;
    DK_ObjectAdmin* anObject = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */
    assert(home);
    assert(action);
    /* arg may be null     */

    DK_CacheAccessAdmin_lock(_this);
    DK_ObjectHomeAdmin_lockAdmin(home);
    DK_CacheAccessAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    /* if the cache access is alive, all homes must be as well. */
    assert(home->alive);

    homeIndex = DK_ObjectHomeAdmin_us_getRegistrationIndex(home);
    if(homeIndex != -1)
    {
        registry = _this->registry[homeIndex];
    }
    if(registry)
    {
        size = DK_CacheAccessTypeRegistry_us_getNrOfObjects(registry);
        iterator = Coll_Set_getFirstElement(
            DK_CacheAccessTypeRegistry_us_getObjects(registry));
        while(iterator)
        {
            anObject = Coll_Iter_getObject(iterator);
            /* for each object, call the action routine */
            action(exception, userData, anObject, size, elementIndex, arg);
            DLRL_Exception_PROPAGATE(exception);
            /* dont propagate exception, wait until the unlock has been
             * performed */
            elementIndex++;
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(home);
    DK_CacheAccessAdmin_unlock(_this);
    DLRL_INFO(INF_EXIT);

}

DK_Usage
DK_CacheAccessAdmin_ts_getCacheUsage(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception)
{
    DK_Usage usage = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAccessAdmin_lock(_this);

    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    usage = DK_CacheBase_us_getCacheUsage((DK_CacheBase*)_this);

    DLRL_Exception_EXIT(exception);
    DK_CacheAccessAdmin_unlock(_this);
    DLRL_INFO(INF_EXIT);
    return usage;
}

DLRL_LS_object
DK_CacheAccessAdmin_ts_getLSAccess(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_LS_object retVal = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAccessAdmin_lock(_this);

    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->base.ls_cacheBase)
    {
        retVal = utilityBridge.localDuplicateLSInterfaceObject(
            userData,
            _this->base.ls_cacheBase);
        if(!retVal)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to complete operation, out of resources");
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAccessAdmin_unlock(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

DLRL_LS_object
DK_CacheAccessAdmin_ts_getLSOwner(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_LS_object retVal = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAccessAdmin_lock(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    retVal = DK_CacheAdmin_ts_getLSCache(_this->owner, exception, userData);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_CacheAccessAdmin_unlock(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

void
DK_CacheAccessAdmin_ts_getContainedTypes(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    DLRL_INFO(INF_ENTER);

    DK_CacheAccessAdmin_lock(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    cacheAccessBridge.containedTypesAction(
        exception,
        userData,
        _this->containedTypes,
        _this->currentNrOfTypes, arg);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_CacheAccessAdmin_unlock(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_ts_getContainedTypeNames(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    LOC_unsigned_long count = 0;
    LOC_long index = 0;
    DK_CacheAccessTypeRegistry* registry = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    LOC_string name = NULL;

    DLRL_INFO(INF_ENTER);

    DK_CacheAccessAdmin_lock(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    for(count = 0; count < _this->currentNrOfTypes; count++)
    {
        index = _this->containedTypes[count];
        registry = _this->registry[index];
        home = DK_CacheAccessTypeRegistry_us_getHome(registry);
        DK_ObjectHomeAdmin_lockAdmin(home);
        name = DK_ObjectHomeAdmin_us_getName(home);

        cacheAccessBridge.containedTypeNamesAction(
            exception,
            userData,
            _this->currentNrOfTypes,
            count,
            name,
            arg);
        DK_ObjectHomeAdmin_unlockAdmin(home);/* unlock before propagate */
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAccessAdmin_unlock(_this);
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_CacheAccessAdmin_us_isAlive(
    DK_CacheAccessAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->base.alive;
}

DK_CacheAdmin*
DK_CacheAccessAdmin_us_getOwner(
    DK_CacheAccessAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->owner;
}

LOC_long*
DK_CacheAccessAdmin_us_getTypes(
    DK_CacheAccessAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->containedTypes;
}

LOC_unsigned_long
DK_CacheAccessAdmin_us_getMaxTypes(
    DK_CacheAccessAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->maxContainedTypes;
}

void
DK_CacheAccessAdmin_lock(
    DK_CacheAccessAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    os_mutexLock(&(_this->mutex));

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_unlock(
    DK_CacheAccessAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    os_mutexUnlock(&(_this->mutex));

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_us_checkAlive(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_ts_write(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    LOC_long index = 0;/* must be inited to 0, as -1 is reserved */
    LOC_unsigned_long count = 0;
    DK_CacheAccessTypeRegistry* registry= NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);


    DK_CacheAccessAdmin_lock(_this);
    if(_this->registry)
    {
        for(count = 0; count < _this->currentNrOfTypes; count++)
        {
            registry = _this->registry[_this->containedTypes[count]];
            DK_CacheAccessTypeRegistry_lock(registry);
        }
    }
    DK_CacheAccessAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    if(_this->base.usage == DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
            "This Cache Access has a READ ONLY usage. Therefore this operation "
            "is not allowed for this Cache Access.");
    }

    /* first check if any invalid objects exist within the registries. If so an
     * exception is raised
     */
#if 0
    if(_this->invalidLinks != 0)
#else
    if(DK_CacheAccessAdmin_us_containsInvalidObjects(_this))
#endif
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_INVALID_OBJECTS,
            "Unable to write object changes. Invalid relationships have been detected!");
    }
    for(count = 0; count < _this->currentNrOfTypes; count++)
    {
        index = (LOC_long)_this->containedTypes[count];
        registry = _this->registry[index];
        DK_CacheAccessTypeRegistry_us_commitChanges(
            registry,
            exception,
            userData);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    if(_this->registry)
    {
        for(count = 0; count < _this->currentNrOfTypes; count++)
        {
            registry = _this->registry[_this->containedTypes[count]];
            DK_CacheAccessTypeRegistry_unlock(registry);
            /* if all containing objects are disposed then the registry may be
             * cleaned
             */
            if(DK_CacheAccessTypeRegistry_us_canBeDestroyed(registry))
            {
                DK_CacheAccessAdmin_us_destroySingleRegistry(
                    _this,
                    userData,
                    _this->containedTypes[count]);
                /* when we destroy a registry, we have a chance that a 'hole'
                 * in the registry list forms, which the destroy operation
                 * fixes by filling the hole with the registry at the end of
                 * the list of registries. furthermore the delete operation
                 * lowers the '_this->currentNrOfTypes' by one if we delete
                 * a registry so our evaluation there is not correct either.
                 * In reality we should fix this by replacing the array used
                 * to store all registries with a more sophisticated collection
                 * type, but for now we can hack our way around it by lowering
                 * the count by 1...
                 */
                if(_this->currentNrOfTypes > 0)
                {
                    count--;
                }
            }
        }
    }
    DK_CacheAccessAdmin_unlock(_this);
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_CacheAccessAdmin_us_containsInvalidObjects(
    DK_CacheAccessAdmin* _this)
{
    LOC_boolean invalidObjectsFound = FALSE;
    LOC_unsigned_long i = 0;
    DK_CacheAccessTypeRegistry* registry = NULL;
    Coll_Set* objects = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectAdmin* object = NULL;

    DLRL_INFO(INF_ENTER);

    for(i = 0; (i < _this->currentNrOfTypes) && !invalidObjectsFound; i++)
    {
        assert(_this->containedTypes[i] >= 0);
        registry = _this->registry[_this->containedTypes[i]];
        assert(registry);
        objects = DK_CacheAccessTypeRegistry_us_getObjects(registry);
        iterator = Coll_Set_getFirstElement(objects);
        while(iterator && !invalidObjectsFound)
        {
            object = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
            if(DK_ObjectAdmin_us_hasInvalidRelations(object))
            {
                invalidObjectsFound = TRUE;
            }
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    DLRL_INFO(INF_EXIT);
    return invalidObjectsFound;
}

void
DK_CacheAccessAdmin_ts_getInvalidObjects(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    Coll_List* invalidObjects = NULL;
    LOC_unsigned_long count = 0;
    DK_CacheAccessTypeRegistry* registry = NULL;
    Coll_Set* objects = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectAdmin* object = NULL;
    long errorCode = COLL_OK;

    DLRL_INFO(INF_ENTER);

    DK_CacheAccessAdmin_lockAll(_this);
    DK_CacheAccessAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);
#if 0
    /* only do something if we have invalid objects... */
    if(_this->invalidLinks > 0)
    {
#endif
        /* we first need to locate each invalid object and insert it into
         * a list
         */
        invalidObjects = Coll_List_new();
        DLRL_VERIFY_ALLOC(
            invalidObjects,
            exception,
            "Unable to allocate memory");
        for(count = 0; count < _this->currentNrOfTypes; count++)
        {
            assert(_this->containedTypes[count] >= 0);
            registry = _this->registry[_this->containedTypes[count]];
            assert(registry);
            objects = DK_CacheAccessTypeRegistry_us_getObjects(registry);
            iterator = Coll_Set_getFirstElement(objects);
            while(iterator)
            {
                object = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
                if(DK_ObjectAdmin_us_hasInvalidRelations(object))
                {
                    errorCode = Coll_List_pushBack(invalidObjects, object);
                    if (errorCode != COLL_OK)
                    {
                        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                            "Unable to add an invalid object to list of "
                            "invalid objects.");
                    }
                }
                iterator = Coll_Iter_getNext(iterator);
            }
        }
        /* now call the bridge function to allow the language specific layer
         * to copy it to a structure it needs
         */
        cacheAccessBridge.invalidObjectsAction(
            exception,
            userData,
            arg,
            invalidObjects);
        DLRL_Exception_PROPAGATE(exception);
        /* we can now clear the list we created again */
        while(Coll_List_getNrOfElements(invalidObjects) > 0)
        {
            Coll_List_popBack(invalidObjects);
        }
        Coll_List_delete(invalidObjects);
        invalidObjects = NULL;
#if 0
    }
#endif
    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION && invalidObjects)
    {
        while(Coll_List_getNrOfElements(invalidObjects) > 0)
        {
            Coll_List_popBack(invalidObjects);
        }
        Coll_List_delete(invalidObjects);
        invalidObjects = NULL;

    }
    DK_CacheAccessAdmin_unlockAll(_this);
    DLRL_INFO(INF_EXIT);

}

void
DK_CacheAccessAdmin_ts_purge(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    LOC_long index = 0;
    LOC_unsigned_long count = 0;
    LOC_unsigned_long size = 0;
    DK_Contract* contract = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAccessAdmin_lock(_this);
    DK_CacheAccessAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    for(count = 0; count < _this->maxContainedTypes && (index != -1); count++)
    {
        index = (_this->containedTypes[count]);
        if(index != -1)
        {
            assert(_this->registry[index]);
            /* next operation also frees */
            DK_CacheAccessTypeRegistry_ts_destroy(
                _this->registry[index],
                userData);
            _this->registry[index] = NULL;
            _this->containedTypes[count] = -1;
        }
    }
    _this->currentNrOfTypes = 0;
    size = Coll_List_getNrOfElements(&(_this->contracts));
    for(count = 0; count < size; count++)
    {
        contract = (DK_Contract*)Coll_List_popBack(&(_this->contracts));
        /* TODO ID: 148 not implemented: DK_Contract_ts_delete(contract,
         * userData);
         */
        DK_Entity_ts_release((DK_Entity*)contract);
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAccessAdmin_unlock(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_us_markObjectAsChanged(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* changedAdmin)
{
    DK_ObjectHomeAdmin* home = NULL;
    DK_CacheAccessTypeRegistry* registry;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(changedAdmin);

    home = DK_ObjectAdmin_us_getHome(changedAdmin);
    assert(home);
    registry = DK_CacheAccessAdmin_us_getRegistry(_this, exception, home);
    DLRL_Exception_PROPAGATE(exception);
    DK_CacheAccessTypeRegistry_us_markObjectAsChanged(
        registry,
        exception,
        changedAdmin);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_us_removeUnregisteredObject(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin)
{
    DK_ObjectHomeAdmin* home = NULL;
    DK_CacheAccessTypeRegistry* registry = NULL;
    LOC_long index = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(objectAdmin);

    home = DK_ObjectAdmin_us_getHome(objectAdmin);
    assert(home);
    registry = DK_CacheAccessAdmin_us_getRegistry(_this, exception, home);
    DLRL_Exception_PROPAGATE(exception);

    DK_CacheAccessTypeRegistry_us_removeUnregisteredObject(
        registry,
        exception,
        objectAdmin);
    DLRL_Exception_PROPAGATE(exception);

    if(DK_CacheAccessTypeRegistry_us_canBeDestroyed(registry))
    {
        index = DK_ObjectHomeAdmin_us_getRegistrationIndex(home);
        DK_CacheAccessAdmin_us_destroySingleRegistry(_this, userData, index);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}
/* note this operation is largely the same as
 * DK_CacheAccessAdmin_us_registerUnregisteredObjectAdmin. so if you make
 * changes check if that operation also needs changes */
void
DK_CacheAccessAdmin_us_registerObjectAdmin(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin)
{
    DK_ObjectHomeAdmin* home = NULL;
    DK_CacheAccessTypeRegistry* registry = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(objectAdmin);
    assert(!u_instanceHandleIsNil(DK_ObjectAdmin_us_getHandle(objectAdmin)));

    home = DK_ObjectAdmin_us_getHome(objectAdmin);
    assert(home);
    registry = DK_CacheAccessAdmin_us_getRegistry(_this, exception, home);
    DLRL_Exception_PROPAGATE(exception);

    DK_CacheAccessTypeRegistry_us_registerObject(registry, exception, objectAdmin);
    DLRL_Exception_PROPAGATE(exception);
#if 0
    _this->invalidLinks += DMM_DLRLClass_getNrOfMandatorySingleRelations(
        DK_ObjectHomeAdmin_us_getMetaRepresentative(home));
#endif
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* NOT IN DESIGN */
/* Has to be the in sync of the
 * DK_CacheAccessAdmin_us_unregisterRegisteredObjectAdmin operation
 * note this operation is largely the same as
 * DK_CacheAccessAdmin_us_registerObjectAdmin. so if you make changes check
 * if that operation also needs changes
 */
void
DK_CacheAccessAdmin_us_registerUnregisteredObjectAdmin(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin)
{
    DK_ObjectHomeAdmin* home = NULL;
    DK_CacheAccessTypeRegistry* registry = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(objectAdmin);
    assert(!u_instanceHandleIsNil(DK_ObjectAdmin_us_getHandle(objectAdmin)));

    home = DK_ObjectAdmin_us_getHome(objectAdmin);
    assert(home);
    registry = DK_CacheAccessAdmin_us_getRegistry(_this, exception, home);
    DLRL_Exception_PROPAGATE(exception);

    DK_CacheAccessTypeRegistry_us_registerUnregisteredObject(
        registry,
        exception,
        userData,
        objectAdmin);
    DLRL_Exception_PROPAGATE(exception);
#if 0
    _this->invalidLinks += DMM_DLRLClass_getNrOfMandatorySingleRelations(
        DK_ObjectHomeAdmin_us_getMetaRepresentative(home));
#endif
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* Has to be the in sync of the
 * DK_CacheAccessAdmin_us_registerUnregisteredObjectAdmin operation
 * this is for rollback functionality
 */
void
DK_CacheAccessAdmin_us_unregisterRegisteredObjectAdmin(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin)
{
    DK_ObjectHomeAdmin* home = NULL;
    DK_CacheAccessTypeRegistry* registry = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(objectAdmin);
    assert(DK_ObjectAdmin_us_getIsRegistered(objectAdmin));

    home = DK_ObjectAdmin_us_getHome(objectAdmin);
    assert(home);
    registry = DK_CacheAccessAdmin_us_getRegistry(_this, exception, home);
    DLRL_Exception_PROPAGATE(exception);

    DK_CacheAccessTypeRegistry_us_unregisterRegisteredObject(
        registry,
        exception,
        userData,
        objectAdmin);
    DLRL_Exception_PROPAGATE(exception);
#if 0
    _this->invalidLinks -= DMM_DLRLClass_getNrOfMandatorySingleRelations(
        DK_ObjectHomeAdmin_us_getMetaRepresentative(home));
#endif
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* NOT IN DESIGN */
/* expects an object admin which has already been ref counted */
void
DK_CacheAccessAdmin_us_addUnregisteredObjectAdmin(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin)
{
    DK_ObjectHomeAdmin* home = NULL;
    DK_CacheAccessTypeRegistry* registry = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(objectAdmin);

    home = DK_ObjectAdmin_us_getHome(objectAdmin);
    assert(home);
    registry = DK_CacheAccessAdmin_us_getRegistry(_this, exception, home);
    DLRL_Exception_PROPAGATE(exception);

    DK_CacheAccessTypeRegistry_us_addUnregisteredObject(
        registry,
        exception,
        objectAdmin);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

DK_CacheAccessTypeRegistry*
DK_CacheAccessAdmin_us_getRegistry(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home)
{
    DK_CacheAccessTypeRegistry* registry = NULL;
    LOC_long index = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(home);

    index = DK_ObjectHomeAdmin_us_getRegistrationIndex(home);
    assert((index >= 0) &&
        (((LOC_unsigned_long)index) < (_this->maxContainedTypes)));

    registry = _this->registry[index];
    if(!registry)
    {
        registry = DK_CacheAccessTypeRegistry_new(
            exception,
            home,
            _this->currentNrOfTypes);
        DLRL_Exception_PROPAGATE(exception);
        assert(_this->maxContainedTypes > 0);
        assert(_this->currentNrOfTypes < _this->maxContainedTypes);
        _this->containedTypes[_this->currentNrOfTypes] = index;
        _this->registry[index] = registry;
        _this->currentNrOfTypes++;
        if(_this->currentNrOfTypes < _this->maxContainedTypes)
        {
            _this->containedTypes[_this->currentNrOfTypes] = -1;
        }
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return registry;
}

/* NOT IN DESIGN */
void
DK_CacheAccessAdmin_lockAll(
    DK_CacheAccessAdmin* _this)
{
    LOC_unsigned_long count = 0;
    DK_CacheAccessTypeRegistry* registry;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DK_CacheAccessAdmin_lock(_this);
    if(_this->registry)
    {
        for(count = 0; count < _this->maxContainedTypes; count++)
        {
            registry = _this->registry[count];
            if(registry != NULL)
            {
                DK_CacheAccessTypeRegistry_lock(registry);
            }
        }
    }
    DLRL_INFO(INF_EXIT);
}

/* NOT IN DESIGN */
void
DK_CacheAccessAdmin_unlockAll(
    DK_CacheAccessAdmin* _this)
{
    LOC_unsigned_long count = 0;
    DK_CacheAccessTypeRegistry* registry;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(_this->registry)
    {
        for(count = 0; count < _this->maxContainedTypes; count++)
        {
            registry = _this->registry[count];
            if(registry != NULL)
            {
                DK_CacheAccessTypeRegistry_unlock(registry);
            }
        }
    }

    DK_CacheAccessAdmin_unlock(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_us_unregisterUnresolvedElement(
    DK_CacheAccessAdmin* _this,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectHolder* holder)
{
    DK_CacheAccessTypeRegistry* registry = NULL;
    LOC_long index = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(home);
    assert(holder);

    index = DK_ObjectHomeAdmin_us_getRegistrationIndex(home);
    assert((index >= 0) &&
        (((LOC_unsigned_long)index) < (_this->maxContainedTypes)));
    registry = _this->registry[index];
    assert(registry);
    DK_CacheAccessTypeRegistry_us_unregisterUnresolvedElement(
        registry,
        userData,
        holder);

    if(DK_CacheAccessTypeRegistry_us_canBeDestroyed(registry))
    {
        DK_CacheAccessAdmin_us_destroySingleRegistry(_this, userData, index);
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_us_destroySingleRegistry(
    DK_CacheAccessAdmin* _this,
    void* userData,
    LOC_long homeIndex)
{
    LOC_unsigned_long containedTypesIndex = 0;
    DK_CacheAccessTypeRegistry* registry = NULL;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(homeIndex >= 0);
    assert(_this->registry[homeIndex]);
    assert(_this->currentNrOfTypes > 0);

    registry = _this->registry[homeIndex];
    containedTypesIndex = DK_CacheAccessTypeRegistry_us_getContainedTypesStorageLocation(registry);
    DK_CacheAccessTypeRegistry_us_destroyEmptyRegistry(registry, userData);
    _this->registry[homeIndex] = NULL;


    /* Need to reshuffle the containedTypes indexes
     * The 'DK_CacheAccessAdmin_ts_write' operation is dependant on the logic
     * defined here for reshuffling! So do not change this without changing
     * things there. This dependency is to perform an action there to
     * prevent a bug there. So yeah this is kinda a small hack.
     */
    if(containedTypesIndex < (_this->currentNrOfTypes-1))
    {
        _this->containedTypes[containedTypesIndex] =
            _this->containedTypes[_this->currentNrOfTypes-1];

        DK_CacheAccessTypeRegistry_us_setContainedTypesStorageLocation(
            _this->registry[_this->containedTypes[containedTypesIndex]],
            containedTypesIndex);
    }/*else nothing to do*/
    _this->containedTypes[(_this->currentNrOfTypes-1)] = -1;
    _this->currentNrOfTypes--;

    DLRL_INFO(INF_EXIT);
}
#if 0
void
DK_CacheAccessAdmin_us_decreaseInvalidLinks(
    DK_CacheAccessAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->invalidLinks > 0);

    _this->invalidLinks--;

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessAdmin_us_increaseInvalidLinksWithAmount(
    DK_CacheAccessAdmin* _this,
    LOC_unsigned_long amount)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->invalidLinks += amount;

    DLRL_INFO(INF_EXIT);
}
#endif
