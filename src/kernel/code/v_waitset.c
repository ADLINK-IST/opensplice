/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "v__waitset.h"
#include "v__observer.h"
#include "v_observable.h"
#include "v_public.h"
#include "v_proxy.h"
#include "v_event.h"
#include "v_query.h"

#include "os_report.h"

#define v_waitsetEvent(o) (C_CAST(o,v_waitsetEvent))
#define v_waitsetEventHistoryDelete(e)  (C_CAST(e,v_waitsetEventHistoryDelete))
#define v_waitsetEventHistoryRequest(e) (C_CAST(e, v_waitsetEventHistoryRequest))

#define v_waitsetEventList(_this) \
        (v_observer(_this)->eventData)

#define v_waitsetLock(_this) \
        v_observerLock(v_observer(_this))

#define v_waitsetUnlock(_this) \
        v_observerUnlock(v_observer(_this))

#define v_waitsetWakeup(_this,event,userData) \
        v_observerNotify(v_observer(_this),event,userData)

v_waitset
v_waitsetNew(
    v_participant p)
{
    v_waitset _this;
    v_kernel kernel;

    assert(C_TYPECHECK(p,v_participant));

    kernel = v_objectKernel(p);
    _this = v_waitset(v_objectNew(kernel,K_WAITSET));
    if (_this != NULL) {
        v_observerInit(v_observer(_this),"Waitset", NULL, TRUE);
        _this->participant = p;
        _this->eventCache = NULL;
        v_observerSetEventData(v_observer(_this), NULL);
        v_participantAdd(p, v_entity(_this));
    }
    return _this;
}

void
v_waitsetFree(
   v_waitset _this)
{
    v_kernel kernel;
    v_participant p;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    kernel = v_objectKernel(_this);
    p = v_participant(_this->participant);
    if (p != NULL) {
        v_participantRemove(p, v_entity(_this));
        _this->participant = NULL;
    }
    v_observerFree(v_observer(_this));
}

void
v_waitsetDeinit(
   v_waitset _this)
{
    v_waitsetEvent event;
    if (_this == NULL) {
        return;
    }
    assert(C_TYPECHECK(_this,v_waitset));

    v_waitsetLock(_this);
    /* remove all events */
    while (v_waitsetEvent(v_waitsetEventList(_this)) != NULL) {
        event = v_waitsetEvent(v_waitsetEventList(_this));
        v_waitsetEventList(_this) = event->next;
        event->next = NULL;
        c_free(event);
    }
    v_waitsetWakeup(_this, NULL, NULL);
    v_waitsetUnlock(_this);
    v_observerDeinit(v_observer(_this));
}

static v_waitsetEvent
v_waitsetEventNew(
    v_waitset _this)
{
    v_kernel k;
    v_waitsetEvent event;

    if (_this->eventCache) {
        event = _this->eventCache;
        _this->eventCache = event->next;
    } else {
        k = v_objectKernel(_this);
        event = c_new(v_kernelType(k,K_WAITSETEVENT));
    }
    return event;
}

static void
v_waitsetEventFree(
    v_waitset _this,
    v_waitsetEvent event)
{
//    v_waitsetLock(_this);

    if (event) {
        if (v_eventTest(event->kind,V_EVENT_HISTORY_DELETE)) {
            c_free(event);
        } else if(event->kind == V_EVENT_HISTORY_REQUEST) {
            c_free(event);
        } else {
            event->next = _this->eventCache;
            _this->eventCache = event;
        }
    }
//    v_waitsetUnlock(_this);
}

void
v_waitsetNotify(
    v_waitset _this,
    v_event e,
    c_voidp userData)
/* the userData argument is the data associated to the observable
 * for this waitset during the v_waitsetAttach call.
 * This data is unique for this association between this waitset and
 * the attached observable.
 */
{
    v_waitsetEvent event,found;
    c_base base;
    v_historyDeleteEventData hde;
    v_waitsetEventHistoryDelete wehd;
    v_waitsetEventHistoryRequest wehr;
    v_kernel k;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    if (e != NULL) {
        k = v_objectKernel(_this);
        if (e->kind == V_EVENT_HISTORY_DELETE) {
            /* delete historical data */
            wehd = c_new(v_kernelType(k,K_WAITSETEVENTHISTORYDELETE));
            base = c_getBase(c_object(_this));
            hde = (v_historyDeleteEventData)e->userData;

            wehd->deleteTime    = hde->deleteTime;
            wehd->partitionExpr = c_stringNew(base,hde->partitionExpression);
            wehd->topicExpr     = c_stringNew(base,hde->topicExpression);
            event = (v_waitsetEvent)wehd;
        } else if (e->kind == V_EVENT_HISTORY_REQUEST) {
            /* request historical data */
            wehr = c_new(v_kernelType(k, K_WAITSETEVENTHISTORYREQUEST));
            wehr->request = (v_historicalDataRequest)c_keep(e->userData);
            event = (v_waitsetEvent)wehr;
        } else {
            /* Group events by origin of event */
            /* What about events while no threads are waiting?
             * It seems that the list of events can grow to infinite length.
             */
            found = v_waitsetEvent(v_waitsetEventList(_this));
            while ((found != NULL) && (!v_handleIsEqual(found->source,e->source))){
                found = found->next;
            }
            if (found == NULL) {
                event = v_waitsetEventNew(_this);
            } else {
                found->kind |= e->kind;
                event = NULL;
            }
        }
        if (event) {
            event->source.server = e->source.server;
            event->source.index  = e->source.index;
            event->source.serial = e->source.serial;
            event->kind          = e->kind;
            event->userData      = userData;
            event->next = v_waitsetEvent(v_waitsetEventList(_this));
            v_waitsetEventList(_this) = (c_voidp)event;
        }
    }
}

v_result
v_waitsetWait(
    v_waitset _this,
    v_waitsetAction action,
    c_voidp arg)
{
    v_waitsetEvent event, eventList;
    v_result result;
    c_ulong wait_flags;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    wait_flags = 0;
    v_waitsetLock(_this);
    eventList = v_waitsetEvent(v_waitsetEventList(_this));

    while ((eventList == NULL) &&
           (!(wait_flags & V_EVENT_OBJECT_DESTROYED)))
    {
        wait_flags = v__observerWait(v_observer(_this));
        eventList = v_waitsetEvent(v_waitsetEventList(_this));
    }

    v__observerClearEventFlags(_this);
    v_waitsetEventList(_this) = NULL;
    if (action) {
        v_waitsetUnlock(_this);
        event = eventList;
        while (event) {
            action(event, arg);
            event = event->next;
        }
        v_waitsetLock(_this);
    }
    while (eventList) {
        event = eventList;
        eventList = eventList->next;
        event->next = NULL; /* otherwise entire list is freed! */
        v_waitsetEventFree(_this,event); /* free whole list. */
    }
    v_waitsetUnlock(_this);

    if (wait_flags & V_EVENT_OBJECT_DESTROYED) {
        result = V_RESULT_DETACHING;
    } else {
        result = V_RESULT_OK;
    }
    return result;
}

v_result
v_waitsetTimedWait(
    v_waitset _this,
    v_waitsetAction action,
    c_voidp arg,
    const c_time time)
{
    v_waitsetEvent event, eventList;
    v_result result;
    c_ulong wait_flags;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    wait_flags = 0;
    v_waitsetLock(_this);
    eventList = v_waitsetEvent(v_waitsetEventList(_this));

    while ((eventList == NULL) &&
           (!(wait_flags & (V_EVENT_OBJECT_DESTROYED | V_EVENT_TIMEOUT)))) {
        wait_flags = v__observerTimedWait(v_observer(_this),time);
        eventList = v_waitsetEvent(v_waitsetEventList(_this));
    }
    v__observerClearEventFlags(_this);
    v_waitsetEventList(_this) = NULL;
    if (action) {
        v_waitsetUnlock(_this);
        event = eventList;
        while (event) {
            action(event, arg);
            event = event->next;
        }
        v_waitsetLock(_this);
    }
    while (eventList) {
        event = eventList;
        eventList = eventList->next;
        event->next = NULL; /* otherwise entire list is freed! */
        v_waitsetEventFree(_this,event); /* free whole list. */
    }
    v_waitsetUnlock(_this);

    if (wait_flags & V_EVENT_OBJECT_DESTROYED) {
        result = V_RESULT_DETACHING;
    } else if (wait_flags & V_EVENT_TIMEOUT) {
        result = V_RESULT_TIMEOUT;
    } else {
        result = V_RESULT_OK;
    }
    return result;
}

void
v_waitsetTrigger(
    v_waitset _this,
    c_voidp eventArg)
{
    C_STRUCT(v_event) event;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    v_waitsetLock(_this);
    event.kind = V_EVENT_TRIGGER;
    event.source = v_publicHandle(v_public(_this));
    event.userData = NULL;
    v_waitsetWakeup(_this, &event, eventArg);
    v_waitsetUnlock(_this);
}

c_bool
v_waitsetAttach (
    v_waitset _this,
    v_observable o,
    c_voidp userData)
{
    c_bool result;
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));
    result = v_observableAddObserver(o,v_observer(_this), userData);
    /* wakeup blocking threads to evaluate new condition. */
    if (v_observerWaitCount(_this)) {
        v_waitsetTrigger(_this, NULL);
    }
    return result;
}

c_bool
v_waitsetDetach (
    v_waitset _this,
    v_observable o)
{
    c_bool result;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    result = v_observableRemoveObserver(o,v_observer(_this));
    /* wakeup blocking threads to evaluate new condition. */
    if (v_observerWaitCount(_this)) {
        v_waitsetTrigger(_this, NULL);
    }
    return result;
}
