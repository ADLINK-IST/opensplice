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

/* C includes */
#include <assert.h>
#include <stdio.h>

/* OS abstraction layer includes */
#include "os_heap.h"

/* DLRL utilities includes */
#include "DLRL_Util.h"
#include "DLRL_Report.h"
#include "DLRL_Exception.h"

/* collection includes */
#include "Coll_Compare.h"

/* DLRL MetaModel includes */
#include "DMM_DLRLClass.h"

/* DLRL kernel includes */
#include "DK_DCPSUtility.h"
#include "DK_ObjectAdmin.h"
#include "DK_CacheAccessTypeRegistry.h"
#include "DK_ObjectHomeAdmin.h"
#include "DK_ObjectWriterBridge.h"
#include "DK_ObjectWriter.h"
#include "DK_Types.h"
#include "DK_UnresolvedObjectsUtility.h"
#include "DLRL_Kernel_private.h"

#define ENTITY_NAME "DLRL Kernel CacheAccessTypeRegistry"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

static void
DK_CacheAccessTypeRegistry_us_resetModificationInformation(
    DK_CacheAccessTypeRegistry* _this,
    void* userData);

DK_CacheAccessTypeRegistry*
DK_CacheAccessTypeRegistry_new(
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home,
    LOC_unsigned_long containedTypesIndex)
{
    DK_CacheAccessTypeRegistry* _this = NULL;

    DLRL_INFO(INF_ENTER);

    assert(home);
    assert(exception);

    DLRL_ALLOC(_this, DK_CacheAccessTypeRegistry, exception, allocError);

    _this->containedTypesIndex = containedTypesIndex;
    Coll_Set_init(&(_this->unresolvedElements), pointerIsLessThen, TRUE);
    Coll_Set_init(&(_this->objects), pointerIsLessThen, FALSE);
    Coll_Set_init(&(_this->changedObjects), pointerIsLessThen, FALSE);
    Coll_Set_init(&(_this->unregisteredObjects), pointerIsLessThen, FALSE);
    Coll_List_init(&(_this->newObjects));
    Coll_List_init(&(_this->modifiedObjects));
    Coll_List_init(&(_this->deletedObjects));
    _this->home = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)home);

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION && _this)
    {
        DK_Entity_ts_release((DK_Entity*)_this->home);
        os_free(_this);
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

/* only to be used when the registry is empty */
void
DK_CacheAccessTypeRegistry_us_destroyEmptyRegistry(
    DK_CacheAccessTypeRegistry* _this,
    void* userData)
{

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(Coll_Set_getNrOfElements(&(_this->unresolvedElements)) == 0);
    assert(Coll_Set_getNrOfElements(&(_this->changedObjects)) == 0);
    assert(Coll_Set_getNrOfElements(&(_this->objects)) == 0);
    assert(Coll_Set_getNrOfElements(&(_this->unregisteredObjects)) == 0);

    DK_CacheAccessTypeRegistry_us_resetModificationInformation(_this, userData);
    if(_this->home)
    {
        DK_Entity_ts_release((DK_Entity*)_this->home);
        _this->home = NULL;
    }
    os_free(_this);

    DLRL_INFO(INF_EXIT);
}

/* deletes & frees the memory */
/* requires lock on all homes in the CacheAccess */
void
DK_CacheAccessTypeRegistry_ts_destroy(
    DK_CacheAccessTypeRegistry* _this,
    void* userData)
{
    Coll_Iter* iterator = NULL;
    DK_ObjectAdmin* objectAdmin= NULL;
    DLRL_Exception exception;
    DK_ObjectWriter* objWriter = NULL;
    u_writer writer = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DK_CacheAccessTypeRegistry_lock(_this);

    /* clear unresolved list */
    DLRL_Exception_init(&exception);
    DK_UnresolvedObjectsUtility_us_clear(&(_this->unresolvedElements),userData);

    iterator = Coll_Set_getFirstElement(&(_this->changedObjects));
    while(iterator)
    {
        objectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&(_this->changedObjects), objectAdmin);
    }
    assert(_this->home);
    assert(DK_ObjectHomeAdmin_us_isAlive(_this->home));
    objWriter = DK_ObjectHomeAdmin_us_getObjectWriter(_this->home);
    assert(objWriter);
    assert(DK_ObjectWriter_us_isAlive(objWriter));
    writer = DK_ObjectWriter_us_getWriter(objWriter);
    assert(writer);
    iterator = Coll_Set_getFirstElement(&(_this->objects));
    while(iterator)
    {
        objectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&(_this->objects), objectAdmin);
        DK_DCPSUtility_us_unregisterObjectFromWriterInstance(
            &exception,
            objectAdmin,
            writer);
        if(exception.exceptionID != DLRL_NO_EXCEPTION)
        {
          DLRL_REPORT(REPORT_ERROR, "Exception %s occured when attempting to "
            "unregister an object from a writer instance\n%s",
            DLRL_Exception_exceptionIDToString(exception.exceptionID),
            exception.exceptionMessage);

            /* reset the exception, maybe it's used again later in this
             * deletion function. We dont propagate the exception here anyway,
             * so it can do no harm as we already logged the exception directly
             * above.
             */
            DLRL_Exception_init(&exception);
        }
        DK_ObjectAdmin_us_delete(objectAdmin, userData);
        DK_Entity_ts_release((DK_Entity*)objectAdmin);
    }
    iterator = Coll_Set_getFirstElement(&(_this->unregisteredObjects));
    while(iterator)
    {
        objectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&(_this->unregisteredObjects), objectAdmin);
        DK_ObjectAdmin_us_delete(objectAdmin, userData);
        DK_Entity_ts_release((DK_Entity*)objectAdmin);
    }
    DK_CacheAccessTypeRegistry_us_resetModificationInformation(_this, userData);
    DK_CacheAccessTypeRegistry_unlock(_this);
    if(_this->home)
    {
        DK_Entity_ts_release((DK_Entity*)_this->home);
        _this->home = NULL;
    }
    os_free(_this);

    DLRL_INFO(INF_EXIT);
}

/* do not claim home lock, will cause deadlock */
void
DK_CacheAccessTypeRegistry_us_resetModificationInformation(
    DK_CacheAccessTypeRegistry* _this,
    void* userData)
{
    Coll_Iter* iterator = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    /* userData may be NULL */

    iterator = Coll_List_getFirstElement(&(_this->newObjects));
    while(iterator)
    {
        /* TODO ID: ??/implementing read_write */
        iterator = Coll_Iter_getNext(iterator);
    }
     iterator = Coll_List_getFirstElement(&(_this->modifiedObjects));
    while(iterator)
    {
        /* TODO ID: ??/implementing read_write */
        iterator = Coll_Iter_getNext(iterator);
    }
    iterator = Coll_List_getFirstElement(&(_this->deletedObjects));
    while(iterator)
    {
        /* TODO ID: ??/implementing read_write */
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_INFO(INF_EXIT);
}
void
DK_CacheAccessTypeRegistry_us_resolveElements(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectAdmin* objectAdmin)
{
    void* keyArray = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(home);
    assert(objectAdmin);

    keyArray = DK_ObjectAdmin_us_getKeyValueArray(objectAdmin);
    assert(Coll_Set_contains(&(_this->objects), objectAdmin));
    /* dds439: using TRUE, TRUE because this isnt used anyway atm, figure out
     * usage later. Though TRUE, TRUE seems best for now only when cloning
     * enters the playing field do we need to rethink it
     */
    DK_UnresolvedObjectsUtility_us_resolveUnresolvedElements(
        &(_this->unresolvedElements),
        exception,
        userData,
        home,
        objectAdmin,
        keyArray,
        TRUE,
        TRUE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessTypeRegistry_us_markObjectAsChanged(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* changedAdmin)
{
    LOC_long errorCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(changedAdmin);

    errorCode = Coll_Set_addUniqueObject(
        &(_this->changedObjects),
        (void*)changedAdmin);
    /* we ignore the already existing return code */
    if(errorCode == COLL_ERROR_ALLOC)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to add object admin %p to the set of changed objects"
            " of %s '%p'", changedAdmin, ENTITY_NAME, _this);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessTypeRegistry_lock(
    DK_CacheAccessTypeRegistry* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->home);

    DK_ObjectHomeAdmin_lockAdmin(_this->home);

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessTypeRegistry_unlock(
    DK_CacheAccessTypeRegistry* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DK_ObjectHomeAdmin_unlockAdmin(_this->home);

    DLRL_INFO(INF_EXIT);
}

LOC_unsigned_long
DK_CacheAccessTypeRegistry_us_getNrOfObjects(
    DK_CacheAccessTypeRegistry* _this)
{
    LOC_unsigned_long size = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    size = Coll_Set_getNrOfElements(&(_this->objects));

    DLRL_INFO(INF_EXIT);
    return size;
}

Coll_Set*
DK_CacheAccessTypeRegistry_us_getObjects(
    DK_CacheAccessTypeRegistry* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->objects);
}

/* keep in sync with the
 * DK_CacheAccessTypeRegistry_us_unregisterRegisteredObject operation
 */
void
DK_CacheAccessTypeRegistry_us_registerUnregisteredObject(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin)
{
    LOC_long errorCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(objectAdmin);
    assert(!DK_ObjectAdmin_us_getIsRegistered(objectAdmin));
    assert(!u_instanceHandleIsNil(DK_ObjectAdmin_us_getHandle(objectAdmin)));

    /* the object list takes over the ref count of the unregistered objects
     * list
     */
    errorCode = Coll_Set_addUniqueObject(&(_this->objects), (void*)objectAdmin);
    if(errorCode != COLL_OK)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,"Unable to add "
         "object admin %p to the set of registered objects of %s '%p'",
         objectAdmin,
         ENTITY_NAME,
         _this);
    }
    errorCode = Coll_Set_addUniqueObject(
        &(_this->changedObjects),
        (void*)objectAdmin);
    if(errorCode != COLL_OK)
    {
        Coll_Set_remove(&(_this->objects), (void*)objectAdmin);
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,"Unable to add "
            "object admin %p to the set of changed objects of %s '%p'",
            objectAdmin,
            ENTITY_NAME,
            _this);
    }

    assert(Coll_Set_contains(&(_this->unregisteredObjects),(void*)objectAdmin));
    Coll_Set_remove(&(_this->unregisteredObjects), (void*)objectAdmin);
    DK_ObjectAdmin_us_setIsRegistered(objectAdmin, userData, TRUE);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* expects an object admin which has already been ref counted */
void
DK_CacheAccessTypeRegistry_us_registerObject(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin)
{
    LOC_long errorCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(objectAdmin);
    assert(DK_ObjectAdmin_us_getIsRegistered(objectAdmin));
    assert(!u_instanceHandleIsNil(DK_ObjectAdmin_us_getHandle(objectAdmin)));

    errorCode = Coll_Set_addUniqueObject(&(_this->objects), (void*)objectAdmin);
    if(errorCode != COLL_OK)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,"Unable to add "
            "object admin %p to the set of registered objects of %s '%p'",
            objectAdmin,
            ENTITY_NAME,
            _this);
    }
    errorCode = Coll_Set_addUniqueObject(
        &(_this->changedObjects),
        (void*)objectAdmin);
    if(errorCode != COLL_OK)
    {
        Coll_Set_remove(&(_this->objects), (void*)objectAdmin);
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,"Unable to add "
            "object admin %p to the set of changed objects of %s '%p'",
            objectAdmin,
            ENTITY_NAME,
            _this);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* keep in sync with the
 * DK_CacheAccessTypeRegistry_us_registerUnregisteredObject operation
 */
void
DK_CacheAccessTypeRegistry_us_unregisterRegisteredObject(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin)
{
    long errorCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);
    assert(objectAdmin);
    assert(DK_ObjectAdmin_us_getIsRegistered(objectAdmin));

    DK_ObjectAdmin_us_setIsRegistered(objectAdmin, userData, FALSE);
    errorCode = Coll_Set_addUniqueObject(
        &(_this->unregisteredObjects),
        (void*)objectAdmin);
    /* we ignore the already existing return code */
    if(errorCode != COLL_OK)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,"Unable to add "
            "object admin %p to the set of unregistered objects of %s '%p'",
            objectAdmin,
            ENTITY_NAME,
            _this);
    }
    assert(Coll_Set_contains(&(_this->changedObjects), (void*)objectAdmin));
    Coll_Set_remove(&(_this->changedObjects), (void*)objectAdmin);

    assert(Coll_Set_contains(&(_this->objects), (void*)objectAdmin));
    Coll_Set_remove(&(_this->objects), (void*)objectAdmin);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* expects an object admin which has already been ref counted */
void
DK_CacheAccessTypeRegistry_us_addUnregisteredObject(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin)
{
    LOC_long errorCode = COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);
    assert(objectAdmin);
    assert(!DK_ObjectAdmin_us_getIsRegistered(objectAdmin));

    errorCode = Coll_Set_addUniqueObject(
        &(_this->unregisteredObjects),
        (void*)objectAdmin);
    /* we ignore the already existing return code */
    if(errorCode == COLL_ERROR_ALLOC)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,"Unable to add "
            "object admin %p to the set of unregistered objects of %s '%p'",
            objectAdmin,
            ENTITY_NAME,
            _this);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessTypeRegistry_us_removeUnregisteredObject(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(objectAdmin);
    assert(!DK_ObjectAdmin_us_getIsRegistered(objectAdmin));

    assert(Coll_Set_contains(&(_this->unregisteredObjects),(void*)objectAdmin));
    Coll_Set_remove(&(_this->unregisteredObjects), (void*)objectAdmin);
    DK_Entity_ts_release((DK_Entity*)objectAdmin);

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAccessTypeRegistry_us_unregisterUnresolvedElement(
    DK_CacheAccessTypeRegistry* _this,
    void* userData,
    DK_ObjectHolder* holder)
{

    assert(_this);
    assert(holder);
    /* userData may be nil */
#ifndef NDEBUG
    printf("NDEBUG - Not implemented\n");
#endif
}

void
DK_CacheAccessTypeRegistry_us_commitChanges(
    DK_CacheAccessTypeRegistry* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DK_ObjectAdmin* object = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectWriter* objWriter = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);
    /* userData may be null */

    objWriter = DK_ObjectHomeAdmin_us_getObjectWriter(_this->home);
    iterator = Coll_Set_getFirstElement(&(_this->changedObjects));
    while(iterator)
    {
        object = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);
        assert(DK_ObjectAdmin_us_getWriteState(object) !=
            DK_OBJECT_STATE_OBJECT_NOT_MODIFIED);
        if(DK_ObjectAdmin_us_getWriteState(object) ==
            DK_OBJECT_STATE_OBJECT_DELETED)
        {
            DK_ObjectWriter_us_dispose(objWriter, exception, userData, object);
            DLRL_Exception_PROPAGATE(exception);
            Coll_Set_remove(&(_this->objects), object);
            /* release ref count from objects list */
            DK_Entity_ts_release((DK_Entity*)object);
        } else
        {
            DK_ObjectWriter_us_write(objWriter, exception, userData, object);
            DLRL_Exception_PROPAGATE(exception);
            assert(DK_ObjectAdmin_us_getWriteState(object) ==
                DK_OBJECT_STATE_OBJECT_NOT_MODIFIED);
        }
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&(_this->changedObjects), object);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

DK_ObjectHomeAdmin*
DK_CacheAccessTypeRegistry_us_getHome(
    DK_CacheAccessTypeRegistry* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->home;
}

LOC_boolean
DK_CacheAccessTypeRegistry_us_canBeDestroyed(
    DK_CacheAccessTypeRegistry* _this)
{
    LOC_boolean canBeDestroyed = FALSE;

    DLRL_INFO(INF_ENTER);

    if(     (Coll_Set_getNrOfElements(&(_this->unresolvedElements)) == 0) &&
            (Coll_Set_getNrOfElements(&(_this->objects)) == 0)            &&
            (Coll_List_getNrOfElements(&(_this->deletedObjects)) == 0)    &&
            (Coll_Set_getNrOfElements(&(_this->unregisteredObjects)) == 0)
       ){
        /* should be 0 if objects set is empty */
        assert(Coll_Set_getNrOfElements(&(_this->changedObjects)) == 0);
        /* should be 0 if objects set is empty */
        assert(Coll_List_getNrOfElements(&(_this->newObjects)) == 0);
        /* should be 0 if objects set is empty */
        assert(Coll_List_getNrOfElements(&(_this->modifiedObjects)) == 0);

        canBeDestroyed = TRUE;
    }

    DLRL_INFO(INF_EXIT);
    return canBeDestroyed;
}

LOC_unsigned_long
DK_CacheAccessTypeRegistry_us_getContainedTypesStorageLocation(
    DK_CacheAccessTypeRegistry* _this)
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return _this->containedTypesIndex;
}

void
DK_CacheAccessTypeRegistry_us_setContainedTypesStorageLocation(
    DK_CacheAccessTypeRegistry* _this,
    LOC_unsigned_long containedTypesIndex)
{
    DLRL_INFO(INF_ENTER);

    _this->containedTypesIndex = containedTypesIndex;

    DLRL_INFO(INF_EXIT);
}
