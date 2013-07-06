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
#include "os_stdlib.h"

/* OS abstraction layer includes */
#include "os_heap.h"
#include "os_time.h"/* REMOVE when dds416 is available */

/* user layer includes */
#include "u_topic.h"
#include "u_writer.h"
#include "u_reader.h"

/* DLRL util includes */
#include "DLRL_Report.h"
#include "DLRL_Types.h"
#include "DLRL_Util.h"

/* Collection includes */
#include "Coll_Compare.h"

/* DLRL Meta Model includes */
#include "DMM_DCPSTopic.h"
#include "DMM_DLRLAttribute.h"
#include "DMM_DLRLMultiRelation.h"
#include "DMM_DLRLRelation.h"
#include "DMM_InheritanceTable.h"

/* DLRL kernel includes */
#include "DK_DCPSUtility.h"
#include "DK_DCPSUtilityBridge.h"
#include "DK_ObjectAdmin.h"
#include "DK_ObjectHomeAdmin.h"
#include "DK_CacheAccessTypeRegistry.h"
#include "DK_ObjectReader.h"
#include "DK_ObjectWriter.h"
#include "DK_MMFacade.h"
#include "DK_SelectionAdmin.h"
#include "DK_UnresolvedObjectsUtility.h"
#include "DK_Utility.h"
#include "DK_UtilityBridge.h"
#include "DK_CollectionReader.h"
#include "DK_CollectionWriter.h"

#define ENTITY_NAME "DLRL Kernel ObjectHomeAdmin"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

static void
DK_ObjectHomeAdmin_us_destroy(
    DK_Entity* _this);

static void
DK_ObjectHomeAdmin_us_delete(
    DK_ObjectHomeAdmin* _this,
    void* userData);

static void
DK_ObjectHomeAdmin_us_addChild(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* child);

static void
DK_ObjectHomeAdmin_us_removeParent(
    DK_ObjectHomeAdmin* _this);

static void
DK_ObjectHomeAdmin_us_createObjectReader(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo);

static void
DK_ObjectHomeAdmin_us_createObjectWriter(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo);

void
DK_ObjectHomeAdmin_us_checkObjectforSelection(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_SelectionAdmin* selection,
    Coll_List* objects);

static DK_TopicInfo*
DK_ObjectHomeAdmin_us_createTopicInfo(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DMM_DCPSTopic* metaTopic,
    LOC_boolean isMainTopic);/* boolean might need to be something else if place/extension topics come into play */

/* todo dds416 : temp operation */
/*static void
DK_ObjectHomeAdmin_us_SetUniqueIdTEMP(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    Coll_List* keyFields,
    u_writer writer,
*/ /*    void* dataSample,*/
 /*   DK_ObjectAdmin* objectAdmin,
    DMM_DCPSTopic* mainTopic);
*/
DK_ObjectHomeAdmin*
DK_ObjectHomeAdmin_new(
    DLRL_Exception* exception,
    LOC_string name)
{
    os_mutexAttr updateMutexAttr;
    os_result resultUpdate;
    os_mutexAttr adminMutexAttr;
    os_result resultAdmin;
    DK_ObjectHomeAdmin* _this = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(name);

    DLRL_ALLOC(_this, DK_ObjectHomeAdmin, exception, "%s '%s'",
            allocError, DLRL_VALID_NAME(name));

    /* initialize everything to default values or parameters provided in the create call */
    _this->index = -1;
    _this->alive = TRUE;
    _this->autoDeref = TRUE;
    _this->userData = NULL;
    _this->name = NULL;
    _this->cache = NULL;
    _this->filter = NULL;
    _this->parent = NULL;
    _this->relatedHomes = NULL;
    _this->ls_home = NULL;
    _this->meta_representative = NULL;
    _this->objectReader = NULL;
    _this->objectWriter = NULL;
    Coll_Set_init(&(_this->children), pointerIsLessThen, TRUE);
    Coll_Set_init(&(_this->listeners), pointerIsLessThen, TRUE);
    Coll_Set_init(&(_this->selections), pointerIsLessThen, TRUE);
    Coll_List_init(&(_this->topicInfos));

    /* Setup mutex: Set scope of mutex to local */
    updateMutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
    /* Setup mutex: init the mutex with the specified attributes */
    resultUpdate = os_mutexInit(&(_this->updateMutex), &updateMutexAttr);
    if(resultUpdate != os_resultSuccess)
    {
        os_free(_this);
        _this = NULL;
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "%s '%s'",
            allocError, DLRL_VALID_NAME(name));
    }

    /* Setup mutex: Set scope of mutex to local */
    adminMutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
    /* Setup mutex: init the mutex with the specified attributes */
    resultAdmin = os_mutexInit(&(_this->adminMutex), &adminMutexAttr);
    if(resultAdmin != os_resultSuccess)
    {
        os_mutexDestroy (&(_this->updateMutex));
        os_free(_this);
        _this = NULL;
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "%s '%s'",
            allocError, DLRL_VALID_NAME(name));
    }

    DK_Entity_us_init(&(_this->entity), DK_CLASS_OBJECT_HOME_ADMIN, DK_ObjectHomeAdmin_us_destroy);

    DLRL_STRDUP(_this->name, name, exception, "%s '%s'", allocError, DLRL_VALID_NAME(name));

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        DK_ObjectHomeAdmin_us_delete(_this, NULL);
        DK_Entity_ts_release(&(_this->entity));
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
DK_ObjectHomeAdmin_ts_delete(
    DK_ObjectHomeAdmin* _this,
    void* userData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* userData may be null */
    /* this may be null */

    if(_this)
    {
        DK_ObjectHomeAdmin_lockUpdate(_this);
        DK_ObjectHomeAdmin_lockAdmin(_this);
        DK_ObjectHomeAdmin_us_delete(_this, userData);
        DK_ObjectHomeAdmin_unlockAdmin(_this);
        DK_ObjectHomeAdmin_unlockUpdate(_this);
    }

    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_ObjectHomeAdmin_ts_getAutoDeref(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception)
{
    LOC_boolean autoDeref = FALSE;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    autoDeref = DK_ObjectHomeAdmin_us_getAutoDeref(_this);

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return autoDeref;
}

DLRL_LS_object
DK_ObjectHomeAdmin_ts_getLSHome(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_LS_object retVal = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->ls_home)
    {
        retVal = utilityBridge.localDuplicateLSInterfaceObject(userData, _this->ls_home);
        if(!retVal)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to complete operation, out of resources");
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

void
DK_ObjectHomeAdmin_ts_getLSSelections(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    DK_Utility_us_copySelectionsIntoTypedSelectionSeq(
        exception,
        userData,
        _this,
        &(_this->selections),
        arg);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_ts_getLSListeners(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    DK_Utility_us_copyListenersIntoTypedListenerSeq(
        exception,
        userData,
        _this,
        &(_this->listeners),
        arg);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);

}

void
DK_ObjectHomeAdmin_ts_getModifiedObjects(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    Coll_Iter* iterator = NULL;
    LOC_unsigned_long size = 0;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->objectReader)
    {
        iterator = Coll_List_getFirstElement(&(_this->objectReader->modifiedSamples));
        size = Coll_List_getNrOfElements(&(_this->objectReader->modifiedSamples));
    }
    DK_Utility_us_copyObjectsIntoTypedObjectSeq(
        exception,
        userData,
        _this,
        iterator,
        size,
        arg,
        FALSE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_ts_getCreatedObjects(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    Coll_Iter* iterator = NULL;
    LOC_unsigned_long size = 0;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->objectReader)
    {
        iterator = Coll_List_getFirstElement(&(_this->objectReader->newSamples));
        size = Coll_List_getNrOfElements(&(_this->objectReader->newSamples));
    }
    DK_Utility_us_copyObjectsIntoTypedObjectSeq(
        exception,
        userData,
        _this,
        iterator,
        size,
        arg,
        FALSE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_ts_getDeletedObjects(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    Coll_Iter* iterator = NULL;
    LOC_unsigned_long size = 0;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->objectReader)
    {
        iterator = Coll_List_getFirstElement(&(_this->objectReader->deletedSamples));
        size = Coll_List_getNrOfElements(&(_this->objectReader->deletedSamples));
    }
    DK_Utility_us_copyObjectsIntoTypedObjectSeq(
        exception,
        userData,
        _this,
        iterator,
        size,
        arg,
        FALSE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
}

LOC_long
DK_ObjectHomeAdmin_ts_getRegistrationIndex(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception)
{
    LOC_long index = -1;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    index = DK_ObjectHomeAdmin_us_getRegistrationIndex(_this);

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return index;
}

/* caller must release the object home. */
DK_ObjectHomeAdmin*
DK_ObjectHomeAdmin_ts_getParent(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception)
{
    DK_ObjectHomeAdmin* parent = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->parent)
    {
        parent = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)(_this->parent));
    }
    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return parent;
}

LOC_boolean
DK_ObjectHomeAdmin_ts_attachListener(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object listener,
    LOC_boolean concernsContained)
{
    LOC_long returnCode;
    LOC_boolean success = FALSE;
    LOC_boolean alreadyExists = FALSE;
    Coll_Iter* iterator;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_ObjectHomeAdmin_lockUpdate(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    if (!listener)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER,
            "Unable to add a NULL listener to %s '%s'",
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }

    /* verify the listener isnt already known within the set of listeners */
    iterator = Coll_Set_getFirstElement(&(_this->listeners));
    while(iterator && !alreadyExists)
    {
        DLRL_LS_object aListener = (DLRL_LS_object)Coll_Iter_getObject(iterator);
        if(utilityBridge.areSameLSObjects(userData, aListener, listener))
        {
            alreadyExists = TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    /* if the listener was not known, then it should be added */
    if(!alreadyExists)
    {
        returnCode = Coll_Set_add(&(_this->listeners), listener);
        if(returnCode != COLL_OK)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to add listener '%p' to %s '%s'",
                listener, ENTITY_NAME, DLRL_VALID_NAME(_this->name));
        }
        success = TRUE;
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockUpdate(_this);
    DLRL_INFO(INF_EXIT);
    return success;
}

DLRL_LS_object
DK_ObjectHomeAdmin_ts_detachListener(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object listener)
{
    DLRL_LS_object kernelListener = NULL;
    Coll_Iter* iterator;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */
    DK_ObjectHomeAdmin_lockUpdate(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    if (!listener)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_PARAMETER,
            "Unable to detach a NULL listener from %s '%s'",
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }

    /* find the listener to detach */
    iterator = Coll_Set_getFirstElement(&(_this->listeners));
    while(iterator && !kernelListener && listener)
    {
        DLRL_LS_object aListener = (DLRL_LS_object)Coll_Iter_getObject(iterator);
        if(utilityBridge.areSameLSObjects(userData, aListener, listener))
        {
            kernelListener = aListener;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    /* remove the listener */
    if(kernelListener)
    {
        Coll_Set_remove(&(_this->listeners), kernelListener);
    }
    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockUpdate(_this);
    DLRL_INFO(INF_EXIT);
    return kernelListener;
}

DK_SelectionAdmin*
DK_ObjectHomeAdmin_ts_createSelection(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_selection,
    DK_SelectionCriterion* criterion,
    DK_CriterionKind kind,
    LOC_boolean autoRefresh,
    LOC_boolean concernsContainedObjects)
{
    DK_SelectionAdmin* selection = NULL;
    LOC_long returnCode= COLL_OK;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */
    assert(ls_selection);
    assert(criterion);
    assert(kind < DK_CriterionKind_elements);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(!_this->cache)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to "
            "create a selection at %s '%s'. The %s must be registered to a "
            "Cache entity first.",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name),
            ENTITY_NAME);
    }
    /* no topics implicitly means the owning cache is still in initial pubsub
     * mode, so lets raise an exception
     */
    if(Coll_List_getNrOfElements(&(_this->topicInfos)) == 0)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to "
            "create a selection at %s '%s'. The Cache entity to which this %s "
            "belongs may not be in INITIAL pub sub mode.",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name),
            ENTITY_NAME);
    }
    selection = DK_SelectionAdmin_new(
        exception,
        userData,
        _this,
        ls_selection,
        criterion,
        kind,
        autoRefresh,
        concernsContainedObjects);
    DLRL_Exception_PROPAGATE(exception);
    /* insert into set thingy */
    returnCode = Coll_Set_add(&(_this->selections), selection);
    if(returnCode != COLL_OK)
    {
        DK_SelectionAdmin_us_delete(selection, userData);
        DK_Entity_ts_release((DK_Entity*)selection);
        selection = NULL;
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to add "
            "selection to the set of selections of %s %s, out of resources",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name));
    }
    selection=(DK_SelectionAdmin*)DK_Entity_ts_duplicate((DK_Entity*)selection);

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return selection;
}

void
DK_ObjectHomeAdmin_ts_deleteSelection(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_SelectionAdmin* selection)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(selection);
    /* userData may be null */

    DK_ObjectHomeAdmin_lockUpdate(_this);
    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    DK_SelectionAdmin_us_checkAlive(selection, exception);
    DLRL_Exception_PROPAGATE(exception);
    if(!Coll_Set_contains(&(_this->selections), (void*)selection))
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to delete the selection. "
                                "Selection %p was not created by %s '%s'.", ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    /* remove it from the set of selections */
    Coll_Set_remove(&(_this->selections), (void*)selection);
    /* delete it and all its contents */
    DK_SelectionAdmin_us_delete(selection, userData);
    /* finally decrease the reference count */
    DK_Entity_ts_release((DK_Entity*)selection);

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DK_ObjectHomeAdmin_unlockUpdate(_this);
    DLRL_INFO(INF_EXIT);
}

/* calling party must take care of any releasing needed on the returned object home if its not null */
DLRL_LS_object
DK_ObjectHomeAdmin_ts_registerLSObjectHome(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    DLRL_LS_object ls_home)
{
    DLRL_LS_object oldVal = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* ls_home may be null*/

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    oldVal = _this->ls_home;
    _this->ls_home = ls_home;

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return oldVal;
}

void
DK_ObjectHomeAdmin_ts_setUserData(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* homeUserData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* homeUserData may be null */

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    _this->userData = homeUserData;

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
}

/* during the execution of this operation the Cache-updateMutex is locked. */
/* This is important for the correct execution of this operation. */
/* assumes the owning home is locked as well as any related homes needed to manage relations. */
void
DK_ObjectHomeAdmin_us_collectObjectUpdates(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ReadInfo* info)
{
    DLRL_INFO_OBJECT(INF_ENTER);
    DLRL_INFO(INF, " Updating for object home with name %s", _this->meta_representative->name);

    assert(_this);
    assert(exception);
    assert(info);
    assert(_this->objectReader);/* object reader must be present */
    /* userData may be null */

    DK_ObjectReader_us_collectObjectUpdates(_this->objectReader, exception, userData, _this, info);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_us_updateSelections(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    Coll_Iter* iterator;
    DK_SelectionAdmin* selection;
    Coll_Iter* objectIterator;
    DK_ObjectAdmin* objectAdmin;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    iterator = Coll_Set_getFirstElement(&(_this->selections));
    while(iterator)
    {
        selection = (DK_SelectionAdmin*)Coll_Iter_getObject(iterator);

        if(DK_SelectionAdmin_us_getAutoRefresh(selection))
        {

            DK_ObjectHomeAdmin_us_checkObjectforSelection(
                _this,
                exception,
                userData,
                selection,
                &(_this->objectReader->newSamples));
            DLRL_Exception_PROPAGATE(exception);
            DK_ObjectHomeAdmin_us_checkObjectforSelection(
                _this,
                exception,
                userData,
                selection,
                &(_this->objectReader->modifiedSamples));
            DLRL_Exception_PROPAGATE(exception);


            objectIterator = Coll_List_getFirstElement(&(_this->objectReader->deletedSamples));
            while(objectIterator)
            {
                objectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(objectIterator);
                DK_SelectionAdmin_us_addMember(
                    selection,
                    exception,
                    objectAdmin,
                    FALSE);
                DLRL_Exception_PROPAGATE(exception);
                objectIterator = Coll_Iter_getNext(objectIterator);
            }
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_us_checkObjectforSelection(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_SelectionAdmin* selection,
    Coll_List* objects)
{
    Coll_Iter* objectIterator;
    DK_ObjectAdmin* objectAdmin;
    LOC_boolean belongs;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(selection);
    assert(objects);
    /* userData may be null */

    objectIterator = Coll_List_getFirstElement(objects);
    while(objectIterator)
    {
        objectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(objectIterator);
        belongs = objectHomeBridge.checkObjectForSelection(
            _this,
            exception,
            userData,
            selection,
            objectAdmin);
        DLRL_Exception_PROPAGATE(exception);
        DK_SelectionAdmin_us_addMember(
            selection,
            exception,
            objectAdmin,
            belongs);
        DLRL_Exception_PROPAGATE(exception);
        objectIterator = Coll_Iter_getNext(objectIterator);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* during the execution of this operation the Cache-updateMutex is locked. */
/* This is important for the correct execution of this operation. */
/* assumes the owning home is locked as well as any related homes needed to manage relations. */
void
DK_ObjectHomeAdmin_us_processObjectUpdates(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ReadInfo* info)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(info);
    assert(_this->objectReader);/* object reader must be present */
    /* userData may be null */

    DK_ObjectReader_us_processObjectUpdates(_this->objectReader, exception, userData, _this, &info->dataSamples);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* during the execution of this operation the Cache-updateMutex is locked. */
/* This is important for the correct execution of this operation. */
/* assumes the owning home is locked as well as any related homes needed to manage relations. */
void
DK_ObjectHomeAdmin_us_processAllObjectRelationUpdates(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ReadInfo* info)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(info);
    assert(_this->objectReader);/* object reader must be present */
    /* userData may be null */

    DK_ObjectReader_us_processRelationUpdates(_this->objectReader, exception, userData, _this, &info->dataSamples);
    DLRL_Exception_PROPAGATE(exception);


    DK_ObjectReader_us_processCollectionUpdates(_this->objectReader, exception, userData);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_us_clearAllRelationsToDeletedObjects(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ReadInfo* info)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(info);
    assert(_this->objectReader);/* object reader must be present */
    /* userData may be null */

    DK_ObjectReader_us_clearAllRelationsToDeletedObjects(_this->objectReader, exception, userData, _this,
                                                                                &info->dataSamples);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_ts_enableDCPSEntities(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(&(_this->topicInfos))
    {
        Coll_Iter* iterator = Coll_List_getFirstElement(&(_this->topicInfos));
        while(iterator)
        {
            DK_TopicInfo* aTopicInfo = Coll_Iter_getObject(iterator);
            DK_TopicInfo_us_enable(aTopicInfo, exception, userData);
            DLRL_Exception_PROPAGATE(exception);
            iterator = Coll_Iter_getNext(iterator);
        }
    }
    if(_this->objectReader)
    {
        DK_ObjectReader_us_enable(_this->objectReader, exception, userData);
        DLRL_Exception_PROPAGATE(exception);
    }
    if(_this->objectWriter)
    {
        DK_ObjectWriter_us_enable(_this->objectWriter, exception, userData);
        DLRL_Exception_PROPAGATE(exception);
    }
    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
}

DLRL_LS_object
DK_ObjectHomeAdmin_ts_createLSObject(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAccessAdmin* access)
{
    DK_WriterData writerData; /* on stack def */
    DK_ObjectAdmin* objectAdmin;
    DLRL_LS_object ls_object;
    DLRL_LS_object retVal = NULL;
    /*u_writer writer;*/
    u_instanceHandle handle;
    Coll_List* keyFields;
    LOC_unsigned_long keyArraySize;
    /*c_long offset = 0;*/
    DMM_DCPSTopic* mainTopic;
    LOC_string topicTypeName = NULL;/* must be inited to NIL */
    DK_ObjectID oid;
    DK_ObjectState readState = DK_OBJECT_STATE_OBJECT_VOID;/* init this to void state */
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(access);

    DK_CacheAccessAdmin_lock(access);
    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED, "Unable to create object. This %s '%p' is"
                                " already deleted.", ENTITY_NAME, _this);
    }
    if(!DK_CacheAccessAdmin_us_isAlive(access))
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to create object. The "
                            "CacheAccess into which the newly created object is to be inserted is already deleted.");
    }
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)access) == DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to create object. Object "
                    "creation is not allowed in a READ ONLY Cache Access. You can only create objects within a WRITE "
                    "ONLY or READ WRITE Cache Access.");
    }
    if(!_this->cache)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to create object. This %s '%p' "
                "is not yet registered with a Cache", ENTITY_NAME, _this);
    }
    if(_this->cache != DK_CacheAccessAdmin_us_getOwner(access))
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to create object. This %s '%p' "
                "belongs to a different Cache then the provided CacheAccess", ENTITY_NAME, _this);
    }
    if(DK_CacheAdmin_us_getPubSubState(_this->cache) != DK_PUB_SUB_STATE_ENABLED)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to create object. The Cache "
            "to which this %s '%p' belongs is not yet enabled for publication/subscribtion. Objects can only be created"
            " for object homes registered to an enabled Cache.", ENTITY_NAME, _this);
    }
    assert(_this->meta_representative);/*must exist if the pubsubstate is enabled...*/
    if(DMM_DLRLClass_getMapping(_this->meta_representative) != DMM_MAPPING_DEFAULT)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "%s '%p': Unable to create object. The Object type "
            "is mapped using predefined mapping rules. Objects can only be created"
            " by this operation if they are mapped using default mapping rules.", ENTITY_NAME, _this);
    }
/* step 1) create and prepare an object admin */
    /* maybe we need the not_modified read state if this access is readable */
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)access) != DK_USAGE_WRITE_ONLY)
    {
        readState = DK_OBJECT_STATE_OBJECT_NOT_MODIFIED;
    }
    /* Create an object admin with a nil handle, set it's registered state to TRUE */
    objectAdmin = DK_ObjectAdmin_new(exception, userData, _this, DK_DCPSUtility_ts_getNilHandle(), NULL, 0,
                                                                readState, DK_OBJECT_STATE_OBJECT_NEW, access, TRUE);
    DLRL_Exception_PROPAGATE(exception);
    /* now create an language specific object and link it to the just created admin */
    ls_object = objectHomeBridge.createTypedObject(exception, userData, _this,
                                                DK_ObjectWriter_us_getTopicInfo(_this->objectWriter), NULL, objectAdmin);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectAdmin_us_setLSObject(objectAdmin, userData, ls_object);
    /* TODO creating collections here consistitues a problem when we have the possibility of unresolved collections
     * in the unresolved list in a cache access for this object. Currently this is not the case, as we dont use
     * the unresolved list, but in the future it may lead to problems. However it is currently unclear if unresolved
     * collections will even exist in a cache access scope. Analyze this later on, no time atm
     */
    DK_ObjectAdmin_us_createCollections(objectAdmin, exception, userData);
    DLRL_Exception_PROPAGATE(exception);

    mainTopic = DMM_DLRLClass_getMainTopic(_this->meta_representative);
    keyFields = DMM_DCPSTopic_getKeyFields(mainTopic);
    writerData.exception = exception;
    writerData.objectAdmin = objectAdmin;
    writerData.mainTopic = mainTopic;
    writerData.keyFields = keyFields;
    writerData.writer = DK_ObjectWriter_us_getWriter(_this->objectWriter);
    handle = DK_ObjectWriter_us_registerInstance(_this->objectWriter,
                                                exception,
                                                userData,
                                                objectAdmin,
                                                &writerData);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectAdmin_us_setHandle(objectAdmin, handle);

    /* step 3) Update the key values in the language specific object with the key fields obtained in the previous step
     * if we have 4 key fields we also need to take the topic type name into account, take note this is the same field
     * used during the creation of the unique keys for the object!
     */
    keyArraySize = Coll_List_getNrOfElements(keyFields);
    if(keyArraySize == 4)
    {
        topicTypeName = DMM_DCPSTopic_getTopicTypeName(mainTopic);
    }/*else let it remain nil*/
    /* get the oid, its part of the keys for this instance */
    DK_ObjectAdmin_us_getObjectID(objectAdmin, &oid);
    objectHomeBridge.setDefaultTopicKeys(exception,
                                        userData,
                                        _this,
                                        DK_ObjectWriter_us_getTopicInfo(_this->objectWriter),
                                        ls_object,
                                        &oid,
                                        topicTypeName);

  /*  writer = DK_ObjectWriter_us_getWriter(_this->objectWriter);*/


    /* TODO dds416: this is temp code. we need to get a unique ID, somehow. The following code also sets the
     * key value array in the object admin
     */
  /*  DK_ObjectHomeAdmin_us_SetUniqueIdTEMP(_this, exception, keyFields, writer, */ /*dataSample,*/ /* objectAdmin, mainTopic);*/
  /*  DLRL_Exception_PROPAGATE(exception);*/


    /* Now we can register the instance to the writer, take note that this operation also updates the user data
     * of the registered instance to contain a reference to the created object admin.
     */





/* Step 2) We need to register an instance with the data writer so we can retrieve a handle. */
    /* get the writer, we need it to perform the register instance among other things
alot of code is turned off here due to missing implementation of dds416*/

    /* create a message and displace towards the data sample, we need the data sample to copy the key values into
     * these key values will be needed during the register instance
     */
/*    message = (void*)DK_DCPSUtility_ts_createMessageForDataWriter(writer, exception, &offset);
    DLRL_Exception_PROPAGATE(exception);
    dataSample = C_DISPLACE(message, offset);
*/




    /* Get the mainTopic and it's key fields. We need this to get the database fields with which we can assign values
     * in the correct places in the data sample
     */





 /*   DLRL_Exception_PROPAGATE(exception);
   */ /* now we can do the copy in with the data sample */
/*    objectHomeBridge.doCopyInForTopicOfObject(exception, userData, _this, _this->objectWriter, objectAdmin,
                                                    message, dataSample);
    DLRL_Exception_PROPAGATE(exception);
*/
    /* dds416: this is now disable because its done in the setUniqueIdTEMP operation. If this should ever be turned on
     * again depends on key/data seperation usage. We might no longer need it!
     * keyArray = DK_DCPSUtility_us_convertDataFieldsOfDataSampleIntoValueArray(exception, keyFields, dataSample, 0);
     * DLRL_Exception_PROPAGATE(exception);
     * DK_ObjectAdmin_us_setKeyValueArray(objectAdmin, keyArray, keyArraySize);
     */



/* step 4) actually insert the object into the DLRL administration */
/* takes over the new ref count!! */
    /* takes over the new ref count!! */
    DK_CacheAccessAdmin_us_registerObjectAdmin(access, exception, userData, objectAdmin);
    DLRL_Exception_PROPAGATE(exception);

/* step 5) Resolve any unresolved relations once the object is fully a part of DLRL administration */
    /* Maybe there are unresolved relations for this newly created object? This will never be the case in a cache access
     * which has no 'cloned' objects, in that case the unresolved elements list will always be empty.
     */
    DK_CacheAccessAdmin_us_resolveElements(access, exception, userData, _this, objectAdmin);
    DLRL_Exception_PROPAGATE(exception);

/* step 6) prepare to return the object */
    retVal = utilityBridge.localDuplicateLSValuetypeObject(userData, ls_object);
    if(!retVal)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to complete operation, out of resources");
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DK_CacheAccessAdmin_unlock(access);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

DLRL_LS_object
DK_ObjectHomeAdmin_ts_createUnregisteredObject(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAccessAdmin* access)
{
    DK_ObjectAdmin* objectAdmin = NULL;
    DLRL_LS_object ls_object = NULL;
    DLRL_LS_object retVal = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(access);

    DK_CacheAccessAdmin_lock(access);
    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED, "Unable to create unregistered object. This %s '%p' is"
                                " already deleted.", ENTITY_NAME, _this);
    }
    if(!DK_CacheAccessAdmin_us_isAlive(access))
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to create unregistered object. The "
                            "CacheAccess into which the newly created object is to be inserted is already deleted.");
    }
    if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)access) == DK_USAGE_READ_ONLY)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to create unregistered object. Object "
                    "creation is not allowed in a READ ONLY Cache Access. You can only create objects within a WRITE "
                    "ONLY or READ WRITE Cache Access.");
    }
    if(!_this->cache)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to create unregistered object. This %s '%p' "
                "is not yet registered with a Cache", ENTITY_NAME, _this);
    }
    if(_this->cache != DK_CacheAccessAdmin_us_getOwner(access))
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to create unregistered object. This %s '%p' "
                "belongs to a different Cache then the provided CacheAccess", ENTITY_NAME, _this);
    }
    if(DK_CacheAdmin_us_getPubSubState(_this->cache) != DK_PUB_SUB_STATE_ENABLED)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to create unregistered object. The Cache "
            "to which this %s '%p' belongs is not yet enabled for publication/subscribtion. Objects can only be created"
            " for object homes registered to an enabled Cache.", ENTITY_NAME, _this);
    }
    assert(_this->meta_representative);/*must exist if the pubsubstate is enabled...*/
    if(DMM_DLRLClass_getMapping(_this->meta_representative) != DMM_MAPPING_PREDEFINED)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "%s '%p': Unable to create object. The Object type "
            "is mapped using default mapping rules. Objects can only be created"
            " by this operation if they are mapped using predefined mapping rules.", ENTITY_NAME, _this);
    }

    objectAdmin = DK_ObjectAdmin_new(exception, userData, _this, DK_DCPSUtility_ts_getNilHandle(), NULL, 0,
                                        DK_OBJECT_STATE_OBJECT_VOID, DK_OBJECT_STATE_OBJECT_VOID, access, FALSE);
    DLRL_Exception_PROPAGATE(exception);

    ls_object = objectHomeBridge.createTypedObject(exception, userData, _this,
                                            DK_ObjectWriter_us_getTopicInfo(_this->objectWriter), NULL, objectAdmin);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectAdmin_us_setLSObject(objectAdmin, userData, ls_object);
    /* TODO creating collections here consistitues a problem when we have the possibility of unresolved collections
     * in the unresolved list in a cache access for this object. Currently this is not the case, as we dont use
     * the unresolved list, but in the future it may lead to problems. However it is currently unclear if unresolved
     * collections will even exist in a cache access scope. Analyze this later on, no time atm
     */
    DK_ObjectAdmin_us_createCollections(objectAdmin, exception, userData);
    DLRL_Exception_PROPAGATE(exception);
    /* takes over the new ref count!! */
    DK_CacheAccessAdmin_us_addUnregisteredObjectAdmin(access, exception, objectAdmin);
    DLRL_Exception_PROPAGATE(exception);

    retVal = utilityBridge.localDuplicateLSValuetypeObject(userData, ls_object);
    if(!retVal)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to complete operation, out of resources");
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DK_CacheAccessAdmin_unlock(access);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

void
DK_ObjectHomeAdmin_ts_registerObject(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin)
{
    DK_CacheAccessAdmin* access = NULL;
    u_instanceHandle handle;
    LOC_unsigned_long keyArraySize = 0;
    Coll_List* keyFields = NULL;
    void* keyArray = NULL;
    void* message = NULL;
    void* dataSample = NULL;
    c_long offset = 0;
    u_writer writer = NULL;
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(objectAdmin);

    handle = DK_DCPSUtility_ts_getNilHandle();

    access = DK_ObjectAdmin_us_getAccess(objectAdmin);
    if(access)
    {
        DK_CacheAccessAdmin_lockAll(access);
    } else
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to register unregistered object. The object "
                "provided is not located within a cache access.");
    }
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED, "Unable to register unregistered object. This %s '%p' is"
                                " already deleted.", ENTITY_NAME, _this);
    }
    if(!DK_ObjectAdmin_us_isAlive(objectAdmin))
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to register unregistered object. The target "
                            "unregistered object '%p' has already been deleted!", objectAdmin);
    }
    if(_this != DK_ObjectAdmin_us_getHome(objectAdmin))
    {/* the object and its home must belong together! */
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to register unregistered object. The target "
                "unregistered object '%p' doesnt belong to this %s '%p'", objectAdmin, ENTITY_NAME, _this);
    }
    if(!DK_CacheAccessAdmin_us_isAlive(access))
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable register unregistered object. "
                  "The DLRL Kernel Object Admin '%p' is located in an already deleted Cache Access.", ENTITY_NAME,
                      _this);
    }
    if(!_this->cache)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to register object. This %s '%p' "
                "is not yet registered with a Cache", ENTITY_NAME, _this);
    }
    if(DK_CacheAdmin_us_getPubSubState(_this->cache) != DK_PUB_SUB_STATE_ENABLED)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "Unable to register object. The Cache "
            "to which this %s '%p' belongs is not yet enabled for publication/subscribtion. Objects can only be "
            " registered for object homes registered to an enabled Cache.", ENTITY_NAME, _this);
    }
    assert(_this->meta_representative);/*must exist if the pubsubstate is enabled...*/
    if(DMM_DLRLClass_getMapping(_this->meta_representative) != DMM_MAPPING_PREDEFINED)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET, "%s '%p': Unable to register object. The Object type "
            "is mapped using default mapping rules. Objects can only be registered"
            " by this operation if they are mapped using predefined mapping rules.", ENTITY_NAME, _this);
    }
    /* dont make an exception of this check, would mess up unwinding... */
    if(!DK_ObjectAdmin_us_getIsRegistered(objectAdmin))
    {
        /* ensure that the dataSample pointer is filled as one of the first things... (checked to see if unwinding is  */
        /* needed) */

        /* create a data sample as target for the copy in         */
        writer = DK_ObjectWriter_us_getWriter(_this->objectWriter);
        message = (void*)DK_DCPSUtility_ts_createMessageForDataWriter(writer, exception, &offset);
        DLRL_Exception_PROPAGATE(exception);
        dataSample = C_DISPLACE(message, offset);

        /* now we can do the copy in with the data sample */
        objectHomeBridge.doCopyInForTopicOfObject(exception, userData, _this, _this->objectWriter, objectAdmin,
                                                        message, dataSample);
        DLRL_Exception_PROPAGATE(exception);

        /* do the register to get the instance handle */
        handle = DK_ObjectWriter_us_registerInstance(_this->objectWriter, exception, userData, objectAdmin, NULL);
        DLRL_Exception_PROPAGATE(exception);
        DK_ObjectAdmin_us_setHandle(objectAdmin, handle);

        keyFields = DMM_DCPSTopic_getKeyFields(DMM_DLRLClass_getMainTopic(_this->meta_representative));
        keyArraySize = Coll_List_getNrOfElements(keyFields);
        keyArray = DK_DCPSUtility_us_convertDataFieldsOfDataSampleIntoValueArray(exception, keyFields, dataSample, 0);
        DLRL_Exception_PROPAGATE(exception);
        DK_ObjectAdmin_us_setKeyValueArray(objectAdmin, keyArray, keyArraySize);

        /* transfer the object admin from the unregistered objects list to the normal objects list --> can throw an */
        /* already existing exception */
        assert(!DK_ObjectAdmin_us_getIsRegistered(objectAdmin));
        DK_CacheAccessAdmin_us_registerUnregisteredObjectAdmin(access, exception, userData, objectAdmin);
        DLRL_Exception_PROPAGATE(exception);
        assert(DK_ObjectAdmin_us_getIsRegistered(objectAdmin));
        DK_ObjectAdmin_us_setWriteState(objectAdmin, userData, DK_OBJECT_STATE_OBJECT_NEW);
        if(DK_CacheBase_us_getCacheUsage((DK_CacheBase*)access) != DK_USAGE_WRITE_ONLY)
        {
            DK_ObjectAdmin_us_setReadState(objectAdmin, DK_OBJECT_STATE_OBJECT_NOT_MODIFIED);
        }/* else let it remain void */
        DK_CacheAccessAdmin_us_resolveElements(access, exception, userData, _this, objectAdmin);
        DLRL_Exception_PROPAGATE(exception);
    }/* else no op */

    DLRL_Exception_EXIT(exception);
    /* the check for the data sample pointer ensures we cleared the first line of 'normal' exceptions */
    if(exception->exceptionID != DLRL_NO_EXCEPTION && dataSample)
    {
        DLRL_Exception tmpException;
        /* only needs to unwind if we have sample data */
        /* import, check the handle of the object... because if the exception occured due to the fact an object with the */
        /* identify already existed we reset the handle on the object to indicate this occurance..... */
        if(!u_instanceHandleIsNil(DK_ObjectAdmin_us_getHandle(objectAdmin)))
        {
            DLRL_Exception_init(&tmpException);
            DK_DCPSUtility_us_unregisterObjectFromWriterInstance(
                &tmpException,
                objectAdmin,
                DK_ObjectWriter_us_getWriter(_this->objectWriter));
            if(tmpException.exceptionID != DLRL_NO_EXCEPTION)
            {
                DLRL_REPORT(REPORT_ERROR, "Exception %s occured when attempting to unregister an object from a writer "
                    "instance\n%s", DLRL_Exception_exceptionIDToString(tmpException.exceptionID),
                    tmpException.exceptionMessage);
            }
            DK_ObjectAdmin_us_setHandle(objectAdmin, DK_DCPSUtility_ts_getNilHandle());
        }
        if(keyArray)
        {
            DK_ObjectAdmin_us_setKeyValueArray(objectAdmin, NULL, 0);
            DK_DCPSUtility_us_destroyValueArray(keyArray, keyArraySize);
        }
        if(DK_ObjectAdmin_us_getIsRegistered(objectAdmin))
        {
            DLRL_Exception_init(&tmpException);
            DK_CacheAccessAdmin_us_unregisterRegisteredObjectAdmin(access, exception, userData, objectAdmin);
            if(tmpException.exceptionID != DLRL_NO_EXCEPTION)
            {
                DLRL_REPORT(REPORT_ERROR, "Exception %s occured when attempting to transform an invalidly registered "
                "object back into an unregistered object\n%s",
                DLRL_Exception_exceptionIDToString(tmpException.exceptionID), tmpException.exceptionMessage);
            }
        }
        /* recovery from exceptions during the resolving of unresolved elements action is not possible. */
    }
    if(message)
    {
        assert(writer);
        result = u_entityAction(u_entity(writer), DK_DCPSUtility_us_freeMessage, message);
        if(result != U_RESULT_OK)
        {
           DLRL_Exception_transformResultToException(exception, result, "Unable to free message.");
        }
    }
    if(objectAdmin->access)
    {
        DK_CacheAccessAdmin_unlockAll(objectAdmin->access);
    }
    DLRL_INFO(INF_EXIT);
}

/**********************************************************************************************************************
******************************** DLRL Kernel Thread safety calls of the ObjectHomeAdmin *******************************
**********************************************************************************************************************/

void
DK_ObjectHomeAdmin_lockAdmin(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    os_mutexLock(&(_this->adminMutex));
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_unlockAdmin(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    os_mutexUnlock(&(_this->adminMutex));
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_lockUpdate(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    os_mutexLock(&(_this->updateMutex));
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_unlockUpdate(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    os_mutexUnlock(&(_this->updateMutex));
    DLRL_INFO(INF_EXIT);
}

/**********************************************************************************************************************
*********************** Kernel Thread unsafe calls of the ObjectHomeAdmin (Internal kernel API) ***********************
**********************************************************************************************************************/
void
DK_ObjectHomeAdmin_us_checkAlive(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_ObjectHomeAdmin_us_isAlive(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->alive;
}

/* relation index is only used if the entity is a object admin, in other cases it may be any value */
void
DK_ObjectHomeAdmin_us_registerUnresolvedElement(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHolder* holder,
    LOC_unsigned_long relationIndex)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(holder);
    assert(_this->objectReader);/* object reader must be present */

    DK_UnresolvedObjectsUtility_us_registerUnresolvedElement(&(_this->objectReader->unresolvedElements), exception,
                                                                     userData, holder, relationIndex);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_us_unregisterUnresolvedElement(
    DK_ObjectHomeAdmin* _this,
    void* userData,
    DK_ObjectHolder* holder)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(holder);
    assert(_this->objectReader);/* object reader must be present */

    DK_UnresolvedObjectsUtility_us_unregisterUnresolvedElement(&(_this->objectReader->unresolvedElements), userData,
                                                                                                                holder);
    DLRL_INFO(INF_EXIT);
}

/* caller must handle the collection, (eventually destroy & release it) */
DK_Collection*
DK_ObjectHomeAdmin_us_unregisterUnresolvedCollection(
    DK_ObjectHomeAdmin* _this,
    void* userData,
    void* ownerValues,
    LOC_unsigned_long valuesSize,
    LOC_unsigned_long collectionIndex)
{
    DK_Collection* collection = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert((valuesSize > 0 && ownerValues) || (valuesSize == 0 && !ownerValues));
    assert(_this->objectReader);/* object reader must be present */

    collection = DK_UnresolvedObjectsUtility_us_unregisterUnresolvedCollection(
        &(_this->objectReader->unresolvedElements),
        userData,
        ownerValues,
        valuesSize,
        collectionIndex);
    DLRL_INFO(INF_EXIT);
    return collection;
}

/* one owner may register multiple callbacks if wanted. no reason to forbid it. For each call to the register operation */
/* a call to either the unregister operation must be made or a callback must have been received */
/* returns the collection if already exists as an unresolved element, if it didnt yet exist its created and then */
/* added to the unresolved list and returns it. */
/* Caller may not release the pointer of the collection */
/* asumes the home is locked */
DK_Collection*
DK_ObjectHomeAdmin_us_registerUnresolvedCollection(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void* ownerValues,
    LOC_unsigned_long valuesSize,
    DMM_DLRLMultiRelation* metaRelation,
    LOC_unsigned_long collectionIndex)
{
    DK_Collection* collection = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert((valuesSize > 0 && ownerValues) || (valuesSize == 0 && !ownerValues));
    assert(metaRelation);
    assert(_this->objectReader);/* object reader must be present */

    /* the DK_UnresolvedObjectsUtility_us_registerUnresolvedCollection requires that the owning home is locked, which is */
    /* also a pre condition for this operation */
    collection = DK_UnresolvedObjectsUtility_us_registerUnresolvedCollection(&(_this->objectReader->unresolvedElements),
                                 exception, userData, ownerValues, valuesSize, metaRelation, collectionIndex, _this);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
    return collection;
}

LOC_long
DK_ObjectHomeAdmin_us_getRegistrationIndex(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->index;
}

LOC_boolean
DK_ObjectHomeAdmin_us_hasCacheAdmin(
    DK_ObjectHomeAdmin* _this)
{
    LOC_boolean hasCacheAdmin = FALSE;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(_this->cache)
    {
        hasCacheAdmin = TRUE;
    }
    DLRL_INFO(INF_EXIT);
    return hasCacheAdmin;
}

DLRL_LS_object
DK_ObjectHomeAdmin_us_getLSHome(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_home;
}

void*
DK_ObjectHomeAdmin_us_getUserData(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->userData;
}

DMM_DLRLClass*
DK_ObjectHomeAdmin_us_getMetaRepresentative(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->meta_representative;
}

/* may return null */
DK_ObjectReader*
DK_ObjectHomeAdmin_us_getObjectReader(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->objectReader;
}

/* caller must free set, but not the elements. Elements only valid during the encapsulation lock on the ObjectHome */
/* when calling this operation */
/* be aware there is an threadsafe variant for this operation as well!
 * This unsafe variant must be replaced by the thread safe variant, but the
 * java language binding is not ready for that yet, so some double maintainance
 * is needed until the java language binding is adapted.
 */
Coll_Set*
DK_ObjectHomeAdmin_us_getAllTopicNames(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception)
{
    Coll_Set* topicNames = NULL;
    LOC_long returnCode;
    Coll_Iter* iterator;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    if(!_this->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
            "Unable to retrieve topic information for %s '%s'. No meta information known for this home. "
            "You must register this home with a cache first and then register the cache.",
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }

    topicNames = Coll_Set_new(pointerIsLessThen, TRUE);
    if(!topicNames)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to retrieve topic information for %s '%s'. Allocation error for the list of topic names.",
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }

    iterator = Coll_Set_getFirstElement(DMM_DLRLClass_getTopics(_this->meta_representative));
    while(iterator)
    {
        DMM_DCPSTopic* aTopic = (DMM_DCPSTopic*)Coll_Iter_getObject(iterator);
        returnCode = Coll_Set_add(topicNames, DMM_DCPSTopic_getTopicName(aTopic));
        if(returnCode != COLL_OK)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to retrieve topic information for %s '%s'. "
                "Allocation error when adding the name of a topic %s to the list of topic names.",
                ENTITY_NAME, DLRL_VALID_NAME(_this->name), DLRL_VALID_NAME(DMM_DCPSTopic_getTopicName(aTopic)));
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION)
    {
        /* rollback */
        if(topicNames)
        {
            Coll_Iter* iterator = Coll_Set_getFirstElement(topicNames);
            while(iterator)
            {
                LOC_string aTopicName = (LOC_string)Coll_Iter_getObject(iterator);
                iterator = Coll_Iter_getNext(iterator);
                Coll_Set_remove(topicNames, (void*)aTopicName);
            }
            Coll_Set_delete(topicNames);
        }
        topicNames = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return topicNames;
}
/* be aware there is an unsafe variant for this operation as well!
 * This unsafe variant must be replaced by this thread safe variant, but the
 * java language binding is not ready for that yet, so some double maintainance
 * is needed until the java language binding is adapted
 */
void
DK_ObjectHomeAdmin_ts_getAllTopicNames(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    Coll_Iter* iterator;
    DMM_DCPSTopic* aTopic;
    Coll_Set* topics;
    LOC_unsigned_long count = 0;/* must init to 0*/

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    DK_ObjectHomeAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    if(!_this->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
            "Unable to retrieve topic information for %s '%s'. "
            "No meta information known for this home. You must "
            "register this home with a cache first and then "
            "register the cache.", ENTITY_NAME,
            DLRL_VALID_NAME(_this->name));
    }

    topics = DMM_DLRLClass_getTopics(_this->meta_representative);
    utilityBridge.createStringSeq(
        exception,
        userData,
        arg,
        Coll_Set_getNrOfElements(topics));
    DLRL_Exception_PROPAGATE(exception);

    iterator = Coll_Set_getFirstElement(topics);
    while(iterator)
    {
        aTopic = (DMM_DCPSTopic*)Coll_Iter_getObject(iterator);
        utilityBridge.addElementToStringSeq(
            exception,
            userData,
            *arg,
            count,
            (void*)DMM_DCPSTopic_getTopicName(aTopic));
        DLRL_Exception_PROPAGATE(exception);
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
}

/* caller must not free anything. Elements only valid during the encapsulation lock on the ObjectHome */
/* when calling this operation */
Coll_Set*
DK_ObjectHomeAdmin_us_getChildren(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->children);
}

LOC_string
DK_ObjectHomeAdmin_us_getName(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->name;
}


DLRL_LS_object
DK_ObjectHomeAdmin_ts_getLSDataReader(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const char* topicName)
{
    Coll_Iter* iterator;
    DLRL_LS_object retVal = NULL;
    DK_TopicInfo* topicInfo;
    DMM_DCPSTopic* metaTopic;
    DLRL_LS_object tmp;
    LOC_boolean found = FALSE;
    Coll_List* collectionReaders;
    DK_CollectionReader* collectionReader;

    DLRL_INFO_OBJECT(INF_ENTER);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    DK_ObjectHomeAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    if(!_this->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
            "Unable to retrieve the datareader for topicname %s of %s '%s'. "
            "No DCPS entities created yet for this home. You must register "
            "this home with a cache first and then register the cache.",
            DLRL_VALID_NAME(topicName), ENTITY_NAME,
            DLRL_VALID_NAME(_this->name));
    }
    /* we must have an object reader, otherwise we can just return null
     * as no object reader means no readers at all
     */
    if(_this->objectReader)
    {
        topicInfo = DK_ObjectReader_us_getTopicInfo(_this->objectReader);
        metaTopic = DK_TopicInfo_us_getMetaTopic(topicInfo);
        if(0 == strcmp(DMM_DCPSTopic_getTopicName(metaTopic), topicName))
        {
            /* may return null*/
            tmp = DK_ObjectReader_us_getLSReader(_this->objectReader);
            /* must return a copy */
            retVal = utilityBridge.localDuplicateLSInterfaceObject(
                userData,
                tmp);
            if(tmp && !retVal)
            {
                DLRL_Exception_THROW(
                    exception,
                    DLRL_OUT_OF_MEMORY,
                    "Unable to complete operation, out of resources");
            }
            found = TRUE;
        }
        if(!found)/*i.e., we havent found anything yet */
        {
            collectionReaders = DK_ObjectReader_us_getCollectionReaders(_this->objectReader);
            iterator = Coll_List_getFirstElement(collectionReaders);
            while(iterator && !found)
            {
                collectionReader = (DK_CollectionReader*)Coll_Iter_getObject(iterator);
                topicInfo = DK_CollectionReader_us_getTopicInfo(collectionReader);
                metaTopic = DK_TopicInfo_us_getMetaTopic(topicInfo);
                if(0 == strcmp(DMM_DCPSTopic_getTopicName(metaTopic), topicName))
                {
                    /* may return null*/
                    tmp = DK_CollectionReader_us_getLSReader(collectionReader);
                    /* must return a copy */
                    retVal = utilityBridge.localDuplicateLSInterfaceObject(
                        userData,
                        tmp);
                    if(tmp && !retVal)
                    {
                        DLRL_Exception_THROW(
                            exception,
                            DLRL_OUT_OF_MEMORY,
                            "Unable to complete operation, out of resources");
                    }
                    found = TRUE;
                }
                iterator = Coll_Iter_getNext(iterator);
            }
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}


DLRL_LS_object
DK_ObjectHomeAdmin_ts_getLSDataWriter(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const char* topicName)
{
    Coll_Iter* iterator;
    DLRL_LS_object retVal = NULL;
    DK_TopicInfo* topicInfo;
    DMM_DCPSTopic* metaTopic;
    DLRL_LS_object tmp;
    LOC_boolean found = FALSE;
    Coll_List* collectionWriters;
    DK_CollectionWriter* collectionWriter;

    DLRL_INFO_OBJECT(INF_ENTER);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    DK_ObjectHomeAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    if(!_this->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
            "Unable to retrieve the datareader for topicname %s of %s '%s'. "
            "No DCPS entities created yet for this home. You must register "
            "this home with a cache first and then register the cache.",
            DLRL_VALID_NAME(topicName), ENTITY_NAME,
            DLRL_VALID_NAME(_this->name));
    }
    /* we must have an object reader, otherwise we can just return null
     * as no object reader means no readers at all
     */
    if(_this->objectWriter)
    {
        topicInfo = DK_ObjectWriter_us_getTopicInfo(_this->objectWriter);
        metaTopic = DK_TopicInfo_us_getMetaTopic(topicInfo);
        if(0 == strcmp(DMM_DCPSTopic_getTopicName(metaTopic), topicName))
        {
            /* may return null*/
            tmp = DK_ObjectWriter_us_getLSWriter(_this->objectWriter);
            /* must return a copy */
            retVal = utilityBridge.localDuplicateLSInterfaceObject(
                userData,
                tmp);
            if(tmp && !retVal)
            {
                DLRL_Exception_THROW(
                    exception,
                    DLRL_OUT_OF_MEMORY,
                    "Unable to complete operation, out of resources");
            }
            found = TRUE;
        }
        if(!found)/*i.e., we havent found anything yet */
        {
            collectionWriters = DK_ObjectWriter_us_getCollectionWriters(_this->objectWriter);
            iterator = Coll_List_getFirstElement(collectionWriters);
            while(iterator && !found)
            {
                collectionWriter = (DK_CollectionWriter*)Coll_Iter_getObject(iterator);
                topicInfo = DK_CollectionWriter_us_getTopicInfo(collectionWriter);
                metaTopic = DK_TopicInfo_us_getMetaTopic(topicInfo);
                if(0 == strcmp(DMM_DCPSTopic_getTopicName(metaTopic), topicName))
                {
                    /* may return null*/
                    tmp = DK_CollectionWriter_us_getLSWriter(collectionWriter);
                    /* must return a copy */
                    retVal = utilityBridge.localDuplicateLSInterfaceObject(
                        userData,
                        tmp);
                    if(tmp && !retVal)
                    {
                        DLRL_Exception_THROW(
                            exception,
                            DLRL_OUT_OF_MEMORY,
                            "Unable to complete operation, out of resources");
                    }
                    found = TRUE;
                }
                iterator = Coll_Iter_getNext(iterator);
            }
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

DLRL_LS_object
DK_ObjectHomeAdmin_ts_getLSTopic(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const char* topicName)
{
    Coll_Iter* iterator;
    DLRL_LS_object retVal = NULL;
    DLRL_LS_object tmp;
    DK_TopicInfo* topicInfo;
    DMM_DCPSTopic* metaTopic;

    DLRL_INFO_OBJECT(INF_ENTER);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    DK_ObjectHomeAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    if(!_this->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
            "Unable to retrieve the datareader for topicname %s of %s '%s'. "
            "No DCPS entities created yet for this home. You must register "
            "this home with a cache first and then register the cache.",
            DLRL_VALID_NAME(topicName), ENTITY_NAME,
            DLRL_VALID_NAME(_this->name));
    }
    iterator = Coll_List_getFirstElement(&(_this->topicInfos));
    while(iterator && !retVal)
    {
        topicInfo = (DK_TopicInfo*)Coll_Iter_getObject(iterator);
        metaTopic = DK_TopicInfo_us_getMetaTopic(topicInfo);
        if(0 == strcmp(DMM_DCPSTopic_getTopicName(metaTopic), topicName))
        {
            /* may return null*/
            tmp = DK_TopicInfo_us_getLSTopic(topicInfo);
            /* must return a copy */
            retVal = utilityBridge.localDuplicateLSInterfaceObject(
                userData,
                tmp);
            if(tmp && !retVal)
            {
                DLRL_Exception_THROW(
                    exception,
                    DLRL_OUT_OF_MEMORY,
                    "Unable to complete operation, out of resources");
            }
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

LOC_string
DK_ObjectHomeAdmin_ts_getTopicName(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    const char * attributeName)
{
    LOC_string topicName = NULL;
    LOC_string retVal = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    DK_ObjectHomeAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    topicName = DK_ObjectHomeAdmin_us_getTopicName(
        _this,
        exception,
        attributeName);
    DLRL_Exception_PROPAGATE(exception);
    /* must return a copy */
    DLRL_STRDUP(retVal, topicName, exception, "Out of resources");

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

LOC_string
DK_ObjectHomeAdmin_ts_getName(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception)
{
    LOC_string name = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    DK_ObjectHomeAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_STRDUP(name, _this->name, exception, "Out of resources");

    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return name;
}


/* should actually be just a part of the thread safe variant, but no
 * time to adapt java language binding at the moment...
 */
LOC_string
DK_ObjectHomeAdmin_us_getTopicName(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    const char* attributeName)
{
    LOC_string topicName = NULL;
    LOC_boolean found = FALSE;
    Coll_Set* attributes = NULL;
    Coll_Iter* iterator = NULL;
    DMM_DCPSTopic* dcpsTopic;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(attributeName);

    if(!_this->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
            "Unable to retrieve the topic name for attribute %s of %s '%s'. No meta information known for this home. "
            "You must register this home with a cache first and then register the cache.",
            DLRL_VALID_NAME(attributeName), ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    /* maybe the attribute name specifies a normal attribute */
    attributes = DMM_DLRLClass_getAttributes(_this->meta_representative);
    iterator = Coll_Set_getFirstElement(attributes);
    while(iterator && !found)
    {
        DMM_DLRLAttribute* anAttribute = (DMM_DLRLAttribute*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(attributeName, DMM_DLRLAttribute_getName(anAttribute)))
        {
            DMM_DCPSTopic* dcpsTopic = DMM_DLRLAttribute_getTopic(anAttribute);
            topicName = DMM_DCPSTopic_getTopicName(dcpsTopic);
            found = TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    if(!found)
    {
        DMM_DLRLRelation* aRelation = DMM_DLRLClass_findRelationByName(_this->meta_representative, attributeName);
        if(aRelation && DMM_DLRLRelation_getClassID(aRelation) == DMM_DLRL_RELATION_CLASS)
        {
            DMM_DCPSTopic* dcpsTopic = DMM_DLRLRelation_getOwnerTopic(aRelation);
            topicName = DMM_DCPSTopic_getTopicName(dcpsTopic);
            found = TRUE;
        } else if(aRelation)
        {
            assert(DMM_DLRLRelation_getClassID(aRelation) == DMM_DLRL_MULTI_RELATION_CLASS);
            dcpsTopic = DMM_DLRLMultiRelation_getRelationTopic((DMM_DLRLMultiRelation*)aRelation);
            topicName = DMM_DCPSTopic_getTopicName(dcpsTopic);
            found = TRUE;
        }
    }
    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION)
    {
        topicName = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return topicName;
}

void
DK_ObjectHomeAdmin_us_setCacheAdmin(
    DK_ObjectHomeAdmin* _this,
    DK_CacheAdmin* cache)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(cache);

    if(_this->cache)
    {
        DK_Entity_ts_release((DK_Entity*)(_this->cache));
    }
    _this->cache = (DK_CacheAdmin*)DK_Entity_ts_duplicate((DK_Entity*)cache);
    DLRL_INFO(INF_EXIT);
}

/* maybe only be called once to set the registeration index from -1 to another value (no limit when setting from -1 to -1) */
void
DK_ObjectHomeAdmin_us_setRegistrationIndex(
    DK_ObjectHomeAdmin* _this,
    LOC_long registrationIndex)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->index == -1);

    _this->index = registrationIndex;
    DLRL_INFO(INF_EXIT);
}

DK_ObjectWriter*
DK_ObjectHomeAdmin_us_getObjectWriter(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->objectWriter;
}

Coll_Set*
DK_ObjectHomeAdmin_us_getListeners(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->listeners);
}


Coll_Set*
DK_ObjectHomeAdmin_us_getSelections(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->selections);
}

/* may return null */
Coll_List*
DK_ObjectHomeAdmin_us_getNewObjects(
    DK_ObjectHomeAdmin* _this)
{
    Coll_List* returnValue = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(_this->objectReader)
    {
        returnValue = &(_this->objectReader->newSamples);
    }

    DLRL_INFO(INF_EXIT);
    return returnValue;
}

/* may return null */
Coll_List*
DK_ObjectHomeAdmin_us_getModifiedObjects(
    DK_ObjectHomeAdmin* _this)
{
    Coll_List* returnValue = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(_this->objectReader)
    {
        returnValue = &(_this->objectReader->modifiedSamples);
    }
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

/* may return null */
Coll_List*
DK_ObjectHomeAdmin_us_getDeletedObjects(
    DK_ObjectHomeAdmin* _this)
{
    Coll_List* returnValue = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(_this->objectReader)
    {
        returnValue = &(_this->objectReader->deletedSamples);
    }
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

void
DK_ObjectHomeAdmin_us_markObjectAsModified(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* modifiedObject)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(modifiedObject);
    assert(exception);

    DK_ObjectReader_us_markObjectAsModified(_this->objectReader, exception, modifiedObject);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* precondition: Owning CacheAdmin is locked */
/* assumes all related homes are locked (parent/child homes) */
void
DK_ObjectHomeAdmin_us_setRelations(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception)
{
    DK_ObjectHomeAdmin* parentHome;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    if(!_this->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to resolve parent-child relations for %s '%s'. "
            "No meta information known for this home. "
            "You must register this home with a cache first and then register the cache. "
            "Ensure that the meta information is defined correctly for this ObjectHome!",
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    if(DMM_DLRLClass_getParentName(_this->meta_representative))
    {
        /* cache is already locked, operation wont lock any homes */
        parentHome = DK_CacheAdmin_us_findHomeByName(_this->cache,
                                                    DMM_DLRLClass_getParentName(_this->meta_representative));
        if(!parentHome)
        {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                "Unable to find parent with name %s for %s '%s'",
                DLRL_VALID_NAME(DMM_DLRLClass_getParentName(_this->meta_representative)), ENTITY_NAME,
                DLRL_VALID_NAME(_this->name));
        }
        if(parentHome == _this)
        {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
                "A home cannot be it's own parent. Bad definition occured within %s '%s'",
                DLRL_VALID_NAME(DMM_DLRLClass_getParentName(_this->meta_representative)), ENTITY_NAME,
                DLRL_VALID_NAME(_this->name));

        }
        _this->parent = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)parentHome);
        DK_ObjectHomeAdmin_us_addChild(parentHome, exception, _this);
        DLRL_Exception_PROPAGATE(exception);
    }/* else do nothing */

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/**********************************************************************************************************************
****************************** Kernel Thread unsafe internal calls of the ObjectHomeAdmin *****************************
**********************************************************************************************************************/
void
DK_ObjectHomeAdmin_us_destroy(
    DK_Entity* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if(_this)
    {
        os_mutexDestroy(&(((DK_ObjectHomeAdmin*)_this)->updateMutex));
        os_mutexDestroy(&(((DK_ObjectHomeAdmin*)_this)->adminMutex));
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_ObjectHomeAdmin*)_this);
    }

    DLRL_INFO(INF_EXIT);
}

/* needs locks on its owning home + related homes, meta information of all homes must be in tact! */
void
DK_ObjectHomeAdmin_us_deleteAllObjectAdmins(
    DK_ObjectHomeAdmin* _this,
    void* userData)
{
    DLRL_Exception exception;
    DK_ObjectArrayHolder holder;
    LOC_unsigned_long count = 0;
    DK_ObjectAdmin* anObjectAdmin = NULL;
    u_instanceHandle handle;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    /* first delete all object admins */
    DLRL_Exception_init(&exception);
    holder.objectArray = NULL;
    holder.size = 0;
    holder.maxSize = 0;
    /* object array of the holder may remain null after this operation! */
    DK_ObjectHomeAdmin_us_getAllObjects(_this, &exception, &holder);
    if(exception.exceptionID != DLRL_NO_EXCEPTION)
    {
        DLRL_REPORT(REPORT_ERROR, "Unable to clear object admins attached to %s %s. Exception: %s :: %s", ENTITY_NAME,
            _this->name, DLRL_Exception_exceptionIDToString(exception.exceptionID), exception.exceptionMessage);
    } else
    {
        LOC_boolean noException = TRUE;
        for(count = 0; count < holder.size; count++)
        {
            anObjectAdmin = holder.objectArray[count];
            /* reset instance handle, then delete the object admin */
            handle = DK_ObjectAdmin_us_getHandle(anObjectAdmin);
            DK_DCPSUtility_us_setUserDataBasedOnHandle(&exception, handle, NULL);/* ignore return value */

            /* Exceptions here are most likely caused by the DCPS releasing resources because of
             * a termination event. In that case, the DLRL should not terminate abruptly, but in
             * stead should try to continue its deletion task, ignoring any similar exceptions so
             * that it may terminate gracefuly.
             * Because of that, we will log the first exception but ignore all further exceptions.
             */
            if(noException && exception.exceptionID != DLRL_NO_EXCEPTION)
            {
                DLRL_REPORT(REPORT_ERROR, "Unable to clear object admins attached to %s %s. Exception: %s :: %s",
                        ENTITY_NAME, _this->name, DLRL_Exception_exceptionIDToString(exception.exceptionID),
                        exception.exceptionMessage);
                noException = FALSE;
            }
            DK_ObjectAdmin_us_delete(anObjectAdmin, userData);/* needs locks on its owning home + related homes */
            DK_Entity_ts_release((DK_Entity*)anObjectAdmin);
        }
    }
    if(holder.objectArray)
    {
        os_free(holder.objectArray);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_us_delete(
    DK_ObjectHomeAdmin* _this,
    void* userData)
{
    Coll_Iter* iterator = NULL;
    LOC_unsigned_long size = 0;
    LOC_unsigned_long count = 0;
    DLRL_Exception exception;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* userData may be null */
    DLRL_Exception_init(&exception);

    if(_this->alive)
    {
        /* delete selections created through the dlrl */
        iterator = Coll_Set_getFirstElement(&(_this->selections));
        while(iterator)
        {
            void* aSelection = Coll_Iter_getObject(iterator);
            DK_SelectionAdmin_us_delete((DK_SelectionAdmin*)aSelection, userData);
            iterator = Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->selections), aSelection);
            DK_Entity_ts_release((DK_Entity*)aSelection);
        }
        /* delete the listeners */
        iterator = Coll_Set_getFirstElement(&(_this->listeners));
        while(iterator)
        {
            void* aListener = Coll_Iter_getObject(iterator);
            iterator = Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->listeners), aListener);
            utilityBridge.releaseLSInterfaceObject(userData, aListener);
        }
        /* destroy dlrl created parent / child relations */
        iterator = Coll_Set_getFirstElement(&(_this->children));
        while(iterator)
        {
            void* aChild = Coll_Iter_getObject(iterator);
            /* needed, but also prevents issues with the mutex later */
            /* (no one has a ref to this object home so mutex cant be relocked! */
            DK_ObjectHomeAdmin_us_removeParent((DK_ObjectHomeAdmin*)aChild);
            iterator = Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->children), aChild);
            DK_Entity_ts_release((DK_Entity*)aChild);
        }
        if(_this->relatedHomes)
        {
            size = Coll_List_getNrOfElements(DMM_DLRLClass_getRelations(_this->meta_representative));
            for(count = 0; count < size; count++)
            {
                if(_this->relatedHomes[count])
                {
                    DK_Entity_ts_release((DK_Entity*)_this->relatedHomes[count]);
                }
            }
            os_free(_this->relatedHomes);
        }
        if(_this->objectReader)
        {
            DK_ObjectReader_us_delete(_this->objectReader, userData);
            DK_Entity_ts_release((DK_Entity*)_this->objectReader);
            _this->objectReader = NULL;
        }
        if(_this->objectWriter)
        {
            DK_ObjectWriter_us_delete(_this->objectWriter, userData);
            DK_Entity_ts_release((DK_Entity*)_this->objectWriter);
            _this->objectWriter = NULL;
        }
        /* We need to severe the link between the database of OpenSplice and
         * the meta data of DLRL. Within the DLRL meta data a link is maintained
         * to meta data residing in the database. We need to perform a
         * c_free action on that link, but this needs to be done within the
         * scope of a u_entityAction to ensure the database is still there when
         * the c_free is performed. We can use any u_entity as starting point of
         * the e_entityAction, in this specific case we use the u_topic, but it
         * could be any other u_entity as well.
         */
        if(Coll_List_getNrOfElements(&(_this->topicInfos)) > 0 && _this->meta_representative)
        {
            DK_TopicInfo* topicInfo;
            u_result result;

            topicInfo = (DK_TopicInfo*)Coll_List_getObject(&(_this->topicInfos), 0);
            assert(topicInfo);

            result = u_entityAction(
                (u_entity)DK_TopicInfo_us_getTopic(topicInfo),
                DK_DCPSUtility_us_freeDatabaseFields,
                (void*)_this->meta_representative);
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(&exception, result, "Unable to free database meta data fields.");
               DLRL_REPORT(REPORT_ERROR, "Exception %s occured when attempting to free database meta data fields.\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID), exception.exceptionMessage);
               /*reinit the exception, we may use it later on and we dont want wierd ass errors because someone forgets this
                * step*/
               DLRL_Exception_init(&exception);
            }

        }
        while(Coll_List_getNrOfElements(&(_this->topicInfos)) > 0)
        {
            DK_TopicInfo* topicInfo;

            topicInfo = (DK_TopicInfo*)Coll_List_popBack(&(_this->topicInfos));
            DK_TopicInfo_us_delete(topicInfo, userData);
            DK_Entity_ts_release((DK_Entity*)topicInfo);
        }

        /* delete meta model after topic holders, as the string stored in one of the */
        /* DCPSTopics is used as a key in the map of topic holders. */
        /* also delete after the object admins, as they use the meta representative */
        if(_this->meta_representative)
        {
            /* dont forget to release the home user data first! */
            void* metaUserData = DMM_DLRLClass_getUserData(_this->meta_representative);
            if(metaUserData)
            {
                DK_Entity_ts_release((DK_Entity*)metaUserData);
            }
            DMM_DLRLClass_delete(_this->meta_representative);
            _this->meta_representative = NULL;
        }
        if(_this->ls_home)
        {
            LOC_boolean isRegistered = FALSE;
            if(_this->cache)
            {
                isRegistered = TRUE;
            }
            objectHomeBridge.unregisterAdminWithLSHome(userData, _this->ls_home, isRegistered);
            _this->ls_home = NULL;
        }
        if(_this->name)
        {
            os_free(_this->name);
            _this->name = NULL;
        }
        if(_this->filter)
        {
            /* TODO ID: 136 */
        }
        if(_this->cache)
        {
            DK_Entity_ts_release((DK_Entity*)_this->cache);
            _this->cache = NULL;
        }
        if(_this->userData)
        {
            objectHomeBridge.deleteUserData(userData, _this->userData);
            _this->userData = NULL;
        }
        /*  its not allowed to reset the registeration index when deleting the object home */
        /* other entities might have references to it and require this index to perform locking correctly */
        /* see memo regarding locking strategy for more information. _this->index = -1; */
        _this->alive = FALSE;
    }
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_ObjectHomeAdmin_us_getAutoDeref(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->autoDeref;
}

void
DK_ObjectHomeAdmin_us_addChild(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* child)
{
    LOC_long returnCode;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(child);

    returnCode = Coll_Set_add(&(_this->children), (void *)DK_Entity_ts_duplicate((DK_Entity*)child));
    if(returnCode != COLL_OK)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to add %s '%s' as a child of %s '%s'. "
            "Allocation error when adding the child entity to the set of children.", ENTITY_NAME,
            DLRL_VALID_NAME(child->name), ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    DLRL_Exception_EXIT(exception);
    if(exception->exceptionID != DLRL_NO_EXCEPTION)
    {
        DK_Entity_ts_release((DK_Entity*)child);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_us_removeParent(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(_this->parent)
    {
        DK_Entity_ts_release((DK_Entity*)_this->parent);
        _this->parent = NULL;
    }

    DLRL_INFO(INF_EXIT);
}

/* precondition: cache must be locked (administrative) */
void
DK_ObjectHomeAdmin_us_createDCPSEntities(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DMM_DCPSTopic* mainMetaTopic = NULL;
    DK_TopicInfo* mainTopicInfo = NULL;
    Coll_Iter* iterator = NULL;
    Coll_List* multiRelations = NULL;
    DK_Usage usage;
    LOC_long returnCode = COLL_OK;
    DK_TopicInfo* topicInfo;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    /* TODO ID: 29 */
    if(!_this->meta_representative)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to create neccesary DCPS entities for %s '%s'. No meta information known for this home. "
            "You must register this home with a cache first and then register the cache. "
            "Ensure that the meta information is defined correctly for this ObjectHome!",
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    /* main topic (place and extension topics ignored)*/
    mainMetaTopic = DMM_DLRLClass_getMainTopic(_this->meta_representative);
    if(!mainMetaTopic)
    {
        DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
            "Unable to create the DCPS entities for the main topic of %s '%s'. "
            "No meta information describing the main topic known for this home. "
            "You must register this home with a cache first and then register the cache. "
            "Ensure that the meta information is defined correctly for this ObjectHome!",
            ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }

    /* main topic */
    mainTopicInfo = DK_ObjectHomeAdmin_us_createTopicInfo(_this, exception, userData, mainMetaTopic, TRUE);
    DLRL_Exception_PROPAGATE(exception);
    returnCode = Coll_List_pushBack(&(_this->topicInfos), (void*)mainTopicInfo);
    if (returnCode != COLL_OK)
    {
        DK_TopicInfo_us_delete(mainTopicInfo, userData);
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
            "Unable to add topic info to list of topic infos of %s '%s'", ENTITY_NAME, DLRL_VALID_NAME(_this->name));
    }
    usage = DK_CacheBase_us_getCacheUsage((DK_CacheBase*)_this->cache);
    switch (usage)
    {
    case DK_USAGE_READ_ONLY:
        DK_ObjectHomeAdmin_us_createObjectReader(_this, exception, userData, mainTopicInfo);
        DLRL_Exception_PROPAGATE(exception);
        break;
    case DK_USAGE_WRITE_ONLY:
        DK_ObjectHomeAdmin_us_createObjectWriter(_this, exception, userData, mainTopicInfo);
        DLRL_Exception_PROPAGATE(exception);
        break;
    case DK_USAGE_READ_WRITE:
        DK_ObjectHomeAdmin_us_createObjectReader(_this, exception, userData, mainTopicInfo);
        DLRL_Exception_PROPAGATE(exception);
        DK_ObjectHomeAdmin_us_createObjectWriter(_this, exception, userData, mainTopicInfo);
        DLRL_Exception_PROPAGATE(exception);
        break;
    default :
        /* may never happen */
        assert(FALSE);
        break;
    }

    /* multi relations */
    multiRelations = DMM_DLRLClass_getMultiRelations(_this->meta_representative);
    iterator = Coll_List_getFirstElement(multiRelations);
    while(iterator)
    {
        DMM_DLRLMultiRelation* aMultiRelation = (DMM_DLRLMultiRelation*)Coll_Iter_getObject(iterator);
        DMM_DCPSTopic* aMultiPlaceTopic = DMM_DLRLMultiRelation_getRelationTopic(aMultiRelation);
        if(!aMultiPlaceTopic)
        {
            DLRL_Exception_THROW(exception, DLRL_BAD_HOME_DEFINITION,
              "Unable to resolve meta information for %s '%s'. A multi relation was not mapped to a seperate topic.",
              "DLRL Kernel ObjectHomeAdmin", DLRL_VALID_NAME(_this->name));
        }
        topicInfo = DK_ObjectHomeAdmin_us_createTopicInfo(_this, exception, userData, aMultiPlaceTopic, FALSE);
        DLRL_Exception_PROPAGATE(exception);
        /* dont duplicate the topicinfo, we will use the ref count from the creation */
        returnCode = Coll_List_pushBack(&(_this->topicInfos), (void*)topicInfo);
        if (returnCode != COLL_OK)
        {
            DK_TopicInfo_us_delete(topicInfo, userData);
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
               "Unable to add topic info to list of topic infos of %s '%s'", ENTITY_NAME, DLRL_VALID_NAME(_this->name));
        }
        switch (usage)
        {
        case DK_USAGE_READ_ONLY:
            DK_ObjectReader_us_createCollectionReader(_this->objectReader, exception, userData, topicInfo,
                                                            aMultiRelation);
            DLRL_Exception_PROPAGATE(exception);
            break;
        case DK_USAGE_WRITE_ONLY:
            DK_ObjectWriter_us_createCollectionWriter(_this->objectWriter, exception, userData, topicInfo);
            DLRL_Exception_PROPAGATE(exception);
            break;
        case DK_USAGE_READ_WRITE:
            DK_ObjectReader_us_createCollectionReader(_this->objectReader, exception, userData, topicInfo,
                                                            aMultiRelation);
            DLRL_Exception_PROPAGATE(exception);
            DK_ObjectWriter_us_createCollectionWriter(_this->objectWriter, exception, userData, topicInfo);
            DLRL_Exception_PROPAGATE(exception);
            break;
        default :
            /* may never happen */
            assert(FALSE);
            break;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

DK_TopicInfo*
DK_ObjectHomeAdmin_us_createTopicInfo(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DMM_DCPSTopic* metaTopic,
    LOC_boolean isMainTopic)
{
    /* return value */
    DK_TopicInfo* topicInfo = NULL;
    u_topic topic = NULL;
    void* topicUserData = NULL;
    void* ls_topic = NULL;
    LOC_char* theTopicName = NULL;
    LOC_char* theTopicType = NULL;
    u_result result;
    DLRL_Exception exception2;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(metaTopic);
    /* userData may be null */

    DLRL_Exception_init(&exception2);

    theTopicName = (LOC_char*)DMM_DCPSTopic_getTopicName(metaTopic);
    theTopicType = (LOC_char*)DMM_DCPSTopic_getTopicTypeName(metaTopic);

    /* Register the required datatype for the topic. */
    dcpsUtilityBridge.registerType(exception, userData, _this, _this->cache, theTopicName, theTopicType);
    DLRL_Exception_PROPAGATE(exception);

    /* create the topic, the user layer topic returned here is a proxy owned by
     * DLRL!
     */
    topic = dcpsUtilityBridge.createTopic(
        exception,
        userData,
        _this,
        theTopicName,
        theTopicType,
        &topicUserData,
        &ls_topic,
        isMainTopic);
    DLRL_Exception_PROPAGATE(exception);
    assert(topic);

    /* create the topic info, the topic info becomes the owner of the user layer topic! */
    topicInfo = DK_TopicInfo_new(exception, topic, (DLRL_LS_object)ls_topic, _this, metaTopic, topicUserData);
    DLRL_Exception_PROPAGATE(exception);
    assert(topicInfo);

    DLRL_Exception_EXIT(exception);
    if(!topicInfo && topic)
    {
        result = u_entityFree(u_entity(topic));
        if(result != U_RESULT_OK)
        {
           DLRL_Exception_transformResultToException(
               &exception2,
               result,
               "Unable to free the user layer topic!");
           DLRL_REPORT(
               REPORT_ERROR,
               "Exception %s occured when attempting to delete the DCPS "
                    "user layer topic\n%s",
                DLRL_Exception_exceptionIDToString(exception2.exceptionID),
               exception2.exceptionMessage);
           DLRL_Exception_init(&exception2);
        }
    }
    DLRL_INFO(INF_EXIT);
    return topicInfo;
}

void
DK_ObjectHomeAdmin_us_createObjectReader(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo)
{
    u_reader reader = NULL;
    u_reader queryReader = NULL;
    DLRL_LS_object ls_reader = NULL;
    u_result result;
    DLRL_Exception exception2;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(topicInfo);
    /* userData may be null */

    DLRL_Exception_init(&exception2);

    reader = dcpsUtilityBridge.createDataReader(exception, userData, topicInfo, &ls_reader);
    DLRL_Exception_PROPAGATE(exception);
    assert(reader);
    queryReader = DK_DCPSUtility_us_createTakeDisposedNotNewInstanceReader(exception, reader);
    DLRL_Exception_PROPAGATE(exception);
    DK_DCPSUtility_us_resolveDCPSDatabaseTopicInfo(topicInfo, reader, exception);
    DLRL_Exception_PROPAGATE(exception);
    _this->objectReader = DK_ObjectReader_new(exception, reader, queryReader, ls_reader, topicInfo);
    DLRL_Exception_PROPAGATE(exception);
    assert(_this->objectReader);

    DLRL_Exception_EXIT(exception);
    if(!_this->objectReader)
    {
        if(reader)
        {
            result = u_entityFree(u_entity(reader));
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(
                   &exception2,
                   result,
                   "Unable to free the user layer reader!");
               DLRL_REPORT(
                   REPORT_ERROR,
                   "Exception %s occured when attempting to delete the DCPS "
                        "user layer datareader\n%s",
                    DLRL_Exception_exceptionIDToString(exception2.exceptionID),
                   exception2.exceptionMessage);
               DLRL_Exception_init(&exception2);
            }
        }
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_us_createObjectWriter(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_TopicInfo* topicInfo)
{
    u_writer writer = NULL;
    DLRL_LS_object ls_writer = NULL;
    u_result result;
    DLRL_Exception exception2;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(topicInfo);
    /* userData may be null */

    DLRL_Exception_init(&exception2);

    writer = dcpsUtilityBridge.createDataWriter(exception, userData, topicInfo, &ls_writer);
    DLRL_Exception_PROPAGATE(exception);
    assert(writer);
    _this->objectWriter = DK_ObjectWriter_new(exception, writer, ls_writer, topicInfo);
    DLRL_Exception_PROPAGATE(exception);
    assert(_this->objectWriter);

    DLRL_Exception_EXIT(exception);
    if(!_this->objectWriter && writer)
    {
        result = u_entityFree(u_entity(writer));
        if(result != U_RESULT_OK)
        {
           DLRL_Exception_transformResultToException(
               &exception2,
               result,
               "Unable to free the user layer writer!");
           DLRL_REPORT(
               REPORT_ERROR,
               "Exception %s occured when attempting to delete the DCPS "
                    "user layer dataeriter\n%s",
                DLRL_Exception_exceptionIDToString(exception2.exceptionID),
               exception2.exceptionMessage);
           DLRL_Exception_init(&exception2);
        }
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_us_loadMetamodel(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    objectHomeBridge.loadMetamodel(exception, _this, userData);
    DLRL_Exception_PROPAGATE(exception);
    if(!_this->meta_representative)
    {
        DLRL_Exception_THROW(exception,DLRL_BAD_HOME_DEFINITION,"Meta information insertion failed! No type specified");
    }

    DMM_DLRLClass_calculateNrOfMandatorySingleRelations(_this->meta_representative);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* requires a lock of the update mutex of the home */
void
DK_ObjectHomeAdmin_us_triggerSelectionListeners(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    Coll_Iter* iterator;
    DK_SelectionAdmin* selection;
    DLRL_INFO_OBJECT(INF_ENTER);

    iterator = Coll_Set_getFirstElement(&(_this->selections));
    while(iterator)
    {
        selection = (DK_SelectionAdmin*)Coll_Iter_getObject(iterator);
        if(DK_SelectionAdmin_us_getAutoRefresh(selection))
        {
            DK_SelectionAdmin_us_triggerListener(selection, exception, userData);
            DLRL_Exception_PROPAGATE(exception);
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);

    DLRL_INFO(INF_EXIT);
}

/* requires a lock of the update mutex of the home */
void
DK_ObjectHomeAdmin_us_triggerListeners(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    if(((Coll_List_getNrOfElements(DK_ObjectReader_getNewObjects(_this->objectReader)) > 0) ||
            (Coll_List_getNrOfElements(DK_ObjectReader_getModifiedObjects(_this->objectReader)) > 0) ||
            (Coll_List_getNrOfElements(DK_ObjectReader_getDeletedObjects(_this->objectReader)) > 0)) &&
            (Coll_Set_getNrOfElements(&(_this->listeners)) || _this->parent) )
    {
        /* continue if we have received updates and we have listeners, or we have a parent(which might have listeners)*/

        objectHomeBridge.triggerListeners(exception, userData, _this,
                                                DK_ObjectReader_getNewObjects(_this->objectReader),
                                                DK_ObjectReader_getModifiedObjects(_this->objectReader),
                                                DK_ObjectReader_getDeletedObjects(_this->objectReader));
        DLRL_Exception_PROPAGATE(exception);

    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* requires update & admin locks on owner home and related homes! */
void
DK_ObjectHomeAdmin_us_resetObjectModificationInformation(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    Coll_Iter* iterator;
    DK_SelectionAdmin* selection;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_ObjectReader_us_resetObjectModificationInformation(_this->objectReader, exception, userData);
    DLRL_Exception_PROPAGATE(exception);

    iterator = Coll_Set_getFirstElement(&(_this->selections));
    while(iterator)
    {
        selection = (DK_SelectionAdmin*)Coll_Iter_getObject(iterator);

        DK_SelectionAdmin_us_resetModificationInformation(selection);

        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* may return null, if not must release the returned value */
DK_CacheAdmin*
DK_ObjectHomeAdmin_ts_getCache(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception)
{
    DK_CacheAdmin* cache = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,"The %s '%p' entity has already been deleted!",
                ENTITY_NAME, _this);
    }
    if(_this->cache)
    {
        cache = (DK_CacheAdmin*)DK_Entity_ts_duplicate((DK_Entity*)_this->cache);
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
    return cache;
}

void
DK_ObjectHomeAdmin_us_resolveMetaModel(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_MMFacade_us_resolveMetaModel(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* no copy made */
Coll_List*
DK_ObjectHomeAdmin_us_getTopicInfos(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->topicInfos);
}

/* returned cache admin not duplicated! */
DK_CacheAdmin*
DK_ObjectHomeAdmin_us_getCache(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->cache;
}

void
DK_ObjectHomeAdmin_ts_setAutoDeref(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    LOC_boolean autoDeref)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    /* TODO ID: 134 _this->autoDeref = autoDeref; */

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_ts_setFilter(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    LOC_string expression)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(expression);

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    /* TODO ID: 136 */
    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_ts_derefAll(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    /* TODO ID: 137 */
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_ts_unDerefAll(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    /* TODO ID: 138 */
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_ts_getAllObjectsForCache(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    /* on stack declaration of helper class to avoid malloc, this var is only needed during this operation */
    DK_ObjectArrayHolder holder;
    DK_ObjectAdmin* objectAdmin;
    DLRL_LS_object lsObject;
    LOC_unsigned_long count;

    /* first thing to do: init holder */
    holder.objectArray = NULL;
    holder.size = 0;
    holder.maxSize = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_ALREADY_DELETED,
            "The %s '%p' entity has already been deleted!",
            ENTITY_NAME,
            _this);
    }

    /* the holders object array may remain null after this operation! */
    DK_ObjectHomeAdmin_us_getAllObjects(_this, exception, &holder);
    DLRL_Exception_PROPAGATE(exception);

    /* now create the sequence to hold the typed language specific objects */
    if(!(*arg))
    {
        objectHomeBridge.createTypedObjectSeq(
            exception,
            userData,
            _this,
            arg,
            holder.size);
    }
    /* now insert each object into the previously created array */
    for(count = 0; count < holder.size; count++)
    {
        objectAdmin = holder.objectArray[count];
        lsObject = DK_ObjectAdmin_us_getLSObject(objectAdmin);/* dont dup */
        /* assign the object to the seq */
        objectHomeBridge.addElementToTypedObjectSeq(
            exception,
            userData,
            _this,
            *arg,
            lsObject,
            count);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    if(holder.objectArray)
    {
        os_free(holder.objectArray);
    }
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_ts_getAllObjectsForCacheAccess(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    DK_CacheAccessAdmin* source,
    void* userData,
    void** arg)
{
    /* on stack declaration of helper class to avoid malloc, this var is only needed during this operation */
    DLRL_LS_object lsObject;
    LOC_long homeIndex = -1;
    DK_CacheAccessTypeRegistry* registry = NULL;
    LOC_unsigned_long size = 0;
    LOC_unsigned_long elementIndex = 0;
    Coll_Iter* iterator = NULL;
    DK_ObjectAdmin* objectAdmin = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_CacheAccessAdmin_lock(source);
    DK_ObjectHomeAdmin_lockAdmin(_this);
    if(!_this->alive)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_ALREADY_DELETED,
            "The %s '%p' entity has already been deleted!",
            ENTITY_NAME,
            _this);
    }
    DK_CacheAccessAdmin_us_checkAlive(source, exception);
    DLRL_Exception_PROPAGATE(exception);

    homeIndex = DK_ObjectHomeAdmin_us_getRegistrationIndex(_this);
    if(homeIndex != -1)
    {
        registry = source->registry[homeIndex];
        size = DK_CacheAccessTypeRegistry_us_getNrOfObjects(registry);
    }

    /* now create the sequence to hold the typed language specific objects */
    if(!(*arg))
    {
        objectHomeBridge.createTypedObjectSeq(
            exception,
            userData,
            _this,
            arg,
            size);
    }

    /* now insert each object into the previously created array */
    if(registry)
    {
        iterator = Coll_Set_getFirstElement(
            DK_CacheAccessTypeRegistry_us_getObjects(registry));
        while(iterator)
        {
            objectAdmin = Coll_Iter_getObject(iterator);
            lsObject = DK_ObjectAdmin_us_getLSObject(objectAdmin);/* dont dup */
            objectHomeBridge.addElementToTypedObjectSeq(
                exception,
                userData,
                _this,
                *arg,
                lsObject,
                elementIndex);
            DLRL_Exception_PROPAGATE(exception);
            elementIndex++;
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(_this);
    DK_CacheAccessAdmin_unlock(source);
    DLRL_INFO(INF_EXIT);
}

/* array holder internal object array may be null after this operation!. if so the size will be indicated as 0 */
void
DK_ObjectHomeAdmin_us_getAllObjects(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectArrayHolder* holder)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(holder);

    if(_this->objectReader)
    {
        DK_ObjectReader_us_getAllObjects(_this->objectReader, exception, holder);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_ObjectHomeAdmin_us_unregisterAllUnresolvedElementsForEntity(
    DK_ObjectHomeAdmin* _this,
    void* userData,
    DK_Entity* entity)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(entity);

    if(_this->objectReader)
    {
        DK_UnresolvedObjectsUtility_us_unregisterAllUnresolvedElementsForEntity(&(_this->objectReader->unresolvedElements),
                                                                                                    userData, entity);
    }

    DLRL_INFO(INF_EXIT);
}

DK_ObjectHomeAdmin*
DK_ObjectHomeAdmin_us_getRelatedHome(
    DK_ObjectHomeAdmin* _this,
    LOC_unsigned_long index)
{
    DK_ObjectHomeAdmin* home = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    if(_this->relatedHomes)
    {
        home = _this->relatedHomes[index];
    }

    DLRL_INFO(INF_EXIT);
    return home;
}

DK_ObjectHomeAdmin*
DK_ObjectHomeAdmin_us_getParent(
    DK_ObjectHomeAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->parent;
}
