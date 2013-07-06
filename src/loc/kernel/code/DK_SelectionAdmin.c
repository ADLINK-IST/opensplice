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

#include "Coll_Compare.h"

/* DLRL util includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL kernel includes */
#include "DK_SelectionAdmin.h"
#include "DK_SelectionBridge.h"
#include "DK_Types.h"
#include "DLRL_Kernel_private.h"
#include "DK_Utility.h"

#define ENTITY_NAME "DLRL Kernel Selection"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

static void
DK_SelectionAdmin_us_destroy(
    DK_Entity* _this);

static void
DK_SelectionAdmin_us_clearMembers(
    DK_SelectionAdmin* _this);

static void
DK_SelectionAdmin_unlockHomeForUpdates(
    DK_SelectionAdmin* _this);

static void
DK_SelectionAdmin_lockHomeForUpdates(
    DK_SelectionAdmin* _this);

DK_SelectionAdmin*
DK_SelectionAdmin_new(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DLRL_LS_object ls_selection,
    DK_SelectionCriterion* criterion,
    DK_CriterionKind kind,
    LOC_boolean autoRefresh,
    LOC_boolean concernsContainedObjects)
{
    DK_SelectionAdmin* _this = NULL;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(home);
    assert(ls_selection);
    assert(criterion);
    assert(kind < DK_CriterionKind_elements);
    /* userData may be null */

    DLRL_ALLOC(_this, DK_SelectionAdmin, exception,  "%s", allocError);

    _this->home = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)home);
    _this->alive = TRUE;
    _this->autoRefresh = autoRefresh;
    _this->concernsContained = FALSE;/* defaults to FALSE. concernsContainedObjects; */
    _this->listener = NULL;
    _this->criterionKind = kind;
    _this->ls_selection = ls_selection;
    Coll_Set_init(&(_this->membership), pointerIsLessThen, TRUE);

    Coll_List_init(&(_this->newMembers));
    Coll_List_init(&(_this->modifiedMembers));
    Coll_List_init(&(_this->removedMembers));

    if(kind == DK_CRITERION_KIND_FILTER)
    {
        _this->criterion.filterCriterion = criterion->filterCriterion;
    } else
    {
        assert(kind == DK_CRITERION_KIND_QUERY);
        _this->criterion.queryCriterion = (DK_QueryCriterion*)DK_Entity_ts_duplicate((DK_Entity*)criterion->queryCriterion);
    }
    DK_Entity_us_init(&(_this->entity), DK_CLASS_SELECTION_ADMIN, DK_SelectionAdmin_us_destroy);
    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);
    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        DK_SelectionAdmin_us_delete(_this, userData);
        DK_Entity_ts_release((DK_Entity*)_this);
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}
DLRL_LS_object
DK_SelectionAdmin_us_getLSFilter(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->criterion.filterCriterion;
}

void
DK_SelectionAdmin_us_destroy(
    DK_Entity* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if(_this)
    {
        if(((DK_SelectionAdmin*)_this)->home)
        {
            DK_Entity_ts_release((DK_Entity*)(((DK_SelectionAdmin*)_this)->home));
        }
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_SelectionAdmin*)_this);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_SelectionAdmin_us_delete(
    DK_SelectionAdmin* _this,
    void* userData)
{
    Coll_Iter* iterator;
    DK_ObjectAdmin* anObjectAdmin = NULL;
    LOC_unsigned_long nrOfElements;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* userData may be null */

    if(_this->alive)
    {
        /* remember, cant free the home reference here! that is done in the destroy which is called when the */
        /* ref count of this entity reaches zero! We need the home for the mutex locking as operations may be invoked */
        /* on already deleted objects! */
        if(_this->listener)
        {
            utilityBridge.releaseLSInterfaceObject(userData, _this->listener);
            _this->listener = NULL;
        }
        iterator = Coll_Set_getFirstElement(&(_this->membership));
        while(iterator)
        {
            anObjectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);

            iterator = Coll_Iter_getNext(iterator);
            Coll_Set_remove(&(_this->membership), anObjectAdmin);
            DK_Entity_ts_release((DK_Entity*)anObjectAdmin);
        }
        nrOfElements = Coll_List_getNrOfElements(&(_this->newMembers));
        while(nrOfElements != 0)
        {
            anObjectAdmin = (DK_ObjectAdmin*)Coll_List_popBack(&(_this->newMembers));
            DK_Entity_ts_release((DK_Entity*)anObjectAdmin);
            nrOfElements--;
        }
        nrOfElements = Coll_List_getNrOfElements(&(_this->modifiedMembers));
        while(nrOfElements != 0)
        {
            anObjectAdmin = (DK_ObjectAdmin*)Coll_List_popBack(&(_this->modifiedMembers));
            DK_Entity_ts_release((DK_Entity*)anObjectAdmin);
            nrOfElements--;
        }
        nrOfElements = Coll_List_getNrOfElements(&(_this->removedMembers));
        while(nrOfElements != 0)
        {
            anObjectAdmin = (DK_ObjectAdmin*)Coll_List_popBack(&(_this->removedMembers));
            DK_Entity_ts_release((DK_Entity*)anObjectAdmin);
            nrOfElements--;
        }
        if(_this->criterionKind == DK_CRITERION_KIND_FILTER)
        {
            utilityBridge.releaseLSInterfaceObject(userData, _this->criterion.filterCriterion);
        } else
        {
            assert(_this->criterionKind == DK_CRITERION_KIND_QUERY);
            /* we arent responsible for deleting the query condition, as we didnt create it! */
            DK_Entity_ts_release((DK_Entity*)_this->criterion.queryCriterion);
        }
        if(_this->ls_selection)
        {
            utilityBridge.releaseLSInterfaceObject(userData, _this->ls_selection);
        }
        _this->alive = FALSE;
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_SelectionAdmin_us_resetModificationInformation(
    DK_SelectionAdmin* _this)
{
    DK_Entity* anObjectAdmin = NULL;
    LOC_unsigned_long nrOfElements;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    nrOfElements = Coll_List_getNrOfElements(&(_this->newMembers));
    while(nrOfElements != 0)
    {
        anObjectAdmin = (DK_Entity*)Coll_List_popBack(&(_this->newMembers));
        DK_Entity_ts_release(anObjectAdmin);
        nrOfElements--;
    }
    nrOfElements = Coll_List_getNrOfElements(&(_this->modifiedMembers));
    while(nrOfElements != 0)
    {
        anObjectAdmin = (DK_Entity*)Coll_List_popBack(&(_this->modifiedMembers));
        DK_Entity_ts_release(anObjectAdmin);
        nrOfElements--;
    }
    nrOfElements = Coll_List_getNrOfElements(&(_this->removedMembers));
    while(nrOfElements != 0)
    {
        anObjectAdmin = (DK_Entity*)Coll_List_popBack(&(_this->removedMembers));
        DK_Entity_ts_release(anObjectAdmin);
        nrOfElements--;
    }

    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_SelectionAdmin_us_getAutoRefresh(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->autoRefresh;
}

LOC_boolean
DK_SelectionAdmin_ts_getAutoRefresh(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception)
{
    LOC_boolean returnValue = FALSE;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_SelectionAdmin_lockHome(_this);
    DK_SelectionAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    returnValue = _this->autoRefresh;

    DLRL_Exception_EXIT(exception);
    DK_SelectionAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

DLRL_LS_object
DK_SelectionAdmin_ts_getListener(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_LS_object retVal = NULL;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_SelectionAdmin_lockHomeForUpdates(_this);
    DK_SelectionAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    if(_this->listener)
    {
        retVal = utilityBridge.localDuplicateLSInterfaceObject(
            userData,
            _this->listener);
        if(!retVal)
        {
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Unable to complete operation, out of resources");
        }
    }
    DLRL_Exception_EXIT(exception);
    DK_SelectionAdmin_unlockHomeForUpdates(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

DLRL_LS_object
DK_SelectionAdmin_ts_setListener(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object listener)
{
    DLRL_LS_object retVal = NULL;
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_SelectionAdmin_lockHomeForUpdates(_this);
    DK_SelectionAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    if(_this->listener)
    {
        retVal = utilityBridge.localDuplicateLSInterfaceObject(
            userData,
            _this->listener);
        if(!retVal)
        {
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Unable to complete operation, out of resources");
        }
        utilityBridge.releaseLSInterfaceObject(userData, _this->listener);
        _this->listener = NULL;
    }
    if(listener)
    {
        _this->listener = utilityBridge.duplicateLSInterfaceObject(
            userData,
            listener);
        if(!_this->listener)
        {
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Unable to complete operation, out of resources");
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_SelectionAdmin_unlockHomeForUpdates(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

void
DK_SelectionAdmin_us_triggerListener(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    Coll_Iter* iterator;
    DK_ObjectAdmin* objectAdmin;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    if(_this->listener)
    {
        iterator = Coll_List_getFirstElement(&(_this->newMembers));
        while(iterator)
        {
            objectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);

            selectionBridge.triggerListenerInsertedObject(
                exception,
                userData,
                _this->home,
                _this->listener,
                objectAdmin);
            DLRL_Exception_PROPAGATE(exception);
            iterator = Coll_Iter_getNext(iterator);
        }
        iterator = Coll_List_getFirstElement(&(_this->modifiedMembers));
        while(iterator)
        {
            objectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);

            selectionBridge.triggerListenerModifiedObject(
                exception,
                userData,
                _this->home,
                _this->listener,
                objectAdmin);
            DLRL_Exception_PROPAGATE(exception);
            iterator = Coll_Iter_getNext(iterator);
        }
        iterator = Coll_List_getFirstElement(&(_this->removedMembers));
        while(iterator)
        {
            objectAdmin = (DK_ObjectAdmin*)Coll_Iter_getObject(iterator);

            selectionBridge.triggerListenerRemovedObject(
                exception,
                userData,
                _this->home,
                _this->listener,
                objectAdmin);
            DLRL_Exception_PROPAGATE(exception);
            iterator = Coll_Iter_getNext(iterator);
        }
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_SelectionAdmin_ts_getConcernsContained(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception)
{
    LOC_boolean returnValue = FALSE;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_SelectionAdmin_lockHome(_this);
    DK_SelectionAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    returnValue = _this->concernsContained;

    DLRL_Exception_EXIT(exception);
    DK_SelectionAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return returnValue;
}

void
DK_SelectionAdmin_ts_refresh(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    /* on stack declaration of helper class to avoid malloc, this var is only needed during this operation */
    DK_ObjectArrayHolder holder;
    DK_ObjectAdmin** passedAdminsArray = NULL;
    LOC_unsigned_long passedAdminsArraySize = 0;
    LOC_unsigned_long count = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    /* init holder */
    holder.objectArray = NULL;
    holder.size = 0;
    holder.maxSize = 0;

    DK_SelectionAdmin_lockHomeForUpdates(_this);
    DK_SelectionAdmin_lockHome(_this);
    DK_SelectionAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    DK_SelectionAdmin_us_clearMembers(_this);
    DK_SelectionAdmin_us_resetModificationInformation(_this);
        /* now depending on the criterion, we decide which objects become a part of the selection and which do not. */
    if(_this->criterionKind == DK_CRITERION_KIND_FILTER)
    {
        /* the holders object array may remain null after this operation! */
        DK_ObjectHomeAdmin_us_getAllObjects(_this->home, exception, &holder);
        DLRL_Exception_PROPAGATE(exception);
        /* do a callback to determine which obejct will become a part of the selection. The callback will return */
        /* an array of unsigned longs that indicates the indexes of the object admins of the 'holder->objectArray' var */
        /* that need to be put up as membership */
        if(holder.size > 0)
        {
            /* important: Both the update and admin mutex of the owning object home are locked during execution of this */
            /* operation */
            passedAdminsArray = selectionBridge.checkObjects(exception, userData, _this, _this->criterion.filterCriterion,
                                                        holder.objectArray, holder.size, &passedAdminsArraySize);
            DLRL_Exception_PROPAGATE(exception);
        }
        for(count = 0; count < passedAdminsArraySize; count++)
        {
            assert(passedAdminsArray[count]);
            Coll_Set_add(&(_this->membership), DK_Entity_ts_duplicate((DK_Entity*)passedAdminsArray[count]));
        }
    }/* query is not supported */

    DLRL_Exception_EXIT(exception);
    if(holder.objectArray)
    {
        os_free(holder.objectArray);
    }
    if(passedAdminsArray)
    {
        os_free(passedAdminsArray);
    }
    DK_SelectionAdmin_unlockHome(_this);
    DK_SelectionAdmin_unlockHomeForUpdates(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_SelectionAdmin_us_clearMembers(
    DK_SelectionAdmin* _this)
{
    DK_Entity* anObjectAdmin = NULL;
    Coll_Iter* iterator;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    iterator = Coll_Set_getFirstElement(&(_this->membership));
    while(iterator)
    {
        anObjectAdmin = (DK_Entity*)Coll_Iter_getObject(iterator);

        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&(_this->membership), anObjectAdmin);
        DK_Entity_ts_release(anObjectAdmin);
    }
    assert(Coll_Set_getNrOfElements(&(_this->membership)) == 0);
    DLRL_INFO(INF_EXIT);
}

DLRL_LS_object
DK_SelectionAdmin_us_getLSSelection(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_selection;
}

DLRL_LS_object
DK_SelectionAdmin_ts_getCriterion(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_LS_object retVal = NULL;
    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_SelectionAdmin_lockHome(_this);
    DK_SelectionAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);

    if(_this->criterionKind == DK_CRITERION_KIND_FILTER)
    {
        retVal = utilityBridge.localDuplicateLSInterfaceObject(userData, _this->criterion.filterCriterion);
        if(!retVal)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to complete operation, out of resources");
        }
    } else
    {
        assert(_this->criterionKind == DK_CRITERION_KIND_QUERY);
        retVal = NULL;
        /* not yet supported */
    }

    DLRL_Exception_EXIT(exception);
    DK_SelectionAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

void
DK_SelectionAdmin_lockHome(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->home);

    DK_ObjectHomeAdmin_lockAdmin(_this->home);
    DLRL_INFO(INF_EXIT);
}

void
DK_SelectionAdmin_unlockHome(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->home);

    DK_ObjectHomeAdmin_unlockAdmin(_this->home);
    DLRL_INFO(INF_EXIT);
}

void
DK_SelectionAdmin_lockHomeForUpdates(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->home);

    DK_ObjectHomeAdmin_lockUpdate(_this->home);
    DLRL_INFO(INF_EXIT);
}

void
DK_SelectionAdmin_unlockHomeForUpdates(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(_this->home);

    DK_ObjectHomeAdmin_unlockUpdate(_this->home);
    DLRL_INFO(INF_EXIT);
}

void
DK_SelectionAdmin_us_checkAlive(
    DK_SelectionAdmin* _this,
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

Coll_Set*
DK_SelectionAdmin_us_getMembers(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->membership);
}

Coll_List*
DK_SelectionAdmin_us_getInsertedMembers(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->newMembers);
}

Coll_List*
DK_SelectionAdmin_us_getModifiedMembers(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->modifiedMembers);
}


Coll_List*
DK_SelectionAdmin_us_getRemovedMembers(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->removedMembers);
}


void
DK_SelectionAdmin_ts_getLSMembers(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(arg);

    DK_SelectionAdmin_lockHome(_this);

    DK_SelectionAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectHomeAdmin_us_checkAlive(_this->home, exception);
    DLRL_Exception_PROPAGATE(exception);

    DK_Utility_us_copyObjectsIntoTypedObjectSeq(
        exception,
        userData,
        _this->home,
        Coll_Set_getFirstElement(&(_this->membership)),
        Coll_Set_getNrOfElements(&(_this->membership)),
        arg,
        FALSE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_SelectionAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_SelectionAdmin_ts_getLSInsertedMembers(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(arg);

    DK_SelectionAdmin_lockHome(_this);

    DK_SelectionAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectHomeAdmin_us_checkAlive(_this->home, exception);
    DLRL_Exception_PROPAGATE(exception);

    DK_Utility_us_copyObjectsIntoTypedObjectSeq(
        exception,
        userData,
        _this->home,
        Coll_List_getFirstElement(&(_this->newMembers)),
        Coll_List_getNrOfElements(&(_this->newMembers)),
        arg,
        FALSE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_SelectionAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_SelectionAdmin_ts_getLSModifiedMembers(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(arg);

    DK_SelectionAdmin_lockHome(_this);

    DK_SelectionAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectHomeAdmin_us_checkAlive(_this->home, exception);
    DLRL_Exception_PROPAGATE(exception);

    DK_Utility_us_copyObjectsIntoTypedObjectSeq(
        exception,
        userData,
        _this->home,
        Coll_List_getFirstElement(&(_this->modifiedMembers)),
        Coll_List_getNrOfElements(&(_this->modifiedMembers)),
        arg,
        FALSE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_SelectionAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_SelectionAdmin_ts_getLSRemovedMembers(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(arg);

    DK_SelectionAdmin_lockHome(_this);

    DK_SelectionAdmin_us_checkAlive(_this, exception);
    DLRL_Exception_PROPAGATE(exception);
    DK_ObjectHomeAdmin_us_checkAlive(_this->home, exception);
    DLRL_Exception_PROPAGATE(exception);

    DK_Utility_us_copyObjectsIntoTypedObjectSeq(
        exception,
        userData,
        _this->home,
        Coll_List_getFirstElement(&(_this->removedMembers)),
        Coll_List_getNrOfElements(&(_this->removedMembers)),
        arg,
        FALSE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_SelectionAdmin_unlockHome(_this);
    DLRL_INFO(INF_EXIT);
}

DK_ObjectHomeAdmin*
DK_SelectionAdmin_us_getOwnerHome(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->home;
}

DK_CriterionKind
DK_SelectionAdmin_us_getCriterionKind(
    DK_SelectionAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->criterionKind;
}

void
DK_SelectionAdmin_us_addMember(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin,
    LOC_boolean belongs)
{
    LOC_unsigned_long nrOfMembersPre;
    LOC_unsigned_long nrOfMembersPost;
    LOC_long retVal;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(objectAdmin);
    if(belongs)
    {
        nrOfMembersPre = Coll_Set_getNrOfElements(&(_this->membership));
        retVal = Coll_Set_add(&(_this->membership), (void*)objectAdmin);
        if(retVal == COLL_ERROR_ALLOC)
        {
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Out of resources.");
        }
        DK_Entity_ts_duplicate((DK_Entity*)objectAdmin);
        nrOfMembersPost = Coll_Set_getNrOfElements(&(_this->membership));
        /* so yeah we could have the set just return an already contains error
         * code, but then again we could also not. This was easier and OMG its
         * probably not the best way, but then again the best way is relative
         * anyways. So yeah i took the easy route, sue me.
         */
        if(nrOfMembersPre == nrOfMembersPost)
        {
            /* already existed, add to modified list */
            retVal = Coll_List_pushBack(&(_this->modifiedMembers), objectAdmin);
            if(retVal == COLL_ERROR_ALLOC)
            {
                DLRL_Exception_THROW(
                    exception,
                    DLRL_OUT_OF_MEMORY,
                    "Out of resources.");
            }
            DK_Entity_ts_duplicate((DK_Entity*)objectAdmin);
        } else
        {
            assert(nrOfMembersPre < nrOfMembersPost);
            /* new object added, add to new list */
            retVal = Coll_List_pushBack(&(_this->newMembers), objectAdmin);
            if(retVal == COLL_ERROR_ALLOC)
            {
                DLRL_Exception_THROW(
                    exception,
                    DLRL_OUT_OF_MEMORY,
                    "Out of resources.");
            }
            DK_Entity_ts_duplicate((DK_Entity*)objectAdmin);
        }
    } else
    {
        void* removedElement;

        removedElement = Coll_Set_remove(
            &(_this->membership),
            (void*)objectAdmin);
        if(removedElement)
        {
            /*do not release, let the removed list take over the entity ref */
            /* was a member, so add to removed list */
            retVal = Coll_List_pushBack(&(_this->removedMembers), objectAdmin);
            if(retVal == COLL_ERROR_ALLOC)
            {
                /* frack me, something went wrong, lets at least release the
                 * object we removed, though we are outta memory must likely
                 * anyway. but hey it keeps us off the streets.
                 */
                DK_Entity_ts_release((DK_Entity*)objectAdmin);
                DLRL_Exception_THROW(
                    exception,
                    DLRL_OUT_OF_MEMORY,
                    "Out of resources.");
            }
        } /* else do nothing, which is real fracking easy. */
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}
