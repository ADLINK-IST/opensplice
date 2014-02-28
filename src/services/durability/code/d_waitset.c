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
#include "d__waitset.h"
#include "d_waitset.h"
#include "d_subscriber.h"
#include "d_admin.h"
#include "d_durability.h"
#include "d_misc.h"
#include "u_waitset.h"
#include "u_waitsetEvent.h"
#include "u_participant.h"
#include "u_entity.h"
#include "v_event.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_thread.h"
#include "os_report.h"

#define RUN_TO_COMPLETION (0)
#define TIMED_WAIT        (0)

C_CLASS(d_waitsetHelper);

C_STRUCT(d_waitsetHelper) {
    d_waitset waitset;
    u_waitset userWaitset;
    d_waitsetEntity entity;
    os_threadId tid;
    c_bool terminate;
};

#define d_waitsetHelper(h) ((d_waitsetHelper)(h))

static void*
d_waitsetEventHandlerRunToCompletion(
    void* userData)
{
    d_subscriber subscriber;
    d_durability durability;
    d_admin admin;
    d_waitset waitset;
    c_iter events = NULL;
    d_waitsetEntity we;
    c_time time;
    u_waitsetEvent event;

    waitset          = d_waitset(userData);
    subscriber       = d_waitsetGetSubscriber(waitset);
    admin            = d_subscriberGetAdmin(subscriber);
    durability       = d_adminGetDurability(admin);
    time.seconds     = 1;
    time.nanoseconds = 0;

    while(waitset->terminate == FALSE) {
        if(waitset->timedWait == TRUE){
            u_waitsetTimedWaitEvents(waitset->uwaitset, time,&events);
        } else {
            u_waitsetWaitEvents(waitset->uwaitset,&events);
        }
        if(d_durabilityGetState(durability) != D_STATE_TERMINATING){
            if(waitset->terminate == FALSE){
                d_lockLock(d_lock(waitset));

                event = u_waitsetEvent(c_iterTakeFirst(events));

                while(event){
                    we = c_iterResolve(waitset->entities, (c_iterResolveCompare)d_waitsetEntityFind, event->entity);

                    if(we){
                        we->action(we->dispatcher, event, we->usrData);
                    }
                    u_waitsetEventFree(event);
                    event = u_waitsetEvent(c_iterTakeFirst(events));
                }
            }
            d_lockUnlock(d_lock(waitset));
        }
        if(events){/* events may be null if waitset was deleted */
            c_iterFree(events);
        }
    }
    return NULL;
}

static void*
d_waitsetEventHandler(
    void* userData)
{
    d_subscriber subscriber;
    d_durability durability;
    d_admin admin;
    c_iter events;
    d_waitsetEntity we;
    c_time time;
    d_waitsetHelper helper;
    u_waitset userWaitset;
    u_waitsetEvent event;
    u_result ur;

    helper           = d_waitsetHelper(userData);
    we               = helper->entity;
    subscriber       = d_waitsetGetSubscriber(helper->waitset);
    admin            = d_subscriberGetAdmin(subscriber);
    durability       = d_adminGetDurability(admin);
    time.seconds     = 1;
    time.nanoseconds = 0;
    userWaitset      = helper->userWaitset;
    ur               = U_RESULT_OK;

    d_printTimedEvent(durability, D_LEVEL_FINEST,
                          D_THREAD_UNSPECIFIED,
                          "DEBUG: waitset %s eventhandler\n",
                          we->name);

    while((helper->terminate == FALSE) && (ur == U_RESULT_OK)) {
        events = NULL;
        if(helper->waitset->timedWait == TRUE){
            ur = u_waitsetTimedWaitEvents(userWaitset, time,&events);
        } else {
            ur = u_waitsetWaitEvents(userWaitset,&events);
        }
        if(events  && (ur == U_RESULT_OK)){/* events may be null if waitset was deleted*/
            event = u_waitsetEvent(c_iterTakeFirst(events));

            while(event){
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                                      D_THREAD_UNSPECIFIED,
                                      "DEBUG: waitset %s triggered\n",
                                      we->name);

                /* Only dispatch event when durability is not terminating */
                if (d_durabilityGetState(durability) != D_STATE_TERMINATING){
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                                          D_THREAD_UNSPECIFIED,
                                          "DEBUG: waitset %s: calling action (dispatcher=%p)\n",
                                          we->name, we->dispatcher);
                    we->action(we->dispatcher, event, we->usrData);
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                                          D_THREAD_UNSPECIFIED,
                                          "DEBUG: waitset %s: action called\n",
                                          we->name);
                }
                u_waitsetEventFree(event);
                event = u_waitsetEvent(c_iterTakeFirst(events));
            }
            c_iterFree(events);
        }
    }

    d_printTimedEvent(durability, D_LEVEL_FINEST,
                          D_THREAD_UNSPECIFIED,
                          "DEBUG: waitset %s event handled\n",
                          we->name);

    if(ur != U_RESULT_OK){
        d_printTimedEvent(durability, D_LEVEL_SEVERE,
            we->name,
            "Waitset no longer available (result: %d). Fatal error, terminating now...\n",
            ur);
        OS_REPORT_1(OS_ERROR, D_CONTEXT_DURABILITY, 0,
            "Waitset no longer available (result: %d). Fatal error, terminating now...\n",
            ur);
        d_durabilityTerminate(durability, TRUE);
    }
    return NULL;
}

d_waitset
d_waitsetNew(
    d_subscriber subscriber,
    c_bool runToCompletion,
    c_bool timedWait)
{
    d_durability durability;
    d_admin admin;
    u_participant uparticipant;
    os_threadAttr attr;
    os_result osr;
    c_ulong mask;

    d_waitset waitset = NULL;

    assert(d_objectIsValid(d_object(subscriber), D_SUBSCRIBER) == TRUE);

    if(subscriber){
        waitset = d_waitset(os_malloc(C_SIZEOF(d_waitset)));

        if(waitset) {
            d_lockInit(d_lock(waitset), D_WAITSET, d_waitsetDeinit);
            admin                    = d_subscriberGetAdmin(subscriber);
            durability               = d_adminGetDurability(admin);
            uparticipant             = u_participant(d_durabilityGetService(durability));
            waitset->terminate       = FALSE;
            waitset->subscriber      = subscriber;
            waitset->entities        = c_iterNew(NULL);
            waitset->runToCompletion = runToCompletion;
            waitset->timedWait       = timedWait;

            if(runToCompletion == TRUE){
                waitset->uwaitset   = u_waitsetNew(uparticipant);
                mask = V_EVENT_DATA_AVAILABLE;
                mask |= V_EVENT_NEW_GROUP;
                mask |= V_EVENT_HISTORY_DELETE;
                mask |= V_EVENT_HISTORY_REQUEST;
                mask |= V_EVENT_PERSISTENT_SNAPSHOT;
                mask |= V_EVENT_TRIGGER;
                mask |= V_EVENT_CONNECT_WRITER;
                u_waitsetSetEventMask(waitset->uwaitset, mask);
                waitset->threads = NULL;

                osr = os_threadAttrInit(&attr);

                if(osr == os_resultSuccess) {
                    osr = os_threadCreate(&waitset->thread, "waitsetThread", &attr,
                                d_waitsetEventHandlerRunToCompletion, waitset);
                }

                if(osr != os_resultSuccess) {
                    d_waitsetFree(waitset);
                }
            } else {
                waitset->threads = c_iterNew(NULL);
                waitset->uwaitset = NULL;
                waitset->thread = OS_THREAD_ID_NONE;
            }
        }
    }
    return waitset;
}

void
d_waitsetDeinit(
    d_object object)
{
    d_waitset waitset;
    d_waitsetEntity we;
    d_waitsetHelper helper;

    assert(d_objectIsValid(object, D_WAITSET) == TRUE);

    if(object){
        waitset = d_waitset(object);
        waitset->terminate = TRUE;

        if(waitset->runToCompletion == TRUE){
            if(os_threadIdToInteger(waitset->thread)) {
                u_waitsetNotify(waitset->uwaitset, NULL);
                os_threadWaitExit(waitset->thread, NULL);
            }
        } else {
            if(waitset->threads){
                helper = d_waitsetHelper(c_iterTakeFirst(waitset->threads));

                while(helper){
                    helper->terminate = TRUE;
                    u_waitsetNotify(helper->userWaitset, NULL);
                    os_threadWaitExit(helper->tid, NULL);
                    u_waitsetDetach(helper->userWaitset, u_entity(helper->entity->dispatcher));
                    u_waitsetFree(helper->userWaitset);
                    os_free(helper);
                    helper = d_waitsetHelper(c_iterTakeFirst(waitset->threads));
                }
                c_iterFree(waitset->threads);
                waitset->threads = NULL;
            }
        }
        d_lockLock(d_lock(waitset));

        if(waitset->entities) {
            we = d_waitsetEntity(c_iterTakeFirst(waitset->entities));

            while(we) {
                if(waitset->runToCompletion == TRUE){
                    u_waitsetDetach(waitset->uwaitset, u_entity(we->dispatcher));
                }
                d_waitsetEntityFree(we);
                we = d_waitsetEntity(c_iterTakeFirst(waitset->entities));
            }
            c_iterFree(waitset->entities);
        }
        if(waitset->runToCompletion == TRUE){
            if(waitset->uwaitset) {
                u_waitsetFree(waitset->uwaitset);
            }
        }
        d_lockUnlock(d_lock(waitset));
    }
}

void
d_waitsetFree(
    d_waitset waitset)
{
    assert(d_objectIsValid(d_object(waitset), D_WAITSET) == TRUE);

    if(waitset){
        d_lockFree(d_lock(waitset), D_WAITSET);
    }
}

c_bool
d_waitsetAttach(
    d_waitset waitset,
    d_waitsetEntity we)
{
    c_bool result = FALSE;
    u_result ur;
    os_result osr;
    d_waitsetHelper helper;
    c_ulong mask;
    d_admin admin;
    d_durability durability;

    assert(d_objectIsValid(d_object(waitset), D_WAITSET) == TRUE);
    assert(d_objectIsValid(d_object(we), D_WAITSET_ENTITY) == TRUE);

    if(waitset && we){
        d_lockLock(d_lock(waitset));

        if(!we->waitset) {
            if(c_iterContains(waitset->entities, we) == FALSE) {
                waitset->entities = c_iterInsert(waitset->entities, we);
                if(waitset->runToCompletion == TRUE){
                    ur = u_waitsetAttach(waitset->uwaitset, u_entity(we->dispatcher),
                             (c_voidp)we->dispatcher);

                    if(ur == U_RESULT_OK) {
                        we->waitset = waitset;
                        result = TRUE;
                    }
                } else {
                    admin = d_subscriberGetAdmin(waitset->subscriber);
                    durability = d_adminGetDurability(admin);

                    helper = os_malloc(C_SIZEOF(d_waitsetHelper));
                    helper->waitset     = waitset;
                    helper->entity      = we;
                    helper->terminate   = FALSE;
                    helper->tid         = OS_THREAD_ID_NONE;
                    helper->userWaitset = u_waitsetNew(u_participant(d_durabilityGetService(durability)));

                    mask = V_EVENT_DATA_AVAILABLE;
                    mask |= V_EVENT_NEW_GROUP;
                    mask |= V_EVENT_HISTORY_DELETE;
                    mask |= V_EVENT_HISTORY_REQUEST;
                    mask |= V_EVENT_PERSISTENT_SNAPSHOT;
                    mask |= V_EVENT_TRIGGER;
                    mask |= V_EVENT_CONNECT_WRITER;
                    u_waitsetSetEventMask(helper->userWaitset, mask);
                    ur = u_waitsetAttach(helper->userWaitset, u_entity(we->dispatcher),
                             (c_voidp)we->dispatcher);

                    if(ur != U_RESULT_OK) {
                        assert(FALSE);
                    } else {
                        result = TRUE;
                    }
                    if(result){
                        waitset->threads  = c_iterInsert(waitset->threads, helper);

                        osr = os_threadCreate(&(helper->tid), we->name, &(we->attr),
                                    d_waitsetEventHandler, helper);

                        if(osr != os_resultSuccess){
                            c_iterTake(waitset->threads, helper);
                            u_waitsetDetach(helper->userWaitset, u_entity(we->dispatcher));
                            u_waitsetFree(helper->userWaitset);
                            os_free(helper);
                            result = FALSE;
                        }
                    } else {
                        u_waitsetFree(helper->userWaitset);
                        os_free(helper);
                    }

                }
            }
        }
        d_lockUnlock(d_lock(waitset));
    }
    return result;
}

c_bool
d_waitsetDetach(
    d_waitset waitset,
    d_waitsetEntity we)
{
    u_result ur;
    c_bool result = FALSE;
    int i;
    d_waitsetHelper helper;

    helper = NULL;

    assert(d_objectIsValid(d_object(waitset), D_WAITSET) == TRUE);
    assert(d_objectIsValid(d_object(we), D_WAITSET_ENTITY) == TRUE);

    if(waitset && we){
        d_lockLock(d_lock(waitset));

        if(c_iterContains(waitset->entities, we) == TRUE) {
            if(waitset->runToCompletion == TRUE){
                ur = u_waitsetDetach(waitset->uwaitset, u_entity(we->dispatcher));
            } else {
                for(i=0; i<c_iterLength(waitset->threads) && !helper; i++){
                    helper = d_waitsetHelper(c_iterObject(waitset->threads, i));

                    if(helper->entity != we){
                        helper = NULL;
                    }
                }
                assert(helper);
                c_iterTake(waitset->threads, helper);
                helper->terminate = TRUE;
                u_waitsetNotify(helper->userWaitset, NULL);
                os_threadWaitExit(helper->tid, NULL);
                ur = u_waitsetDetach(helper->userWaitset, u_entity(we->dispatcher));
                u_waitsetFree(helper->userWaitset);
                os_free(helper);
            }
            if(ur == U_RESULT_OK) {
                c_iterTake(waitset->entities, we);
                we->waitset = NULL;
                result = TRUE;
            }
        }
        d_lockUnlock(d_lock(waitset));
    }
    return result;
}

d_subscriber
d_waitsetGetSubscriber(
    d_waitset waitset)
{
    d_subscriber subscriber = NULL;

    assert(d_objectIsValid(d_object(waitset), D_WAITSET) == TRUE);

    if(waitset) {
        subscriber = waitset->subscriber;
    }
    return subscriber;
}

c_equality
d_waitsetEntityFind(
    d_waitsetEntity we,
    u_dispatcher entity)
{
    c_equality eq;

    if(we != NULL){
        if(we->dispatcher == entity) {
            eq = C_EQ;
        } else {
            eq = C_GT;
        }
    } else {
        eq = C_LT;
    }
    return eq;
}

d_waitsetEntity
d_waitsetEntityNew(
    const c_char* name,
    u_dispatcher dispatcher,
    d_waitsetAction action,
    c_ulong mask,
    os_threadAttr attr,
    c_voidp usrData)
{
    d_waitsetEntity we = NULL;

    assert(dispatcher);
    assert(action);

    if(dispatcher && action) {
        we = d_waitsetEntity(os_malloc(C_SIZEOF(d_waitsetEntity)));

        if(name){
            we->name   = os_strdup(name);
        } else {
            we->name   = os_strdup("waitsetEntity");
        }
        we->action     = action;
        we->dispatcher = dispatcher;
        we->mask       = mask;
        we->usrData    = usrData;
        we->attr       = attr;
        we->waitset    = NULL;

        d_objectInit(d_object(we), D_WAITSET_ENTITY, d_waitsetEntityDeinit);
    }
    return we;
}

void
d_waitsetEntityDeinit(
    d_object object)
{
    d_waitsetEntity we;

    assert(d_objectIsValid(object, D_WAITSET_ENTITY) == TRUE);

    if(object) {
        we = d_waitsetEntity(object);

        if(we->name){
            os_free(we->name);
        }
        if(we->waitset) {
            d_waitsetDetach(we->waitset, we);
        }
    }
}

void
d_waitsetEntityFree(
    d_waitsetEntity we)
{
    assert(d_objectIsValid(d_object(we), D_WAITSET_ENTITY) == TRUE);

    if(we) {
        d_objectFree(d_object(we), D_WAITSET_ENTITY);
    }
}
