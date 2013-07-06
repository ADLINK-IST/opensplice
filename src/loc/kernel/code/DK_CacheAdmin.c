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
/*  C includes */
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* OS abstraction layer includes */
#include "os_heap.h"

/* user layer includes */
#include "u_entity.h"

/* DLRL utilities includes */
#include "DLRL_Util.h"
#include "DLRL_Report.h"

/* Collection includes */
#include "Coll_Compare.h"

/* DLRL kernel includes */
#include "DK_CacheAdmin.h"
#include "DK_CacheBridge.h"
#include "DK_EventDispatcher.h"
#include "DK_DCPSUtilityBridge.h"
#include "DK_UtilityBridge.h"
#include "DK_ObjectReader.h"
#include "DK_CollectionReader.h"
#include "DLRL_Kernel_private.h"

#define ENTITY_NAME "DLRL Kernel CacheAdmin"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

static void
DK_CacheAdmin_us_createSubscriber(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    u_participant participant);

static void
DK_CacheAdmin_us_createPublisher(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    u_participant participant);

static void
DK_CacheAdmin_us_createConnectivityEntities(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    u_participant participant,
    const DK_Usage usage);

static void
DK_CacheAdmin_us_delete(
    DK_CacheAdmin* _this,
    void* userData);

static void
    DK_CacheAdmin_us_destroy(
    DK_Entity * _this);

static void
DK_CacheAdmin_us_resetObjectModificationInformation(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* NOT IN DESIGN */
static DK_ObjectHomeAdmin*
DK_CacheAdmin_us_findHomeByIndex(
    DK_CacheAdmin* _this,
    const LOC_long index);

/* NOT IN DESIGN */
static LOC_boolean
DK_CacheAdmin_us_isAlive(
    DK_CacheAdmin* _this);

static void
DK_CacheAdmin_us_handleAutoUpdateEvents(
    DK_Entity* _this,
    DLRL_Exception* exception,
    void* userData);

static void
DK_CacheAdmin_us_deactivateAction(
    DLRL_Exception* exception,
    u_waitset waitset,
    DK_Entity* interestedEntity);

static void
DK_CacheAdmin_us_activateAction(
    DLRL_Exception* exception,
    u_waitset waitset,
    DK_Entity* interestedEntity);

typedef u_result
(*DK_CacheAdmin_us_waitsetAction)(
    u_waitset waitset,
    u_entity entity,
    void* arg);

static void
DK_CacheAdmin_us_waitsetActionGeneric(
    DLRL_Exception* exception,
    u_waitset waitset,
    DK_Entity* interestedEntity,
    LOC_boolean doAttach);

/******************************************************************************
******************* DLRL Kernel API calls of the CacheAdmin *******************
*******************************************************************************/
DK_CacheAdmin*
DK_CacheAdmin_new(
    DLRL_Exception* exception,
    DLRL_LS_object ls_cache,
    void* userData,
    const DK_Usage usage,
    u_participant participant,
    DLRL_LS_object ls_participant,
    const LOC_string name)
{
    os_mutexAttr updateMutexAttr;
    os_result resultUpdateInit;
    os_mutexAttr adminMutexAttr;
    os_result resultAdminInit;
    DK_CacheAdmin* _this;

    DLRL_INFO(INF_ENTER);
    assert(exception);
    assert(usage < DK_Usage_elements);
    assert(name);
    assert(participant);
    /* ls_cache may be null */
    /* userData may be null */
    /* ls_participant may be null */

    /* Generic constructor behavior:
        1) allocate the entity (in this case the CacheAdmin)
        2) Set all attribute to default (NIL) values
        3) Init any additional mutexes this entity uses (in this case the
           update mutex).If this fails then free the allocated entity of step 1
        4) Init the DK_Entity mutex for reference/administrative locking. If
           this fails then free the mutexes inited in step 3 and the entity of
           step 1
        5) Continue to init the rest of the entity (any lists, other objects
           etc). If anything fails here (an exception is thrown) then use the
           unsafe_delete function to clean things and release the entity pointer
     */

    DLRL_ALLOC(
        _this,
        DK_CacheAdmin,
        exception,
        "%s '%s'",
        allocError,
        DLRL_VALID_NAME(name));

    /* set default values for everything */
    _this->name = name;
    _this->base.alive = TRUE;
    _this->base.usage = usage;
    _this->updatesEnabled = FALSE;
    _this->participant = participant;
    _this->ls_participant = ls_participant;
    _this->publisher = NULL;
    _this->ls_publisher = NULL;
    _this->dispatcher = NULL;
    _this->base.ls_cacheBase = ls_cache;
    _this->subscriber = NULL;
    _this->ls_subscriber = NULL;
    _this->pub_sub_state = DK_PUB_SUB_STATE_INITIAL;
    Coll_List_init(&(_this->homes));
    Coll_Set_init(&(_this->cache_accesses), pointerIsLessThen, TRUE);
    Coll_Set_init(&(_this->listeners), pointerIsLessThen, TRUE);

    /* Setup & init mutex: Set scope of mutex to local */
    updateMutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
    resultUpdateInit = os_mutexInit(&(_this->updateMutex), &updateMutexAttr);
    if(resultUpdateInit != os_resultSuccess)
    {
        os_free(_this);
        _this = NULL;
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "%s '%s' update mutex init failed",
            allocError,
            DLRL_VALID_NAME(name));
    }

    adminMutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
    resultAdminInit = os_mutexInit(&(_this->adminMutex), &adminMutexAttr);
    if(resultAdminInit != os_resultSuccess)
    {
        os_mutexDestroy (&(_this->updateMutex));
        os_free(_this);
        _this = NULL;
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "%s '%s' admin mutex init failed",
            allocError,
            DLRL_VALID_NAME(name));
    }

    DK_Entity_us_init(
        &(_this->base.entity),
        DK_CLASS_CACHE_ADMIN,
        DK_CacheAdmin_us_destroy);

    DK_CacheAdmin_us_createConnectivityEntities(
        _this,
        exception,
        userData,
        participant,
        usage);
    DLRL_Exception_PROPAGATE(exception);

    if(_this->base.usage != DK_USAGE_WRITE_ONLY)
    {
        assert(_this->subscriber);
        _this->dispatcher = DK_EventDispatcher_new(
            exception,
            userData,
            (DK_Entity*)_this,
            (DK_EventDispatcher_us_eventAction)DK_CacheAdmin_us_handleAutoUpdateEvents,
            _this->participant);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        /* set user layer participant to null to prevent double free, as caller
         * of this operation will still assume ownership if this function has
         * failed!
         */
        _this->participant = NULL;
        DK_CacheAdmin_us_delete(_this, userData);
        DK_Entity_ts_release((DK_Entity*)_this);
        _this = NULL;
    }

    DLRL_INFO(INF_EXIT);
    return _this;
}

void DK_CacheAdmin_us_dispatcher_terminate(
    DK_CacheAdmin* _this)
{
    DLRL_Exception exception;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_Exception_init(&exception);

    if(_this->dispatcher)
    {
        /* first set the terminate flag to true */
        DK_EventDispatcher_us_setTerminate(_this->dispatcher);
        /* then notify the dispatcher so it awakens and can exit */
        DK_EventDispatcher_us_notify(_this->dispatcher, &exception);
        if(exception.exceptionID != DLRL_NO_EXCEPTION)
        {
            DLRL_REPORT(
                REPORT_ERROR,
                "Exception %s occured when attempting to notify event "
                "dispatcher.\n%s",
                DLRL_Exception_exceptionIDToString(exception.exceptionID),
                exception.exceptionMessage);
            /* reinit the exception in case it is used later in the function */
            DLRL_Exception_init(&exception);
        }

        /* release reference, no special delete needed */
        DK_Entity_ts_release((DK_Entity*)_this->dispatcher);
        _this->dispatcher = NULL;
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_ts_delete(
    DK_CacheAdmin* _this,
    void* userData)
{
    LOC_boolean waitForThread = FALSE;
    os_threadId threadId;
    os_result osResult;

    DLRL_INFO_OBJECT(INF_ENTER);

    /* userData may be null */
    /* _this may be null */

    /* The participant entity proxy can not be deleted until after the
     * DK_EventDispatcher_us_notify operation is called and the dispatcher
     * thread is complete.  This is because the participant is used by
     * both the main thread and the dispatcher thread.  The solution here
     * is to call the dispatcher terminate thread from within the locks,
     * and call os_threadWaitExit outside the locks (to avoid potential
     * deadlocking).  Then call DK_CacheAdmin_us_delete from within the
     * locked mutexes again.
     */

    if(_this)
    {
        DK_CacheAdmin_lockUpdates(_this);
        DK_CacheAdmin_lockAdministrative(_this);
        /* only continue if cache is still alive */
        if(DK_CacheAdmin_us_isAlive(_this))
        {
            if(_this->dispatcher)
            {
                waitForThread = TRUE;
                /* we need the thread id to perform the wait with. */
                threadId = DK_EventDispatcher_us_getThreadId(_this->dispatcher);
                DK_CacheAdmin_us_dispatcher_terminate(_this);
            }
        }
        DK_CacheAdmin_unlockAdministrative(_this);
        DK_CacheAdmin_unlockUpdates(_this);

        if(waitForThread)
        {
            /* wait until the thread is finished */
            osResult = os_threadWaitExit(threadId, NULL);
            if(osResult != os_resultSuccess)
            {
                DLRL_REPORT(REPORT_ERROR, "An error occured when attempting to "
                    "wait for event dispatcher thread exit.");
            }
        }

        /* Now any dispatcher thread has finished, delete the CacheAdmin,
         * which can now safely free the entities (including the participant
         * which was shared with the thread) */

        DK_CacheAdmin_lockUpdates(_this);
        DK_CacheAdmin_lockAdministrative(_this);
        /* only continue if cache is still alive */
        if(DK_CacheAdmin_us_isAlive(_this))
        {
            DK_CacheAdmin_us_delete(_this, userData);
        }
        DK_CacheAdmin_unlockAdministrative(_this);
        DK_CacheAdmin_unlockUpdates(_this);
    }

    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_CacheAdmin_ts_isAlive(
    DK_CacheAdmin* _this)
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return _this->base.alive;
}

/* this operation exists, because it garantees no locks, the ts variant current
 * is the same, but one doesnt know if it will claim a mutex in the future, so
 * we have this us variant which ensures no mutex is claimed
 */
LOC_boolean
DK_CacheAdmin_us_isAlive(
    DK_CacheAdmin* _this)
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return _this->base.alive;
}

LOC_long
DK_CacheAdmin_ts_registerHome(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home)
{
    LOC_long registrationIndex = -1;
    LOC_long returnCode = COLL_OK;
    Coll_Iter* iterator = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(home);

    DK_CacheAdmin_lockAdministrative(_this);
    DK_ObjectHomeAdmin_lockAdmin(home);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    DK_ObjectHomeAdmin_us_checkAlive(home, exception);
    DLRL_Exception_PROPAGATE(exception);
    /* homes may only be registered in initial pub sub mode and to 1 cache */
    if(_this->pub_sub_state != DK_PUB_SUB_STATE_INITIAL)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to register home to %s '%s'. %s",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name),
            "The cache must be in initial publication and subscribtion mode to "
            "register homes.");
    }
    /* the check for a -1 value on the registration index is important for the
    * call to set the registration index in the home, later on in this
    * operation. For locking strategy purposes the registration index may never
    * be changed once set to a value (not -1) value. See the memo regarding the
    * locking strategy for more info.
    */
    if(DK_ObjectHomeAdmin_us_hasCacheAdmin(home) ||
        (DK_ObjectHomeAdmin_us_getRegistrationIndex(home) != -1))
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to register home to %s '%s'. %s %s",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name),
            "The home being registered is already registered with another DLRL ",
            "cache. An home instance may only be registered to one DLRL cache.");
    }

    /* check if another home instance representing the same type is already
     * registered to the Cache
     */
    iterator = Coll_List_getFirstElement(&(_this->homes));
    while(iterator)
    {
        DK_ObjectHomeAdmin* aHome;

        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        /* a home retrieved from this list can never be the same as a home
         * object provided as parameter here due to the check if the home
         * provided as parameter was already registered before and consequently
         * throwing an exception. this therefore prevents a potential deadlock
         * when locking the homes from the homes list
         */
        assert(DK_ObjectHomeAdmin_us_hasCacheAdmin(aHome));
        assert(DK_ObjectHomeAdmin_us_getRegistrationIndex(aHome) != -1);
        DK_ObjectHomeAdmin_lockAdmin(aHome);
        if(0 == strcmp(DK_ObjectHomeAdmin_us_getName(aHome),
            DK_ObjectHomeAdmin_us_getName(home)))
        {
            DK_ObjectHomeAdmin_unlockAdmin(aHome);
            DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
             "Unable to register home to %s '%s'. The type represented by this "
             "home is already registered within DLRL.",
             ENTITY_NAME, DLRL_VALID_NAME(_this->name));
        }
        DK_ObjectHomeAdmin_unlockAdmin(aHome);
        iterator = Coll_Iter_getNext(iterator);
    }

    registrationIndex = (LOC_long)Coll_List_getNrOfElements(&(_this->homes));
    /* Register the home. */
    returnCode = Coll_List_pushBack(
        &(_this->homes),
        (void *)DK_Entity_ts_duplicate((DK_Entity*)home));
    if(returnCode != COLL_OK)
    {
        registrationIndex = -1;/* reset registration index */
        DK_Entity_ts_release((DK_Entity*)home);
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to register home to %s '%s'. %s",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name),
            "Allocation error when adding the home entity to the list of homes "
            "within the cache.");
    }
    /* because in the above code an exception is thrown when the registration
     * index is not -1, we do not have to perform any checks here. Its good to
     * remember that we may only set the registration index of an object home
     * once from -1 to another not -1 value.
     */
    DK_ObjectHomeAdmin_us_setRegistrationIndex(home, registrationIndex);
    DK_ObjectHomeAdmin_us_setCacheAdmin(home, _this);

    DLRL_Exception_EXIT(exception);
    DK_ObjectHomeAdmin_unlockAdmin(home);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
    return registrationIndex;
}

DLRL_LS_object
DK_CacheAdmin_ts_findLSHomeByIndex(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const LOC_long index)
{
    DK_ObjectHomeAdmin* aHome = NULL;
    DLRL_LS_object lsHome = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    /* must claim mutex here to prevent other threads from deleting the cache.
     * If a cache is deleted the list of object homes is cleared in the delete,
     * which would negatively effect the code in this operation
    */
    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    aHome = DK_CacheAdmin_us_findHomeByIndex(_this, index);
    if(aHome)
    {
        lsHome = DK_ObjectHomeAdmin_ts_getLSHome(aHome, exception, userData);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);

    DLRL_INFO(INF_EXIT);
    return lsHome;
}

/* The returned lshome must be released by the caller */
DLRL_LS_object
DK_CacheAdmin_ts_findLSHomeByName(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    LOC_const_string name)
{
    DK_ObjectHomeAdmin* aHome = NULL;
    DLRL_LS_object lsHome = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(name);
    assert(exception);

    /* must claim mutex here to prevent other threads from deleting the cache.
     * If a cache is deleted the list of object homes is cleared in the delete,
     * which would negatively effect the code in this operation
     */
    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    aHome = DK_CacheAdmin_us_findHomeByName(_this, name);

    if(aHome)
    {
        lsHome = DK_ObjectHomeAdmin_ts_getLSHome(aHome, exception, userData);
        DLRL_Exception_PROPAGATE(exception);
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);

    DLRL_INFO(INF_EXIT);
    return lsHome;
}

/* The returned home must be released by the caller */
DK_ObjectHomeAdmin*
DK_CacheAdmin_ts_findHomeByName(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    LOC_const_string name)
{
    DK_ObjectHomeAdmin* aHome = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(name);
    assert(exception);

    /* must claim mutex here to prevent other threads from deleting the cache.
     * If a cache is deleted the list of object homes is cleared in the delete,
     * which would negatively effect the code in this operation
     */
    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    aHome = DK_CacheAdmin_us_findHomeByName(_this, name);
    if(aHome)
    {
        aHome = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)aHome);
    }
    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);

    DLRL_INFO(INF_EXIT);
    return aHome;
}

/* The returned home must be released by the caller */
DK_ObjectHomeAdmin*
DK_CacheAdmin_ts_findHomeByIndex(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    const LOC_long index)
{
    DK_ObjectHomeAdmin* aHome = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    /* must claim mutex here to prevent other threads from deleting the cache.
     * If a cache is deleted the list of object homesis cleared in the delete,
     * which would negatively effect the code in this operation
     */
    DK_CacheAdmin_lockAdministrative(_this);

    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    aHome = DK_CacheAdmin_us_findHomeByIndex(_this, index);
    if(aHome)
    {
        aHome = (DK_ObjectHomeAdmin*)DK_Entity_ts_duplicate((DK_Entity*)aHome);
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
    return aHome;
}

DK_ObjectHomeAdmin*
DK_CacheAdmin_us_findHomeByIndex(
    DK_CacheAdmin* _this,
    const LOC_long index)
{
    DK_ObjectHomeAdmin* aHome = NULL;
    LOC_long nrOfElements;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    nrOfElements = (LOC_long)Coll_List_getNrOfElements(&(_this->homes));
    if((index < nrOfElements) && (index >=0))
    {
        aHome = Coll_List_getObject(&(_this->homes), index);
    }

    DLRL_INFO(INF_EXIT);
    return aHome;
}

void
DK_CacheAdmin_ts_registerAllForPubSub(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DK_ObjectHomeAdmin* aHome = NULL;
    Coll_Iter* iterator = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_CacheAdmin_lockAdministrative(_this);

    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    if(Coll_List_getNrOfElements(&(_this->homes)) == 0)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to register publication and subscribtion entities for %s "
            "'%s'. %s",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name),
            "The cache has no homes registered! Without homes there is nothing "
            "to register!");
    }

    if(_this->pub_sub_state == DK_PUB_SUB_STATE_ENABLED)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to register publication and subscribtion entities for %s "
            "'%s'. %s",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name),
            "The cache is already in enabled publication and subscribtion "
            "mode!");
    }
    /* if registered, this operation is seen as a no-op */
    if(_this->pub_sub_state != DK_PUB_SUB_STATE_REGISTERED)
    {
        /* first lock all homes, so we can load the meta model correct and set
         * required relation ships between homes
         */
        iterator = Coll_List_getFirstElement(&(_this->homes));
        while(iterator)
        {
            aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
            DK_ObjectHomeAdmin_lockAdmin(aHome);
            DK_ObjectHomeAdmin_us_checkAlive(aHome, exception);
            /* propagate is done after unlock, right now we must continue to
             * lock everything
             */
            iterator = Coll_Iter_getNext(iterator);
        }
        /* now load all meta model information and set relations and create
         * dcps entities.
         */
        iterator = Coll_List_getFirstElement(&(_this->homes));
        while(iterator && exception->exceptionID == DLRL_NO_EXCEPTION)
        {
            aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
            DK_ObjectHomeAdmin_us_loadMetamodel(aHome, exception, userData);
            /* propagate is done after unlock! */
            if(exception->exceptionID == DLRL_NO_EXCEPTION)
            {
                /* bad home definition can occur here */
                DK_ObjectHomeAdmin_us_setRelations(aHome, exception);
                /* propagate is done after unlock! */
                if(exception->exceptionID == DLRL_NO_EXCEPTION)
                {
                    /* DCPS error can occur here */
                    DK_ObjectHomeAdmin_us_createDCPSEntities(
                        aHome,
                        exception,
                        userData);
                    /* propagate is done after unlock! */
                }
            }
            iterator = Coll_Iter_getNext(iterator);
        }
        /* when all meta models are loaded the homes need to be iterated again
         * so that the meta model can be connected to eachother certain data
         * within the a single meta model is only known by identifying strings
         * such as the case for relations. To ease management at DLRL run time
         * this resolving is done once during registration.
         */
        iterator = Coll_List_getFirstElement(&(_this->homes));
        while(iterator && exception->exceptionID == DLRL_NO_EXCEPTION)
        {
            aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
            DK_ObjectHomeAdmin_us_resolveMetaModel(aHome, exception, userData);
            /* propagate is done after unlock! */
            iterator = Coll_Iter_getNext(iterator);
        }

        /* once we have done our tasks, unlock the homes again. */
        iterator = Coll_List_getFirstElement(&(_this->homes));
        while(iterator)
        {
            aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
            DK_ObjectHomeAdmin_unlockAdmin(aHome);
            iterator = Coll_Iter_getNext(iterator);
        }
        /* propagate after the homes have been unlock again, not before! */
        DLRL_Exception_PROPAGATE(exception);
        _this->pub_sub_state = DK_PUB_SUB_STATE_REGISTERED;
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_ts_enableAllForPubSub(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DK_ObjectHomeAdmin* aHome = NULL;
    Coll_Iter* iterator = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAdmin_lockUpdates(_this);
    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    if(_this->pub_sub_state == DK_PUB_SUB_STATE_INITIAL)
    {
        DLRL_Exception_THROW(exception, DLRL_PRECONDITION_NOT_MET,
            "Unable to enable publication and subscribtion entities for %s "
            "'%s'. %s %s",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name),
            "The cache is still in initial publication and subscribtion mode. ",
            "Must register the DCPS entities for publication and subscribtion "
            "first!");
    }
    if(_this->pub_sub_state != DK_PUB_SUB_STATE_ENABLED)
    {
        if(_this->publisher)
        {
            dcpsUtilityBridge.enableEntity(
                exception,
                userData,
                _this->ls_publisher);
            DLRL_Exception_PROPAGATE(exception);
        }
        if(_this->subscriber)
        {
            dcpsUtilityBridge.enableEntity(
                exception,
                userData,
                _this->ls_subscriber);
            DLRL_Exception_PROPAGATE(exception);
        }
        iterator = Coll_List_getFirstElement(&(_this->homes));
        while(iterator)
        {
            aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
            DK_ObjectHomeAdmin_ts_enableDCPSEntities(aHome, exception, userData);
            DLRL_Exception_PROPAGATE(exception);
            iterator = Coll_Iter_getNext(iterator);
        }
        if(_this->updatesEnabled && _this->dispatcher)
        {
            /* in default mode register it if the cache is in read mode */
            DK_EventDispatcher_us_activate(
                _this->dispatcher,
                exception,
                DK_CacheAdmin_us_activateAction);
            DLRL_Exception_PROPAGATE(exception);
        }
        _this->pub_sub_state = DK_PUB_SUB_STATE_ENABLED;
    }/* else do nothing */

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DK_CacheAdmin_unlockUpdates(_this);
    DLRL_INFO(INF_EXIT);
}

DK_Usage
DK_CacheAdmin_ts_getCacheUsage(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception)
{
    DK_Usage usage = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAdmin_lockAdministrative(_this);

    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    usage = DK_CacheBase_us_getCacheUsage((DK_CacheBase*)_this);

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
    return usage;
}

DK_PubSubState
DK_CacheAdmin_ts_getPubSubState(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception)
{
    DK_PubSubState state = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAdmin_lockAdministrative(_this);

    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    state = _this->pub_sub_state;

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);

    DLRL_INFO(INF_EXIT);
    return state;
}

DK_Usage
DK_CacheAdmin_ts_getUsage(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception)
{
    DK_Usage usage = 0;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAdmin_lockAdministrative(_this);

    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    usage = _this->base.usage;

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);

    DLRL_INFO(INF_EXIT);
    return usage;
}

DK_PubSubState
DK_CacheAdmin_us_getPubSubState(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->pub_sub_state;
}

void
DK_CacheAdmin_ts_registerLSCache(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    const DLRL_LS_object ls_cache)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(ls_cache);

    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    _this->base.ls_cacheBase = ls_cache;

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);

    DLRL_INFO(INF_EXIT);
}

/* setters of this variable are protected by the update mutex as they trigger
 * listeners however this attribute is but a long, and does not require the
 * protection of the update mutex as its not relevant.
 */
LOC_boolean
DK_CacheAdmin_ts_getUpdatesEnabled(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception)
{
    LOC_boolean updatesEnabled = FALSE;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    /* no need to claim update mutex, the updatesEnabled attribute is just a
     * boolean, and we are only getting it here locking it wont anything, as if
     * the boolean value were to be changed by another thread while this
     * operation was executing then the result would be determined by whom had
     * the lock first, now the result is determined by who gets (this operation)
     * or set (the other two related operations) the boolean first. all in all
     * there is no  distiguisable difference for the application at all. And
     * the admin mutex is enough here to ensure the cache is still alive.
     */
    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    updatesEnabled = _this->updatesEnabled;

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
    return updatesEnabled;
}

DLRL_LS_object
DK_CacheAdmin_ts_getLSCache(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_LS_object retVal = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAdmin_lockAdministrative(_this);
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
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Unable to complete operation, out of resources");
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

DLRL_LS_object
DK_CacheAdmin_ts_getLSParticipant(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_LS_object retVal = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->ls_participant)
    {
        retVal = utilityBridge.localDuplicateLSInterfaceObject(
            userData,
            _this->ls_participant);
        if(!retVal)
        {
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Unable to complete operation, out of resources");
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

DLRL_LS_object
DK_CacheAdmin_ts_getLSPublisher(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_LS_object retVal = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->ls_publisher)
    {
        retVal = utilityBridge.localDuplicateLSInterfaceObject(
            userData,
            _this->ls_publisher);
        if(!retVal)
        {
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Unable to complete operation, out of resources");
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

DLRL_LS_object
DK_CacheAdmin_ts_getLSSubscriber(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_LS_object retVal = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if(_this->ls_subscriber)
    {
        retVal = utilityBridge.localDuplicateLSInterfaceObject(
            userData,
            _this->ls_subscriber);
        if(!retVal)
        {
            DLRL_Exception_THROW(
                exception,
                DLRL_OUT_OF_MEMORY,
                "Unable to complete operation, out of resources");
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

void
DK_CacheAdmin_ts_getLSHomes(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    cacheBridge.homesAction(exception, userData, &(_this->homes), arg);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_ts_getLSAccesses(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    cacheBridge.accessesAction(
        exception,
        userData,
        &(_this->cache_accesses),
        arg);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_ts_getListeners(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    cacheBridge.listenersAction(exception, userData, &(_this->listeners), arg);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_ts_enableUpdates(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_CacheAdmin_lockUpdates(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    if(_this->base.usage == DK_USAGE_WRITE_ONLY)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to enable updates %s '%s'. The cache is in WRITE_ONLY "
            "usage mode. %s",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name),
            "Updates can only be enabled when the usage is READ_ONLY or "
            "READ_WRITE.");
    }
    /* if usage is not write_only , then dispatcher always exists */
    assert(_this->dispatcher);

    if(!_this->updatesEnabled)
    {
        /* must clear all modification information */
        DK_CacheAdmin_us_resetObjectModificationInformation(
            _this,
            exception,
            userData);
        DLRL_Exception_PROPAGATE(exception);
        /* now continue with processing */
        _this->updatesEnabled = TRUE;
        if(Coll_Set_getNrOfElements(&(_this->listeners)) > 0 &&
            _this->updatesEnabled)
        {
            cacheBridge.triggerListenersWithUpdatesEnabled(
                exception,
                _this,
                userData);
            DLRL_Exception_PROPAGATE(exception);
        }
        if(_this->pub_sub_state == DK_PUB_SUB_STATE_ENABLED )
        {
            DK_EventDispatcher_us_activate(
                _this->dispatcher,
                exception,
                DK_CacheAdmin_us_activateAction);
            DLRL_Exception_PROPAGATE(exception);
        }
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockUpdates(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_ts_disableUpdates(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_CacheAdmin_lockUpdates(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    /* doesnt need to be inside mutex) */
    if(_this->base.usage == DK_USAGE_WRITE_ONLY)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to disable updates %s '%s'. The cache is in WRITE_ONLY "
            "usage mode. %s",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name),
            "Updates can only be disabled when the usage is READ_ONLY or "
            "READ_WRITE.");
    }
    /* always available when usage is not write only */
    assert(_this->dispatcher);
    if(_this->updatesEnabled)
    {
        if(_this->pub_sub_state == DK_PUB_SUB_STATE_ENABLED )
        {
            if(Coll_Set_getNrOfElements(&(_this->listeners)) > 0 &&
                _this->updatesEnabled)
            {
                cacheBridge.triggerListenersWithUpdatesDisabled(
                    exception,
                    _this,
                    userData);
                DLRL_Exception_PROPAGATE(exception);
            }
            /* deactivate dispatcher */
            DK_EventDispatcher_us_deactivate(
                _this->dispatcher,
                exception,
                DK_CacheAdmin_us_deactivateAction);
            DLRL_Exception_PROPAGATE(exception);
        }
        _this->updatesEnabled = FALSE;
    }
    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockUpdates(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_ts_refresh(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_CacheAdmin_lockUpdates(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if((_this->base.usage != DK_USAGE_READ_ONLY) &&
        (_this->base.usage != DK_USAGE_READ_WRITE))
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to load %s '%s'. The cache is configured as a WRITE_ONLY "
            "cache, to be able to process DCPS updates it should be configured "
            "as a READ_WRITE or READ_ONLY cache.",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name));
    }
    if(_this->pub_sub_state != DK_PUB_SUB_STATE_ENABLED)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to load %s '%s'. The cache must be in enabled pub/sub "
            "state.",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name));
    }
    if(!(_this->updatesEnabled))
    {
        /* TODO ID: 178 - turned off: StatusKindMask mask =
         * Subscriber_get_status_changes(_this->subscriber);
         */
        /* reset modification info */
        DK_CacheAdmin_us_resetObjectModificationInformation(
            _this,
            exception,
            userData);
        DLRL_Exception_PROPAGATE(exception);

        /* TODO ID: 178 - turned off if((mask & DATA_ON_READERS_STATUS) ==
         * DATA_ON_READERS_STATUS){
         */
        DK_CacheAdmin_us_processDCPSUpdates(_this, exception, userData);
        DLRL_Exception_PROPAGATE(exception);
        /* TODO ID: 178 - turned off} else do nothing */
    }/* else do nothing */

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockUpdates(_this);
    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_CacheAdmin_ts_attachListener(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const DLRL_LS_object listener)
{
    LOC_boolean alreadyExists = FALSE;
    LOC_boolean success = FALSE;
    Coll_Iter* iterator;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(listener);
    /* userData may be null */

    DK_CacheAdmin_lockUpdates(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    iterator = Coll_Set_getFirstElement(&(_this->listeners));
    while(iterator && !alreadyExists && listener)
    {
        DLRL_LS_object aListener;

        aListener = (DLRL_LS_object)Coll_Iter_getObject(iterator);
        if(utilityBridge.areSameLSObjects(userData, aListener, listener))
        {
            alreadyExists = TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    if(!alreadyExists && listener)
    {
        long errorCode = Coll_Set_add(&(_this->listeners), (void *)listener);
        if(errorCode != COLL_OK)
        {
            DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY,
                "Unable to add cachelistener %p to %s '%s'.",
                listener, ENTITY_NAME, DLRL_VALID_NAME(_this->name));
        }
        success = TRUE;
    }/* else do nothing (no op) */

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockUpdates(_this);
    DLRL_INFO(INF_EXIT);
    return success;
}

DLRL_LS_object
DK_CacheAdmin_ts_detachListener(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const DLRL_LS_object listener)
{
    DLRL_LS_object kernelListener = NULL;
    Coll_Iter* iterator = NULL;
    DLRL_LS_object aListener = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(listener);
    /* userData may be null */

    DK_CacheAdmin_lockUpdates(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }

    iterator = Coll_Set_getFirstElement(&(_this->listeners));
    while(iterator && !kernelListener && listener)
    {
        aListener = (DLRL_LS_object)Coll_Iter_getObject(iterator);
        if(utilityBridge.areSameLSObjects(userData, aListener, listener))
        {
            kernelListener = aListener;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    if(kernelListener)
    {
        Coll_Set_remove(&(_this->listeners), (void*)kernelListener);
    }

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockUpdates(_this);
    DLRL_INFO(INF_EXIT);
    return kernelListener;
}

void
DK_CacheAdmin_ts_getObjects(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg)
{
    DK_ObjectHomeAdmin* aHome = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectArrayHolder* holders = NULL;
    DK_ObjectArrayHolder* aHolder = NULL;
    LOC_unsigned_long count = 0;
    LOC_unsigned_long totalSize = 0;
    LOC_unsigned_long elementIndex = 0;
    LOC_unsigned_long nrOfHomes = 0;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    DK_CacheAdmin_lockUpdates(_this);
    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    nrOfHomes = Coll_List_getNrOfElements(&(_this->homes));
    DLRL_ALLOC_WITH_SIZE(
        holders,
        (sizeof(DK_ObjectArrayHolder)*nrOfHomes),
        exception,
        "Unable to allocate array container for object admin array holders!");
    memset(holders, 0, (sizeof(DK_ObjectArrayHolder)*nrOfHomes));
    iterator = Coll_List_getFirstElement(&(_this->homes));
    while(iterator)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        DK_ObjectHomeAdmin_lockUpdate(aHome);
        DK_ObjectHomeAdmin_lockAdmin(aHome);
        aHolder = &(holders[count]);
        if(exception->exceptionID == DLRL_NO_EXCEPTION)
        {
            DK_ObjectHomeAdmin_us_getAllObjects(aHome, exception, aHolder);
            totalSize = totalSize+aHolder->size;
            /* dont propagate, we need to lock all homes so we know what to
             * unlock :)
             */
        }
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }
    /* reset count to zero */
    count = 0;
    iterator = Coll_List_getFirstElement(&(_this->homes));
    while(iterator)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        /* call user provided action routine, only if there isnt an exception */
        aHolder = &(holders[count]);
        if(exception->exceptionID == DLRL_NO_EXCEPTION)
        {
            cacheBridge.objectsAction(
                exception,
                userData,
                arg,
                totalSize,
                &elementIndex,
                aHolder);
            /* do not propagate, may only be done after all homes are unlocked*/
        }
        if(aHolder->objectArray)
        {
            os_free(aHolder->objectArray);
        }
        DK_ObjectHomeAdmin_unlockAdmin(aHome);
        DK_ObjectHomeAdmin_unlockUpdate(aHome);
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }
    os_free(holders);
    /* now we can propagate the exception */
    DLRL_Exception_PROPAGATE(exception);
    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DK_CacheAdmin_unlockUpdates(_this);
    DLRL_INFO(INF_EXIT);
}

/* no homes may be locked! */
DK_CacheAccessAdmin*
DK_CacheAdmin_ts_createAccess(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_access,
    DK_Usage usage)
{
    LOC_long returnCode = COLL_OK;
    DK_CacheAccessAdmin* access = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* ls_access may be null */
    assert(usage < DK_Usage_elements);

    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    if (_this->pub_sub_state != DK_PUB_SUB_STATE_ENABLED)
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "The %s '%p' entity has not yet been enabled for all pubsub!",
            ENTITY_NAME,
            _this);
    }
    if(((_this->base.usage == DK_USAGE_READ_ONLY) &&
        (usage != DK_USAGE_READ_ONLY)) ||
        ((_this->base.usage == DK_USAGE_WRITE_ONLY) &&
        (usage != DK_USAGE_WRITE_ONLY)))
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "Unable to create a writeable cache access for the read only %s %s'",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name));
    }
    access = DK_CacheAccessAdmin_new(
        exception,
        userData,
        ls_access,
        usage,
        _this);
    DLRL_Exception_PROPAGATE(exception);

    /* takes over the ref count from the new */
    returnCode = Coll_Set_add(&(_this->cache_accesses), (void*)access);
    if(returnCode != COLL_OK)
    {
        DK_CacheAccessAdmin_us_delete(access, userData);
        DK_Entity_ts_release((DK_Entity*)access);
        access = NULL;
        DLRL_Exception_THROW(
            exception,
            DLRL_OUT_OF_MEMORY,
            "Unable to create access within %s '%s'. %s",
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name),
            "Allocation error when adding the access entity to the list of "
            "accesses within the cache.");
    }
    access = (DK_CacheAccessAdmin*)DK_Entity_ts_duplicate((DK_Entity*)access);
    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
    return access;
}

void
DK_CacheAdmin_ts_deleteAccess(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAccessAdmin* access)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */
    assert(access);

    DK_CacheAdmin_lockAdministrative(_this);
    if(!_this->base.alive)
    {
        DLRL_Exception_THROW(exception, DLRL_ALREADY_DELETED,
           "The %s '%p' entity has already been deleted!", ENTITY_NAME, _this);
    }
    /* if this cache access wasnt created by this cache, then we throw a
     * precondition not met.
     */
    if(!Coll_Set_contains(&(_this->cache_accesses), (void*)access))
    {
        DLRL_Exception_THROW(
            exception,
            DLRL_PRECONDITION_NOT_MET,
            "CacheAccess %p is not contained within the set of cache access of "
            "%s '%s'",
            access,
            ENTITY_NAME,
            DLRL_VALID_NAME(_this->name));
    }
    /* remove it */
    Coll_Set_remove(&(_this->cache_accesses), (void*)access);
    /* delete it */
    /* no homes may be locked! */
    DK_CacheAccessAdmin_ts_delete(access, userData);
    /* decrease the ref count! */
    DK_Entity_ts_release((DK_Entity*)access);

    DLRL_Exception_EXIT(exception);
    DK_CacheAdmin_unlockAdministrative(_this);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_lockAdministrative(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    os_mutexLock(&(_this->adminMutex));
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_unlockAdministrative(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    os_mutexUnlock(&(_this->adminMutex));
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_lockUpdates(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    os_mutexLock(&(_this->updateMutex));
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_unlockUpdates(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    os_mutexUnlock(&(_this->updateMutex));
    DLRL_INFO(INF_EXIT);
}

/* must lock and ensure the cache is still alive */
Coll_List*
DK_CacheAdmin_us_getHomes(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->homes);
}

Coll_Set*
DK_CacheAdmin_us_getAccesses(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->cache_accesses);
}

/* must lock and ensure the cache is still alive */
Coll_Set*
DK_CacheAdmin_us_getListeners(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return &(_this->listeners);
}

/* must lock and ensure the cache is still alive */
DLRL_LS_object
DK_CacheAdmin_us_getLSPublisher(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_publisher;
}

/* must lock and ensure the cache is still alive */
DLRL_LS_object
DK_CacheAdmin_us_getLSSubscriber(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_subscriber;
}

DLRL_LS_object
DK_CacheAdmin_us_getLSParticipant(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->ls_participant;
}

u_participant
DK_CacheAdmin_us_getParticipant(
    DK_CacheAdmin* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->participant;
}

void
DK_CacheAdmin_us_checkAlive(
    DK_CacheAdmin* _this,
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
DK_CacheAdmin_us_destroy(
    DK_Entity * _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */

    if(_this)
    {
        os_mutexDestroy (&(((DK_CacheAdmin*)_this)->updateMutex));
        os_mutexDestroy (&(((DK_CacheAdmin*)_this)->adminMutex));
        DLRL_INFO(INF_ENTITY, "destroyed %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_CacheAdmin*)_this);
    }

    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_us_delete(
    DK_CacheAdmin* _this,
    void* userData)
{
    Coll_Iter* iterator = NULL;
    DK_ObjectHomeAdmin* aHome = NULL;
    DLRL_Exception exception;
    DK_CacheAccessAdmin* aCacheAccess = NULL;
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    /* userData may be null */

    DLRL_Exception_init(&exception);

    /* NOTE: Always delete the cache access BEFORE the ObjectHomes as the
     * CacheAccess protects an array of not reference counted ObjectHomes
     * maintained by the home. See the Home header file (struct definition)
     * for more details.
     */
    iterator = Coll_Set_getFirstElement(&(_this->cache_accesses));
    while(iterator)
    {
        aCacheAccess = (DK_CacheAccessAdmin*)Coll_Iter_getObject(iterator);
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&(_this->cache_accesses), (void*)aCacheAccess);
        /* no homes may be locked! */
        DK_CacheAccessAdmin_ts_delete(aCacheAccess, userData);
        DK_Entity_ts_release((DK_Entity*)aCacheAccess);
    }

    iterator = Coll_Set_getFirstElement(&(_this->listeners));
    while(iterator)
    {
        void* aListener = Coll_Iter_getObject(iterator);
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(&(_this->listeners), aListener);
        utilityBridge.releaseLSInterfaceObject(userData, aListener);
        /* no kernel object exists for this listener so dont have to release it */
    }

    /* TODO ID: 181 - workaround code for CR 181 */
    /* lock all homes */
    iterator = Coll_List_getFirstElement(&(_this->homes));
    while(iterator)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        DK_ObjectHomeAdmin_lockUpdate(aHome);
        DK_ObjectHomeAdmin_lockAdmin(aHome);
        assert(aHome->alive);
        iterator = Coll_Iter_getNext(iterator);
    }
    /* delete all object admins per home */
    iterator = Coll_List_getFirstElement(&(_this->homes));
    while(iterator)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        DK_ObjectHomeAdmin_us_deleteAllObjectAdmins(aHome, userData);
        /* delete the object admins */
        iterator = Coll_Iter_getNext(iterator);
    }
    /* unlock all homes */
    iterator = Coll_List_getFirstElement(&(_this->homes));
    while(iterator)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        DK_ObjectHomeAdmin_unlockUpdate(aHome);
        DK_ObjectHomeAdmin_unlockAdmin(aHome);
        iterator = Coll_Iter_getNext(iterator);
    }

    /* delete homes AFTER the delete of the cache accesses and object admins */
    iterator = Coll_List_getFirstElement(&(_this->homes));
    while(iterator)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        DK_ObjectHomeAdmin_ts_delete(aHome, userData);
        DK_Entity_ts_release((DK_Entity*)aHome);
        iterator = Coll_Iter_getNext(iterator);
    }

    /* delete the pub/sub infra. Important that this happens after the delete
     * of the homes. As the homes have topic holders,which in turn have data
     * reader and data writer references!
     */
    if(_this->participant)
    {
        /* not owner of the participant, so dont have to delete it */
        if(_this->publisher)
        {
            cacheBridge.deletePublisher(
                &exception,
                userData,
                _this->participant,
                _this->ls_participant,
                _this->ls_publisher);
            if(exception.exceptionID != DLRL_NO_EXCEPTION)
            {
                DLRL_REPORT(
                    REPORT_ERROR,
                    "Exception %s occured when attempting to delete the DCPS "
                    "publisher\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID),
                    exception.exceptionMessage);
                /* reset the exception, maybe it's used again later in this
                 * deletion function. We dont propagate the exception here
                 * anyway, so it can do no harm as we already logged the
                 * exception directly above.
                 */
                DLRL_Exception_init(&exception);
            }
            _this->ls_publisher = NULL;
#if 1
            result = u_entityFree(u_entity(_this->publisher));
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(
                   &exception,
                   result,
                   "Unable to free the user layer publisher!");
               DLRL_REPORT(
                   REPORT_ERROR,
                   "Exception %s occured when attempting to delete the DCPS "
                        "user layer publisher\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID),
                   exception.exceptionMessage);
               DLRL_Exception_init(&exception);
            }
#endif
            _this->publisher = NULL;
        }
        if(_this->subscriber)
        {
            cacheBridge.deleteSubscriber(
                &exception,
                userData,
                _this->participant,
                _this->ls_participant,
                _this->ls_subscriber);
            if(exception.exceptionID != DLRL_NO_EXCEPTION)
            {
                DLRL_REPORT(
                    REPORT_ERROR,
                    "Exception %s occured when attempting to delete the DCPS "
                    "topic\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID),
                    exception.exceptionMessage);
                /* reset the exception, maybe it's used again later in this
                 * deletion function. We dont propagate the exception here
                 * anyway, so it can do no harm as we already logged the
                 * exception directly above.
                 */
                DLRL_Exception_init(&exception);
            }
            _this->ls_subscriber = NULL;
#if 1
            result = u_entityFree(u_entity(_this->subscriber));
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(
                   &exception,
                   result,
                   "Unable to free the user layer subscriber!");
               DLRL_REPORT(
                   REPORT_ERROR,
                   "Exception %s occured when attempting to delete the DCPS "
                        "user layer subscriber\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID),
                   exception.exceptionMessage);
               DLRL_Exception_init(&exception);
            }
#endif
            _this->subscriber = NULL;
        }
    }
    if(_this->base.ls_cacheBase)
    {
        utilityBridge.releaseLSInterfaceObject(
            userData,
            _this->base.ls_cacheBase);
        _this->base.ls_cacheBase = NULL;
    }
    if(_this->name)
    {
        _this->name = NULL;
        /* not owner so dont have to free it */
    }

    if(_this->ls_participant)
    {
        utilityBridge.releaseLSInterfaceObject(userData, _this->ls_participant);
        _this->ls_participant = NULL;
#if 1
        result = u_entityFree(u_entity(_this->participant));
        if(result != U_RESULT_OK)
        {
           DLRL_Exception_transformResultToException(
               &exception,
               result,
               "Unable to free the user layer participant!");
           DLRL_REPORT(
               REPORT_ERROR,
               "Exception %s occured when attempting to delete the DCPS "
                    "user layer participant\n%s",
                DLRL_Exception_exceptionIDToString(exception.exceptionID),
               exception.exceptionMessage);
           DLRL_Exception_init(&exception);
        }
#endif
        _this->participant = NULL;
    }
    _this->base.alive = FALSE;

    DLRL_INFO(INF_EXIT);
}

/* doesnt require a lock on the home, since the cache is locked already and the
 * accessed operations (getting the name) can only become unsafe if a delete is
 * done, which requires the administration lock. This operation explicitly
 * doesnt lock any homes. This knowledge is used elsewhere where this operation
 * is called from within a home lock.
 */
DK_ObjectHomeAdmin*
DK_CacheAdmin_us_findHomeByName(
    DK_CacheAdmin* _this,
    LOC_const_string name)
{
    DK_ObjectHomeAdmin* tempHome = NULL;
    DK_ObjectHomeAdmin* aHome = NULL;
    LOC_boolean homeFound = FALSE;
    Coll_Iter* iterator = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(name);

    iterator = Coll_List_getFirstElement(&(_this->homes));
    while(iterator && (homeFound == FALSE))
    {
        tempHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        if(0 == strcmp(DK_ObjectHomeAdmin_us_getName(tempHome), name))
        {
            homeFound = TRUE;
            aHome = tempHome;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_INFO(INF_EXIT);
    return aHome;
}

/* the code turned off by comments need to be turned on once DCPS supports
 * coherent accesses. update mutex of the cache needs to be claimed for this
 * operation.
 */
void
DK_CacheAdmin_us_processDCPSUpdates(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    LOC_unsigned_long count = 0;
    LOC_unsigned_long listenerSize = 0;
    DK_ReadInfo* readInfoArray = NULL;
    LOC_unsigned_long readInfoArraySize = 0;
    DK_ReadInfo* readInfo = NULL;
    LOC_unsigned_long dataCount = 0;
    LOC_unsigned_long dataSize = 0;
    DK_ObjectHomeAdmin* aHome = NULL;
    Coll_Iter* iterator = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* this operation should only execute when the cache is actually enabled*/
    assert(_this->pub_sub_state == DK_PUB_SUB_STATE_ENABLED);
    /* userData may be null */

    /* trigger start of updates of all cache listeners.  During the callback of
     * the listeners executed by the application, the other operations also
     * protected by the update mutex can not be accessed. This would create a
     * deadlock.  These operations should be mentioned in the developers guide
     * of the DLRL.
     */
    listenerSize = Coll_Set_getNrOfElements(&(_this->listeners));
    if(listenerSize > 0 && _this->updatesEnabled)
    {
        cacheBridge.triggerListenersWithStartOfUpdates(exception, _this, userData);
        DLRL_Exception_PROPAGATE(exception);
    }

    /* first lock all homes, so that when we process update we can navigate to
     * related homes without fear of deadlocks. This is need because as a rule
     * we always lock object homes with the lowest index first. If we do not
     * first lock all object homes and for example we are processing object home
     * with index == 3 and we have a relation to an object managed by object
     * home with index == 2, then we have a problem. Imagine if another
     * application thread also wanted to do something with homes 2 and 3. It
     * would try to lock home 2 first, which would succeed and then try to lock
     * home 3, which wouldnt succeed as it was already locked. so while the
     * application thread waits for the lock on home 3 to become available the
     * update thread is waiting for the lock on home 2 which is claimed by the
     * application thread. AKA a deadlock. This is why we need to maintain a
     * correct locking sequence to prevent deadlocks within the DLRL. The only
     * way we can effectively do this while processing updates is by first
     * locking all homes in sequence and once updates have been processed we
     * can unlock them all. always start with the first object home in the list,
     * as it will have the lowest index
     */
    iterator = Coll_List_getFirstElement(&(_this->homes));
    while(iterator)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        DK_ObjectHomeAdmin_lockUpdate(aHome);
        DK_ObjectHomeAdmin_lockAdmin(aHome);
        DK_ObjectHomeAdmin_us_checkAlive(aHome, exception);
        /* dont propagate the exception or do anything, we will continue to lock
        * everything
        */
        iterator = Coll_Iter_getNext(iterator);
    }

    /* iterate through the object homes to collect object updates and update the
    * object (not the relations). Start with  the last registered object home to
    * ensure that child homes are always processed first. This is required to
    * correct identify the type of object when it's a part of an inheritance
    * structure. It's a requirement to register parent  homes before child homes
    */
    readInfoArraySize = (Coll_List_getNrOfElements(&(_this->homes)))*sizeof(DK_ReadInfo);
    DLRL_ALLOC_WITH_SIZE(
        readInfoArray,
        readInfoArraySize,
        exception,
        "%s '%s': Unable to allocate memory for the update data array holder. "
        "Out of resources.",
        ENTITY_NAME,
        _this->name);
    memset(readInfoArray, 0, readInfoArraySize);
    /* list is inited in the first loop! dont forget it! */
    count = 0;
    iterator = Coll_List_getLastElement(&(_this->homes));
    while(iterator && exception->exceptionID == DLRL_NO_EXCEPTION)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        readInfo = &readInfoArray[count];
        /* dont forget to init this list */
        Coll_List_init(&readInfo->dataSamples);
        DK_ObjectHomeAdmin_us_collectObjectUpdates(
            aHome,
            exception,
            userData,
            readInfo);
        /* cant propagate due to locks being in place...
         * DLRL_Exception_PROPAGATE(exception);
         */
        if(exception->exceptionID == DLRL_NO_EXCEPTION &&
            Coll_List_getNrOfElements(&readInfo->dataSamples) > 0)
        {
            DK_ObjectHomeAdmin_us_processObjectUpdates(
                aHome,
                exception,
                userData,
                readInfo);
            /* cant propagate due to locks being in place...
             * DLRL_Exception_PROPAGATE(exception);
             */
        }
        iterator = Coll_Iter_getPrev(iterator);
        count++;
    }

    count = 0;
    iterator = Coll_List_getLastElement(&(_this->homes));
    while(iterator && exception->exceptionID == DLRL_NO_EXCEPTION)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        readInfo = &readInfoArray[count];
        DK_ObjectHomeAdmin_us_processAllObjectRelationUpdates(
            aHome,
            exception,
            userData,
            readInfo);
        /* cant propagate due to locks being in place...
         * DLRL_Exception_PROPAGATE(exception);
         */
        iterator = Coll_Iter_getPrev(iterator);
        count++;
    }

    count = 0;
    iterator = Coll_List_getLastElement(&(_this->homes));
    while(iterator && exception->exceptionID == DLRL_NO_EXCEPTION)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        readInfo = &readInfoArray[count];
        DK_ObjectHomeAdmin_us_clearAllRelationsToDeletedObjects(
            aHome,
            exception,
            userData,
            readInfo);
        /* cant propagate due to locks being in place...
         * DLRL_Exception_PROPAGATE(exception);
         */
        dataSize = Coll_List_getNrOfElements(&readInfo->dataSamples);
        for(dataCount = 0; dataCount < dataSize; dataCount++)
        {
            os_free(Coll_List_popBack(&readInfo->dataSamples));
        }
        iterator = Coll_Iter_getPrev(iterator);
        count++;
    }
    os_free(readInfoArray);
    readInfoArray = NULL;
    /* just before we trigger the object listeners of the homes, we can unlock
     * the admin mutex already. so that DLRL application have access to nearly
     * all object home operations in the callbacks. we keep the update mutex
     * locked though as it protects the listeners set.
     */
    iterator = Coll_List_getFirstElement(&(_this->homes));
    /* always need to unlock, even if an exception occured. */
    while(iterator)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        DK_ObjectHomeAdmin_unlockAdmin(aHome);
        iterator = Coll_Iter_getNext(iterator);
    }

    /* iterate through the object homes to update selections. Start with the
     * last registered object home  selection updating not yet supported
     */
    iterator = Coll_List_getFirstElement(&(_this->homes));
    while(iterator && exception->exceptionID == DLRL_NO_EXCEPTION)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        /* need to relock admin 1 by 1 to ensure access is threadsafe. We are
         * also trying to execute the following step outside of a global
         * locking of the admin mutex of all homes. This would be in
         * conflict with the bounds of the 'check_object' operation that will
         * be called within the updateSelections operation... So you might be
         * thinking, what the frack? But as long as the locking design for the
         * DLRL is not changed, this is the way it is. Deal with it =)
         */
        DK_ObjectHomeAdmin_lockAdmin(aHome);
        DK_ObjectHomeAdmin_us_updateSelections(aHome, exception, userData);
        DK_ObjectHomeAdmin_unlockAdmin(aHome);
        /* cant propagate due to locks being in place...
         * DLRL_Exception_PROPAGATE(exception);
         */
        iterator = Coll_Iter_getNext(iterator);
    }

    /* iterate through the object homes to trigger object listeners.
     * Start with the last registered object home.
     */
    iterator = Coll_List_getLastElement(&(_this->homes));
    while(iterator && _this->updatesEnabled &&
        exception->exceptionID == DLRL_NO_EXCEPTION)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        DK_ObjectHomeAdmin_us_triggerListeners(aHome, exception, userData);
        /* cant propagate due to locks being in place...
         * DLRL_Exception_PROPAGATE(exception);
         */
        iterator = Coll_Iter_getPrev(iterator);
    }

    /* iterate through the object homes to trigger object listeners.
     * Start with the last registered object home.
     */
    iterator = Coll_List_getLastElement(&(_this->homes));
    while(iterator && _this->updatesEnabled &&
        exception->exceptionID == DLRL_NO_EXCEPTION)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        DK_ObjectHomeAdmin_us_triggerSelectionListeners(
            aHome,
            exception,
            userData);
        /* cant propagate due to locks being in place...
         * DLRL_Exception_PROPAGATE(exception);
         */
        iterator = Coll_Iter_getPrev(iterator);
    }

    /* after the triggering of the object listener we can unlock the update
     * mutex, we no longer need to keep it locked. unlocking it here gives the
     * DLRL application access to the full scope of object home operations
     * during the callback of the cache listener operation. Cant unlock during
     * the previous while loop of triggering the listeners as that only needs
     * to be done if updates are enabled.
     */
    iterator = Coll_List_getFirstElement(&(_this->homes));
    /* always need to unlock, even if an exception occured. */
    while(iterator)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        DK_ObjectHomeAdmin_unlockUpdate(aHome);
        iterator = Coll_Iter_getNext(iterator);
    }
    /* propagate exception after everything is unlocked! */
    DLRL_Exception_PROPAGATE(exception);

    if(listenerSize > 0 && _this->updatesEnabled)
    {
        cacheBridge.triggerListenersWithEndOfUpdates(
            exception,
            _this,
            userData);
        DLRL_Exception_PROPAGATE(exception);
    }

    if(_this->updatesEnabled)
    {
        DK_CacheAdmin_us_resetObjectModificationInformation(
            _this,
            exception,
            userData);
        DLRL_Exception_PROPAGATE(exception);
    }

/*   returnCode = Subscriber_end_access (_this->subscriber); */
/*   if(returnCode != LOC_RETCODE_OK)
    {*/
/*         DLRL_Exception_THROW(exception, DLRL_DCPS_ERROR, */
/*             "Unable to complete processing of DCPS updates for %s '%s'. %s "
                "Check DCPS error log file for (possibly) more information.", */
/*             ENTITY_NAME, DLRL_VALID_NAME(_this->name), */
/*                 "The DLRL could not end coherent access on DCPS."); */
/*   } */
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_us_resetObjectModificationInformation(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData)
{
    Coll_Iter* iterator = NULL;
    DK_ObjectHomeAdmin* aHome = NULL;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */

    /* we have to lock all homes again, both mutex so we can clear the
     * modification information and delete any  object admins that are marked
     * for deletion
     */
    if(_this->pub_sub_state == DK_PUB_SUB_STATE_ENABLED)
    {
        iterator = Coll_List_getFirstElement(&(_this->homes));
        while(iterator)
        {
            aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
            DK_ObjectHomeAdmin_lockUpdate(aHome);
            DK_ObjectHomeAdmin_lockAdmin(aHome);
            DK_ObjectHomeAdmin_us_checkAlive(aHome, exception);
            /* dont propagate the exception or do anything, we will continue to
            * lock everything
            */
            iterator = Coll_Iter_getNext(iterator);
        }

        iterator = Coll_List_getFirstElement(&(_this->homes));
        while(iterator && (exception->exceptionID == DLRL_NO_EXCEPTION))
        {
            aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
            DK_ObjectHomeAdmin_us_resetObjectModificationInformation(
                aHome,
                exception,
                userData);
            iterator = Coll_Iter_getNext(iterator);
        }
        iterator = Coll_List_getFirstElement(&(_this->homes));
        while(iterator)
        {/* always need to unlock, even if an exception occured. */
            aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
            DK_ObjectHomeAdmin_unlockAdmin(aHome);
            DK_ObjectHomeAdmin_unlockUpdate(aHome);
            iterator = Coll_Iter_getNext(iterator);
        }
    }
    DLRL_Exception_PROPAGATE(exception);
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_us_createConnectivityEntities(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    u_participant participant,
    const DK_Usage usage)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* userData may be null */
    assert(participant);
    assert(usage < DK_Usage_elements);

    /* based upon the purpose of the cache create the neccesary publisher
     * and/or subscriber
     */
    switch (usage)
    {
        case DK_USAGE_READ_ONLY:
            DK_CacheAdmin_us_createSubscriber(
                _this,
                exception,
                userData,
                participant);
            DLRL_Exception_PROPAGATE(exception);
            break;
        case DK_USAGE_WRITE_ONLY:
            DK_CacheAdmin_us_createPublisher(
                _this,
                exception,
                userData,
                participant);
            DLRL_Exception_PROPAGATE(exception);
            break;
        case DK_USAGE_READ_WRITE:
            DK_CacheAdmin_us_createSubscriber(
                _this,
                exception,
                userData,
                participant);
            DLRL_Exception_PROPAGATE(exception);
            DK_CacheAdmin_us_createPublisher(
                _this,
                exception,
                userData,
                participant);
            DLRL_Exception_PROPAGATE(exception);
            break;
        default:
            assert(TRUE);
            break;
    }
    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_us_createPublisher(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    u_participant participant)
{
    u_publisher publisher = NULL;
    DLRL_LS_object ls_publisher = NULL;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(participant);
    /* userData may be null */

    publisher = cacheBridge.createPublisher(
        exception,
        userData,
        participant,
        _this->ls_participant,
        &ls_publisher);
    DLRL_Exception_PROPAGATE(exception);
    assert(publisher);

    _this->publisher = publisher;
    _this->ls_publisher = ls_publisher;

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_us_createSubscriber(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    u_participant participant)
{
    u_subscriber subscriber = NULL;
    DLRL_LS_object ls_subscriber = NULL;

    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(exception);
    assert(participant);
    /* userData may be null */

    subscriber =  cacheBridge.createSubscriber(
        exception,
        userData,
        participant,
        _this->ls_participant,
        &ls_subscriber);
    DLRL_Exception_PROPAGATE(exception);
    assert(subscriber);

    _this->subscriber = subscriber;
    _this->ls_subscriber = ls_subscriber;

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}


void
DK_CacheAdmin_us_handleAutoUpdateEvents(
    DK_Entity* _this,
    DLRL_Exception* exception,
    void* userData)
{
    DK_CacheAdmin* cache = (DK_CacheAdmin*)_this;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);
    /* user data ay be null */

    DK_CacheAdmin_lockUpdates(cache);
    if(DK_CacheAdmin_us_isAlive(cache))
    {
        /* Might be set to false by disableUpdates call which was sceduled in
         * between event received and event handled so ensure updates are still
         * enabled, if not the shit will hit the fan :)
         */
        if(cache->updatesEnabled)
        {
            /* temporary code to check for available data */
            if(cacheBridge.isDataAvailable(
                exception,
                userData,
                cache->ls_subscriber))
            {
                /* only continue if no exceptions occurred*/
                if(exception->exceptionID == DLRL_NO_EXCEPTION)
                {
                    DK_CacheAdmin_us_processDCPSUpdates(
                        cache,
                        exception,
                        userData);
                    /* delay propagate until unlock of mutex */
                }
            }
        }
    }
    DK_CacheAdmin_unlockUpdates(cache);/*unlock before propagate */
    DLRL_Exception_PROPAGATE(exception);/* propgate after unlock */

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_us_deactivateAction(
    DLRL_Exception* exception,
    u_waitset waitset,
    DK_Entity* interestedEntity)
{
    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(waitset);
    assert(interestedEntity);

    DK_CacheAdmin_us_waitsetActionGeneric(
        exception,
        waitset,
        interestedEntity,
        FALSE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_CacheAdmin_us_activateAction(
    DLRL_Exception* exception,
    u_waitset waitset,
    DK_Entity* interestedEntity)
{
    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(waitset);
    assert(interestedEntity);

    DK_CacheAdmin_us_waitsetActionGeneric(
        exception,
        waitset,
        interestedEntity,
        TRUE);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

/* very important:
 * I am iterating over my internal structure to find data readers, i am NOT
 * going to the subscriber and asking for all data readers for that subscriber
 * on purpose (not that such functionality is available atm, but ok). This
 * choice is made because a user of DLRL might have created it's own readers
 * with this subscriber and i do NOT want the waitset to attach/detach from
 * those readers as those readers are just not interesting to DLRL. So dont
 * think the code below can be optimized by simply asking the subscriber for
 * all readers.
 */
void
DK_CacheAdmin_us_waitsetActionGeneric(
    DLRL_Exception* exception,
    u_waitset waitset,
    DK_Entity* interestedEntity,
    LOC_boolean doAttach)
{
    Coll_List* homes;
    Coll_Iter* iterator;
    DK_ObjectHomeAdmin* aHome;
    DK_ObjectReader* objectReader;
    u_reader reader;
    u_result result;
    Coll_List* collectionReaders;
    Coll_Iter* collIterator;
    DK_CollectionReader* collReader;

    DLRL_INFO(INF_ENTER);

    assert(exception);
    assert(waitset);
    assert(interestedEntity);

    homes = DK_CacheAdmin_us_getHomes((DK_CacheAdmin*)interestedEntity);
    iterator = Coll_List_getFirstElement(homes);
    while(iterator)
    {
        aHome = (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
        objectReader = DK_ObjectHomeAdmin_us_getObjectReader(aHome);
        reader = DK_ObjectReader_us_getReader(objectReader);
        if(doAttach)
        {
            result = u_waitsetAttach(waitset, (u_entity)reader, NULL);
        } else
        {
            result = u_waitsetDetach(waitset, (u_entity)reader);
        }
        DLRL_Exception_PROPAGATE_RESULT(
            exception,
            result,
            "Unable to attach event handler waitset to datareader %p.",
            reader);
        collectionReaders = DK_ObjectReader_us_getCollectionReaders(objectReader);
        collIterator = Coll_List_getFirstElement(collectionReaders);
        while(collIterator)
        {
            collReader = (DK_CollectionReader*)Coll_Iter_getObject(collIterator);
            reader = DK_CollectionReader_us_getReader(collReader);
            if(doAttach)
            {
                result = u_waitsetAttach(waitset, (u_entity)reader, NULL);
            } else
            {
                result = u_waitsetDetach(waitset, (u_entity)reader);
            }
            DLRL_Exception_PROPAGATE_RESULT(
                exception,
                result,
                "Unable to attach event handler waitset to datareader %p.",
                reader);
            collIterator = Coll_Iter_getNext(collIterator);
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}
