/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "d__waitset.h"
#include "d__subscriber.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__misc.h"
#include "d__thread.h"
#include "u_waitset.h"
#include "u_participant.h"
#include "u_entity.h"
#include "u_observable.h"
#include "v_event.h"
#include "v_observer.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_thread.h"
#include "os_report.h"

C_CLASS(d_waitsetHelper);

C_STRUCT(d_waitsetHelper) {
    d_waitset waitset;
    u_waitset userWaitset;
    d_waitsetEntity entity;
    os_threadId tid;
    c_bool terminate;
};

#define d_waitsetHelper(h) ((d_waitsetHelper)(h))

static void
resetFlags(
    v_public p, c_voidp arg)
{
    OS_UNUSED_ARG(arg);
    (void)v_observerClearEventFlags(v_observer(p));
}

static void
process_events(
    v_waitsetEvent e,
    void *arg)
{
    d_thread self = d_threadLookupSelf ();
    d_waitsetEntity we = (d_waitsetEntity)arg;

    d_threadAwake(self);
    we->action(we->object, e, we->usrData);
    d_threadAsleep(self, 1);
}

static void*
d_waitsetEventHandler(
    void* userData)
{
    d_thread self = d_threadLookupSelf ();
    d_subscriber subscriber;
    d_durability durability;
    d_admin admin;
    d_waitsetHelper helper;
    d_waitsetEntity we;
    u_result ur;
    os_duration timeout;

    helper = d_waitsetHelper(userData);
    subscriber = d_waitsetGetSubscriber(helper->waitset);
    admin = d_subscriberGetAdmin(subscriber);
    durability = d_adminGetDurability(admin);
    we = helper->entity;
    timeout = OS_DURATION_INIT(1,0); /* wake up every second to assert liveliness */
    ur = U_RESULT_OK;

    while ((helper->terminate == FALSE) && (ur == U_RESULT_OK)) {
        (void)u_observableAction(u_observable(we->object), resetFlags, NULL);
        /* Assert liveliness every 1 second while waiting for events */
        d_threadAsleep(self, (os_uint32) OS_DURATION_GET_SECONDS(timeout) + (OS_DURATION_GET_NANOSECONDS(timeout) > 0));
        ur = u_waitsetWaitAction(helper->userWaitset, process_events, we, timeout);
        if (ur == U_RESULT_TIMEOUT) {
            ur = U_RESULT_OK;
        }
        d_threadAwake(self);
    } /* while */
    if (ur != U_RESULT_OK) {
        d_printTimedEvent(durability, D_LEVEL_SEVERE, we->name,
            "Waitset no longer available (result: %s). Fatal error, terminating now...\n",
            u_resultImage(ur));
        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                    "Waitset no longer available (result: %s). Fatal error, terminating now...\n",
                    u_resultImage(ur));
        d_durabilityTerminate(durability, TRUE);
        d_printTimedEvent(durability, D_LEVEL_SEVERE, we->name,
            "Waitset no longer available (result: %s). Fatal error, terminating now...\n",
            u_resultImage(ur));
    }
    return NULL;
}


void
d_waitsetDeinit(
    d_waitset waitset)
{
    d_waitsetEntity we;
    d_waitsetHelper helper;

    assert(d_waitsetIsValid(waitset));

    waitset->terminate = TRUE;
    if (waitset->threads) {
        /* Stop all threads */
        helper = d_waitsetHelper(c_iterTakeFirst(waitset->threads));
        while (helper) {
            helper->terminate = TRUE;
            u_waitsetNotify(helper->userWaitset, NULL);
            d_threadWaitExit(helper->tid, NULL);
            u_waitsetDetach(helper->userWaitset, u_observable(helper->entity->object));
            u_objectFree(u_object(helper->userWaitset));
            os_free(helper);
            helper = d_waitsetHelper(c_iterTakeFirst(waitset->threads));
        }
        c_iterFree(waitset->threads);
        waitset->threads = NULL;
    }
    /* Lock the waitset while removing entities */
    d_lockLock(d_lock(waitset));
    if (waitset->entities) {
        /*  Remove the entities associated with the waitset */
        we = d_waitsetEntity(c_iterTakeFirst(waitset->entities));
        while(we) {
            d_waitsetEntityFree(we);
            we = d_waitsetEntity(c_iterTakeFirst(waitset->entities));
        }
        c_iterFree(waitset->entities);
        waitset->entities = NULL;
    }
    /* Unlock the waitset */
    d_lockUnlock(d_lock(waitset));
    /* Call super-deinit */
    d_lockDeinit(d_lock(waitset));
}

void
d_waitsetFree(
    d_waitset waitset)
{
    assert(d_waitsetIsValid(waitset));

    d_objectFree(d_object(waitset));
}

d_waitset
d_waitsetNew(
    d_subscriber subscriber)
{
    d_waitset waitset = NULL;

    assert(d_subscriberIsValid(subscriber));

    if (subscriber) {
        /* Allocate waitset */
        waitset = d_waitset(os_malloc(C_SIZEOF(d_waitset)));
        if (waitset) {
            /* Call super-init */
            d_lockInit(d_lock(waitset), D_WAITSET,
                       (d_objectDeinitFunc)d_waitsetDeinit);
            /* Initialize waitset */
            waitset->terminate   = FALSE;
            waitset->subscriber  = subscriber;
            waitset->entities    = c_iterNew(NULL);
            waitset->threads     = c_iterNew(NULL);
        }
    }
    return waitset;
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

    assert(d_waitsetIsValid(waitset));
    assert(d_waitsetEntityIsValid(we));

    if (waitset && we) {
        d_lockLock(d_lock(waitset));
        if (!we->waitset) {
            if (c_iterContains(waitset->entities, we) == FALSE) {
                waitset->entities = c_iterInsert(waitset->entities, we);

                helper = os_malloc(C_SIZEOF(d_waitsetHelper));
                helper->waitset     = waitset;
                helper->entity      = we;
                helper->terminate   = FALSE;
                helper->tid         = OS_THREAD_ID_NONE;
                helper->userWaitset = u_waitsetNew();
                u_waitsetSetEventMask(helper->userWaitset, we->mask);
                ur = u_waitsetAttach(helper->userWaitset,
                                     u_observable(we->object),
                                     (c_voidp)we->object);
                if (ur == U_RESULT_OK) {
                    waitset->threads  = c_iterInsert(waitset->threads, helper);
                    /* Create a thread for the waitset */
                    osr = d_threadCreate(&(helper->tid),
                                         we->name,
                                         &(we->attr),
                                         d_waitsetEventHandler, helper);

                    if (osr == os_resultSuccess) {
                        result = TRUE;
                    } else {
                        c_iterTake(waitset->threads, helper);
                        u_waitsetDetach(helper->userWaitset, u_observable(we->object));
                        u_objectFree(u_object(helper->userWaitset));
                        os_free(helper);
                    }
                } else {
                    assert(FALSE);
                    u_objectFree(u_object(helper->userWaitset));
                    os_free(helper);
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
    d_waitsetEntity found;
    u_result ur;
    c_bool result = FALSE;
    d_waitsetHelper helper;
    u_eventMask mask;
    c_ulong i;

    assert(d_waitsetIsValid(waitset));
    assert(d_waitsetEntityIsValid(we));

    if (waitset && we) {
        d_lockLock(d_lock(waitset));
        found = c_iterTake(waitset->entities, we);
        if (found == we) {
            helper = NULL;
            for (i=0; i<c_iterLength(waitset->threads) && !helper; i++) {
                helper = d_waitsetHelper(c_iterObject(waitset->threads, i));
                if (helper->entity != we) {
                    helper = NULL;
                }
            }
            assert(helper);
            c_iterTake(waitset->threads, helper);
            helper->terminate = TRUE;
            /* Be sure that the helper reacts on our notification. */
            (void)u_waitsetGetEventMask(helper->userWaitset, &mask);
            mask |= V_EVENT_TRIGGER;
            (void)u_waitsetSetEventMask(helper->userWaitset, mask);
            /* Now, wake up the helper thread. */
            u_waitsetNotify(helper->userWaitset, NULL);
            d_threadWaitExit(helper->tid, NULL);
            ur = u_waitsetDetach_s(helper->userWaitset, u_observable(we->object));
            u_objectFree(u_object (helper->userWaitset));
            os_free(helper);
            we->waitset = NULL;
            if (ur == U_RESULT_OK || ur == U_RESULT_OUT_OF_MEMORY) {
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

    assert(d_waitsetIsValid(waitset));

    if (waitset) {
        subscriber = waitset->subscriber;
    }
    return subscriber;
}

d_waitsetEntity
d_waitsetEntityNew(
    const c_char* name,
    u_object object,
    d_waitsetAction action,
    c_ulong mask,
    os_threadAttr attr,
    c_voidp usrData)
{
    d_waitsetEntity we = NULL;

    assert(object);
    assert(action);

    if (object && action) {
        /* Allocate waitsetEntity object */
        we = d_waitsetEntity(os_malloc(C_SIZEOF(d_waitsetEntity)));
        if (we) {
            /* Call super-init */
            d_objectInit(d_object(we), D_WAITSET_ENTITY,
                         (d_objectDeinitFunc)d_waitsetEntityDeinit);
            /* Initialize waitsetEntity */
            if (name) {
                we->name   = os_strdup(name);
            } else {
                we->name   = os_strdup("waitsetEntity");
            }
            we->action     = action;
            we->object     = object;
            we->mask       = mask;
            we->usrData    = usrData;
            we->attr       = attr;
            we->waitset    = NULL;
        }
    }
    return we;
}

void
d_waitsetEntityDeinit(
    d_waitsetEntity we)
{
    assert(d_waitsetEntityIsValid(we));

    if (we->name) {
        os_free(we->name);
    }
    if (we->waitset) {
        d_waitsetDetach(we->waitset, we);
    }
    /* Call super-deinit */
    d_objectDeinit(d_object(we));
}

void
d_waitsetEntityFree(
    d_waitsetEntity we)
{
    assert(d_waitsetEntityIsValid(we));

    d_objectFree(d_object(we));
}
