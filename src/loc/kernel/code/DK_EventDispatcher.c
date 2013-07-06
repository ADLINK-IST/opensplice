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

/* DLRL util includes */
#include "DLRL_Report.h"
#include "DLRL_Util.h"

/* DLRL kernel includes */
#include "DK_EventDispatcher.h"
#include "DK_UtilityBridge.h"

/* kernel includes */
#include "v_event.h"

#define ENTITY_NAME "DLRL Kernel Event Dispatcher"
static LOC_string allocError = "Unable to allocate " ENTITY_NAME;

static void
DK_EventDispatcher_us_destroy(
    DK_Entity* _this);

static void*
DK_EventDispatcher_us_threadRoutine(
    void* userData);

static void
DK_EventDispatcher_us_spawnDispatchThread(
    DK_EventDispatcher* _this,
    DLRL_Exception* exception,
    void* userData);

DK_EventDispatcher*
DK_EventDispatcher_new(
    DLRL_Exception* exception,
    void* userData,
    DK_Entity* interestedEntity,
    DK_EventDispatcher_us_eventAction eventAction,
    u_participant participant)
{
    DK_EventDispatcher* _this = NULL;
    u_result result;

    DLRL_INFO(INF_ENTER);

    assert(exception);

    DLRL_ALLOC(_this, DK_EventDispatcher, exception,  "%s", allocError);
    DK_Entity_us_init(&(_this->entity), DK_CLASS_EVENT_DISPATCHER, DK_EventDispatcher_us_destroy);

    _this->waitset = NULL;
    _this->interestedEntity = DK_Entity_ts_duplicate(interestedEntity);
    _this->eventAction = eventAction;
    _this->terminate = FALSE;
    _this->userData = NULL;
    _this->threadId = OS_THREAD_ID_NONE;

    _this->waitset = u_waitsetNew(participant);
    DLRL_VERIFY_ALLOC(_this->waitset, exception, "Unable to complete operation. Out of resources.");

    result = u_waitsetSetEventMask(_this->waitset, V_EVENT_TRIGGER);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "Unable to set NIL mask on created event handler waitset.");
    DK_EventDispatcher_us_spawnDispatchThread(_this, exception, userData);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_INFO(INF_ENTITY, "created %s, address = %p", ENTITY_NAME, _this);

    DLRL_Exception_EXIT(exception);
    if((exception->exceptionID != DLRL_NO_EXCEPTION) && _this)
    {
        DK_Entity_ts_release((DK_Entity*)_this);/* ref count will reach 0 and the destroy function will be called */
        _this = NULL;
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

/* this function is called once the DK_WaitSet has a ref count of 0 */
void
DK_EventDispatcher_us_destroy(
    DK_Entity* _this)
{
    DLRL_Exception exception;
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);

    /* _this may be null */
    if(_this)
    {
        DLRL_Exception_init(&exception);
        /* delete the internal user layer waitset pointer */
        if(((DK_EventDispatcher*)_this)->interestedEntity)
        {
            DK_Entity_ts_release(((DK_EventDispatcher*)_this)->interestedEntity);
        }
        if(((DK_EventDispatcher*)_this)->waitset)
        {
            result = u_waitsetFree(((DK_EventDispatcher*)_this)->waitset);
            if(result != U_RESULT_OK)
            {
               DLRL_Exception_transformResultToException(&exception, result, "Unable to free wait set.");
               DLRL_REPORT(REPORT_ERROR, "Exception %s occured when attempting to delete u_waitset.\n%s",
                    DLRL_Exception_exceptionIDToString(exception.exceptionID), exception.exceptionMessage);
               /*reinit the exception, we may use it later on and we dont want wierd ass errors because someone forgets this
                * step*/
               DLRL_Exception_init(&exception);
            }
        }
        DLRL_INFO(INF_ENTITY, "deleted %s, address = %p", ENTITY_NAME, _this);
        os_free((DK_EventDispatcher*)_this);
    }
    DLRL_INFO(INF_EXIT);
}

void
DK_EventDispatcher_us_spawnDispatchThread(
    DK_EventDispatcher* _this,
    DLRL_Exception* exception,
    void* userData)
{
    void* threadUserData = NULL;
    os_result osResult;
    os_threadAttr osThreadAttr;

    DLRL_INFO_OBJECT(INF_ENTER);
    assert(_this);
    assert(exception);

    if(utilityBridge.getThreadCreateUserData)
    {
        utilityBridge.getThreadCreateUserData(exception, userData, &threadUserData);
        DLRL_Exception_PROPAGATE(exception);
    }
    osResult = os_threadAttrInit(&osThreadAttr);/* can only return success */
    assert(osResult == os_resultSuccess);
    _this->userData = threadUserData;
    osResult = os_threadCreate(&(_this->threadId), "DK_EventDispatcher", &osThreadAttr,DK_EventDispatcher_us_threadRoutine,
                                                                    (void*)DK_Entity_ts_duplicate((DK_Entity*)_this));
    if(osResult != os_resultSuccess)
    {
        DLRL_Exception_THROW(exception, DLRL_OUT_OF_MEMORY, "Unable to complete operation. Failed to create thread. "
            "Out of resources.");
    }

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void*
DK_EventDispatcher_us_threadRoutine(
    void* userData)
{
    DLRL_Exception exception;
    DLRL_Exception exception2;
    DK_EventDispatcher* dispatcher;
    u_waitset waitset;
    void* dispatchUserData;
    void* threadSessionUserData = NULL;
    c_iter events = NULL;
    u_waitsetEvent event;
    DK_Entity* interestedEntity;
    DK_EventDispatcher_us_eventAction callbackRoutine;

    DLRL_INFO(INF_ENTER);

    assert(userData);

    DLRL_Exception_init(&exception);
    DLRL_Exception_init(&exception2);

    dispatcher = (DK_EventDispatcher*)userData;
    /* following info is valid as long as we have a reference to the dispatcher */
    waitset = DK_EventDispatcher_us_getWaitSet(dispatcher);
    dispatchUserData = DK_EventDispatcher_us_getUserData(dispatcher);
    interestedEntity = DK_EventDispatcher_us_getInterestedEntity(dispatcher);
    callbackRoutine = DK_EventDispatcher_us_getEventAction(dispatcher);

    /* now lets kick some ass (i.e. the actual start of the operation) */
    if(utilityBridge.doThreadAttach)
    {
        utilityBridge.doThreadAttach(&exception, dispatchUserData);
    }
    DLRL_Exception_PROPAGATE(&exception);
    if(utilityBridge.getThreadSessionUserData)
    {
        threadSessionUserData = utilityBridge.getThreadSessionUserData();
    }
    /* continue as long as the dispatcher is not to terminate */
    while(!DK_EventDispatcher_us_terminate(dispatcher))
    {
        u_waitsetWaitEvents(waitset,&events);
        if(events)
        {/* ONLY if an error occured will this var be NULL! */
            event = u_waitsetEvent(c_iterTakeFirst(events));
            while(event)
            {
                /* we check for deleted events here, currently (21-05-2007) this wont ever happen, but in the future it
                 * might as the implementation of the waitset is a tad weird for the destroyed event
                 */
                if(event->events & V_EVENT_OBJECT_DESTROYED)
                {
                    DK_EventDispatcher_us_setTerminate(dispatcher);/* sets terminate to true */
                }
                u_waitsetEventFree(event);
                event = u_waitsetEvent(c_iterTakeFirst(events));
            }
            c_iterFree(events);
            if(!DK_EventDispatcher_us_terminate(dispatcher))
            {/* did we receive a delete event? */
                /* now process received updates */
                callbackRoutine(interestedEntity, &exception, threadSessionUserData);
                DLRL_Exception_PROPAGATE(&exception);
            }
        } else
        {
            /* waitset became corrupted, deleted or something, whatever it is it aint good :). Don't report anything,
             * cos deletion is a valid way to exit for this thread. In fact it is a mechanism we '(ab)use' to terminate
             * this thread when deleting the Cache owning the waitset.
             */
            DK_EventDispatcher_us_setTerminate(dispatcher);/* sets terminate to true */
        }
    }

    DLRL_Exception_EXIT(&exception);
    if(utilityBridge.doThreadDetach)
    {
        /* must always perform detach action for this thread */
        utilityBridge.doThreadDetach(&exception2, dispatchUserData);
    }
    /* must release the pointer to the dispatcher */
    DK_Entity_ts_release((DK_Entity*)dispatcher);
    /* if an exception occured, we need to report it to an error log. */
    if(exception.exceptionID != DLRL_NO_EXCEPTION)
    {
        DLRL_REPORT(REPORT_ERROR, "Exception %s occured in the context of the 'DLRL waitset' thread run\n%s",
            DLRL_Exception_exceptionIDToString(exception.exceptionID), exception.exceptionMessage);
    }
    if(exception2.exceptionID != DLRL_NO_EXCEPTION)
    {
        DLRL_REPORT(REPORT_ERROR, "Exception %s occured when performing detach actions for the 'DLRL waitset' thread.\n%s",
            DLRL_Exception_exceptionIDToString(exception2.exceptionID), exception2.exceptionMessage);
    }
    DLRL_INFO(INF_EXIT);
    return NULL;
}

u_waitset
DK_EventDispatcher_us_getWaitSet(
    DK_EventDispatcher* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->waitset;
}

void*
DK_EventDispatcher_us_getUserData(
    DK_EventDispatcher* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    DLRL_INFO(INF_EXIT);
    return _this->userData;
}

DK_Entity*
DK_EventDispatcher_us_getInterestedEntity(
    DK_EventDispatcher* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    DLRL_INFO(INF_EXIT);
    return _this->interestedEntity;
}

DK_EventDispatcher_us_eventAction
DK_EventDispatcher_us_getEventAction(
    DK_EventDispatcher* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    DLRL_INFO(INF_EXIT);
    return _this->eventAction;
}

void
DK_EventDispatcher_us_setTerminate(
    DK_EventDispatcher* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    _this->terminate = TRUE;

    DLRL_INFO(INF_EXIT);
}

LOC_boolean
DK_EventDispatcher_us_terminate(
    DK_EventDispatcher* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->terminate;
}

void
DK_EventDispatcher_us_activate(
    DK_EventDispatcher* _this,
    DLRL_Exception* exception,
    DK_EventDispatcher_us_waitsetAttachAction waitsetAttachAction)
{
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    /* First attach all entities from the waitset before the mask has been set. We attach them all here
     * because it's unneccesary to add the waitset to the entities observable list before. And we expect
     * activate/deactivate to not happen frequently at all. Most use cases only once.
     */
    waitsetAttachAction(exception, _this->waitset, _this->interestedEntity);
    DLRL_Exception_PROPAGATE(exception);
    result = u_waitsetSetEventMask(_this->waitset, V_EVENT_DATA_AVAILABLE|V_EVENT_TRIGGER);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "Unable to set NIL mask on created event handler waitset.");
    result = u_waitsetNotify(_this->waitset, NULL);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "Unable to trigger DLRL event handler waitset.");

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_EventDispatcher_us_deactivate(
    DK_EventDispatcher* _this,
    DLRL_Exception* exception,
    DK_EventDispatcher_us_waitsetDetachAction waitsetDetachAction)
{
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    /* always respond to a notify */
    result = u_waitsetSetEventMask(_this->waitset, V_EVENT_TRIGGER);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "Unable to reset mask on created event dispatcher waitset.");
    /* Now detach all entities from the waitset after the mask has been set. We detach them all here
     * because it's unneccesary to keep the waitset in the entities observable list. And we expect
     * activate/deactivate to not happen frequently at all. Most use cases only once.
     */
    waitsetDetachAction(exception, _this->waitset, _this->interestedEntity);
    DLRL_Exception_PROPAGATE(exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

void
DK_EventDispatcher_us_notify(
    DK_EventDispatcher* _this,
    DLRL_Exception* exception)
{
    u_result result;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(exception);

    result = u_waitsetNotify(_this->waitset, NULL);
    DLRL_Exception_PROPAGATE_RESULT(exception, result, "Unable to notify event dispatcher waitset.");

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

os_threadId
DK_EventDispatcher_us_getThreadId(
    DK_EventDispatcher* _this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->threadId;
}
