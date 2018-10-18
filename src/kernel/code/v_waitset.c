/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "v__waitset.h"
#include "v__observer.h"
#include "v__subscriber.h"
#include "v__dataView.h"
#include "v__observable.h"
#include "v_public.h"
#include "v_proxy.h"
#include "v_event.h"
#include "v_query.h"
#include "v_group.h"
#include "v_statusCondition.h"
#include "v_dataReaderQuery.h"
#include "v_dataViewQuery.h"

#include "os_report.h"
#include "vortex_os.h"

#define WAITSET_BUSY_FLAG (0x80000000)

#define v_waitsetEventList(_this) \
        (_this->eventList)

static void
v_waitsetClearRemovedObserverPendingEvents(
    v_waitset _this,
    void* userDataRemoved);

static void
waitsetDetachAll(v_waitset _this)
{
    v_proxy proxy, removed;
    v_observable o;
    v_handleResult result;
    v_waitsetEvent event, eventList;
    void* userDataRemoved = NULL;

    /* disable interest to avoid arrival of new events. */
    OSPL_SET_EVENT_MASK(_this, 0);

    OSPL_LOCK(_this);

    /* remove and free all events */
    eventList = v_waitsetEvent(v_waitsetEventList(_this));
    v_waitsetEventList(_this) = NULL;
    while (eventList) {
        event = eventList;
        eventList = eventList->next;
        event->next = NULL;
        c_free(event);
    }
    proxy = _this->observables;
    _this->observables = NULL;
    /* wakeup blocking threads. */
    OSPL_TRIGGER_EVENT(_this, NULL, NULL);
    OSPL_UNLOCK(_this);

    while (proxy) {
        result = v_handleClaim(proxy->source,(v_object *)&o);
        if (result == V_HANDLE_OK) {
            (void)OSPL_REMOVE_OBSERVER(o,_this, V_EVENTMASK_ALL, &userDataRemoved);
            (void)v_handleRelease(proxy->source);
        }
        removed = proxy;
        proxy = proxy->next;
        removed->next = NULL;
        c_free(removed);
    }
}

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
        v_observerInit(v_observer(_this));
        _this->observables = NULL;
        _this->participant = p;
        _this->waitsetEventEnabled = TRUE;
        _this->count = 0;
        _this->waitDisconnectCount = 0;
        _this->eventList = NULL;
        c_condInit(c_getBase(_this), &_this->syncDisconnect, &v_observable(_this)->mutex);
        v_participantAdd(p, v_object(_this));
    }

    return _this;
}

void
v_waitsetFree(
   v_waitset _this)
{
    v_participant p;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    p = v_participant(_this->participant);
    if (p != NULL) {
        v_participantRemove(p, v_object(_this));
        _this->participant = NULL;
    }
    waitsetDetachAll(_this);
    v_observerFree(v_observer(_this));
}

void
v_waitsetDeinit(
   v_waitset _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));
    if (_this == NULL) {
        return;
    }

    v_observerDeinit(v_observer(_this));
}

void
v_waitsetLogEvents(
    v_waitset _this,
    c_bool enable)
{
    _this->waitsetEventEnabled = enable;
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
    v_handle sourceHandle;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));
    assert(e != NULL);

    EVENT_TRACE("v_waitsetNotify: _this(0x%x), event(0x%x), userData(0x%x)\n", _this, e, userData);

    if (_this->waitsetEventEnabled && e->source) {
        sourceHandle = v_publicHandle(v_public(e->source));

        /* Group events by origin of event */
        /* What about events while no threads are waiting?
         * It seems that the list of events can grow to infinite length.
         */
        found = v_waitsetEvent(v_waitsetEventList(_this));
        while ((found != NULL) &&
               (!v_handleIsEqual(found->source,sourceHandle)))
        {
            found = found->next;
        }
        if (found == NULL) {
            event = c_new(v_kernelType(v_objectKernel(_this),K_WAITSETEVENT));
            event->kind = e->kind;
            event->source = sourceHandle;
            event->userData = userData;
            event->eventData = c_keep(e->data);
            event->next = v_waitsetEvent(v_waitsetEventList(_this));
            v_waitsetEventList(_this) = (c_voidp)event;
            EVENT_TRACE("v_waitsetNotify: _this(0x%x), Event(0x%x) { kind(0x%x), source.index(%d), userData(0x%x) }\n",
                        _this, e, e->kind, sourceHandle.index, userData);
        } else {
            found->kind |= e->kind;
        }
    }
}

c_bool
on_data_available(
    c_object o,
    c_voidp arg)
{
    os_uint32 *events = (os_uint32 *)arg;
    OS_UNUSED_ARG(o);

    EVENT_TRACE("v_waitset::on_data_available userData(0x%x) }\n", o);
    *events |= V_EVENT_DATA_AVAILABLE;

    return OS_TRUE;
}

c_bool
test_condition (
    v_handle handle)
{
    os_uint32 events = 0;
    v_object condition = NULL;

    (void)v_handleClaim(handle,(v_object *)&condition);
    if (condition) {
        switch (v_objectKind(condition)) {
        case K_STATUSCONDITION:
            EVENT_TRACE("v_waitset::test_condition K_STATUSCONDITION(0x%x) }\n", condition);
            events = v_statusConditionGetTriggerValue(v_statusCondition(condition));
        break;
        case K_DATAREADERQUERY:
            EVENT_TRACE("v_waitset::test_condition K_DATAREADERQUERY:(0x%x) }\n", condition);
            (void)v_dataReaderQueryTest(v_dataReaderQuery(condition), on_data_available, &events);
        break;
        case K_DATAVIEWQUERY:
            EVENT_TRACE("v_waitset::test_condition K_DATAVIEWQUERY::(0x%x) }\n", condition);
            (void)v_dataViewQueryTest(v_dataViewQuery(condition), on_data_available, &events);
        break;
        default:
        break;
        }
        (void)v_handleRelease(handle);
    }
    return (events != 0);
}

#define INITIAL_BUFFER_SIZE (32)

v_result
v_waitsetWait2(
    v_waitset _this,
    v_waitsetAction2 action,
    c_voidp arg,
    const os_duration time)
{
    os_boolean proceed = OS_TRUE;
    v_result result = V_RESULT_OK;
    v_proxy initial[INITIAL_BUFFER_SIZE];
    v_proxy *buffer, proxy;
    c_long count;
    c_ulong length, i, bufferSize;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    /* Test trigger values of attached observables. */
    count = 0;

    OSPL_LOCK(_this);
    if (_this->count > INITIAL_BUFFER_SIZE) {
        /* Initial buffer size is insufficient so allocate a new one. */
        buffer = os_malloc(_this->count * sizeof(*buffer));
        bufferSize = _this->count;
    } else {
        buffer = initial;
        bufferSize = INITIAL_BUFFER_SIZE;
    }
    length = 0;
    proxy = _this->observables;
    while (proxy) {
        buffer[length++] = c_keep(proxy);
        proxy = proxy->next;
    }
    assert(length == _this->count);

    (void)v_observerGetEventFlags(v_observer(_this)); /* reset event flags */
    if (length > 0) {
        _this->waitDisconnectCount |= WAITSET_BUSY_FLAG;
    }
    OSPL_UNLOCK(_this);

    for (i=0; i<length; i++) {
        if (test_condition(buffer[i]->source)) {
            action(buffer[i]->userData, arg);
            count++;
        }
        c_free(buffer[i]);
    }
    proceed = action(NULL, arg); /* test guard conditions. */

    /* If none of the conditions evaluate true then block until triggered and then
     * reevaluate the conditions and callback on each true conditions.
     */

    if ((count == 0) && proceed) {
        if (OS_DURATION_ISZERO(time)) {
            result = V_RESULT_TIMEOUT;
        } else {
            c_ulong wait_flags;

            OSPL_LOCK(_this);
            _this->waitDisconnectCount &= ~WAITSET_BUSY_FLAG;
            if (_this->waitDisconnectCount > 0) {
                c_condBroadcast(&_this->syncDisconnect);
            }
            if (v_observerGetEventFlags(v_observer(_this)) == 0) {
                EVENT_TRACE("v_waitsetWait: Enter Timed Wait waitset(0x%x)\n", _this);
                wait_flags = OSPL_CATCH_EVENT(_this,time);
                OSPL_CLEAR_EVENT_FLAGS(_this);
                if (wait_flags & V_EVENT_OBJECT_DESTROYED) {
                    result = V_RESULT_DETACHING;
                } else if (wait_flags & V_EVENT_TIMEOUT) {
                    result = V_RESULT_TIMEOUT;
                }
            }
            if (result != V_RESULT_TIMEOUT) {
                if(_this->count > bufferSize){
                    if(bufferSize > INITIAL_BUFFER_SIZE){
                        os_free(buffer);
                    }
                    buffer = os_malloc(_this->count * sizeof(*buffer));
                    bufferSize = _this->count;
                }
                length = 0;
                proxy = _this->observables;
                while (proxy) {
                    buffer[length++] = c_keep(proxy);
                    proxy = proxy->next;
                }
                if (length > 0) {
                    _this->waitDisconnectCount |= WAITSET_BUSY_FLAG;
                }
            }
            OSPL_UNLOCK(_this);
            if (result != V_RESULT_TIMEOUT) {
                for (i=0; i<length; i++) {
                    if (test_condition(buffer[i]->source)) {
                        action(buffer[i]->userData, arg);
                        count++;
                    }
                    c_free(buffer[i]);
                }
                (void)action(NULL, arg); /* test guard conditions. */
            }
        }
    }

    OSPL_LOCK(_this);
    _this->waitDisconnectCount &= ~WAITSET_BUSY_FLAG;
    if (_this->waitDisconnectCount > 0) {
        c_condBroadcast(&_this->syncDisconnect);
    }
    OSPL_UNLOCK(_this);

    if (bufferSize > INITIAL_BUFFER_SIZE) {
        assert(buffer != initial);
        /* The initial static buffer is already replaced so free the previously allocated one. */
        os_free(buffer);
    }
    EVENT_TRACE("v_waitsetWait: Exit Timed Wait waitset(0x%x) result = %s\n", _this, v_resultImage(result));
    return result;
}

static c_bool
event_list_action(
    c_object o,
    c_voidp arg)
{
    v_waitset ws = v_waitset(o);
    v_waitsetEvent *list = (v_waitsetEvent *)arg;

    *list = v_waitsetEvent(v_waitsetEventList(ws));
    v_waitsetEventList(ws) = NULL;

    return TRUE;
}

v_result
v_waitsetWait(
    v_waitset _this,
    v_waitsetAction action,
    c_voidp arg,
    const os_duration time)
{
    v_waitsetEvent event;
    v_waitsetEvent eventList = NULL;
    v_result result = V_RESULT_OK;
    c_ulong wait_flags;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    wait_flags = 0;
    OSPL_LOCK(_this);

    EVENT_TRACE("v_waitsetWait: Enter Timed Wait waitset(0x%x)\n", _this);
    do {
        EVENT_TRACE("v_waitsetWait: -- waitset(0x%x) No events => block!\n", _this);
        wait_flags = OSPL_CATCH_EVENT_ACTION(_this, time, event_list_action, &eventList);
        EVENT_TRACE("v_waitsetWait: -- waitset(0x%x) Trigger! => unblock! result flags = 0x%x\n", _this, wait_flags);
    } while ((eventList == NULL) &&
             (!(wait_flags & (V_EVENT_OBJECT_DESTROYED | V_EVENT_TIMEOUT))));
    OSPL_UNLOCK(_this);

    if (wait_flags & V_EVENT_OBJECT_DESTROYED) {
        result = V_RESULT_DETACHING;
        c_free(eventList); /* Avoid potentially leaking events. */
    } else if (eventList) {
        event = eventList;
        while (event) {
            eventList = event->next;
            EVENT_TRACE("v_waitsetWait: waitset(0x%x), Event = "
                        "{ kind(0x%x), source index(%d), userData(0x%x) } action = 0x%x\n",
                        _this, event->kind, event->source.index, event->userData, action);
            if (action) {
                action(event, arg);
            }
            event->next = NULL; /* otherwise entire list is freed! */
            c_free(event);
            event = eventList;
        }
    } else if (wait_flags & V_EVENT_TIMEOUT) {
        result = V_RESULT_TIMEOUT;
    } else {
        assert(FALSE); /* Not expected. */
    }
    EVENT_TRACE("v_waitsetWait: Exit Timed Wait waitset(0x%x) result = %s\n",
                 _this, v_resultImage(result));
    return result;
}

static void
_waitsetTriggerEvent(
    v_waitset _this,
    v_event e,
    c_voidp eventArg)
{
    v_waitsetEvent event;

    OSPL_LOCK(_this);
    if (_this->waitsetEventEnabled) {
        event = c_new(v_kernelType(v_objectKernel(_this),K_WAITSETEVENT));
        event->kind = (e == NULL) ? V_EVENT_TRIGGER : e->kind;
        event->source = v_publicHandle(v_public(_this));
        event->userData = eventArg;
        event->eventData = NULL;
        event->next = v_waitsetEvent(v_waitsetEventList(_this));
        v_waitsetEventList(_this) = (c_voidp)event;
    }
    /* TODO: It would be more efficient only to trigger when waiting but this not always work
     * e.g. tc_waitset_stress test will crash. need to investigate.
     */
    EVENT_TRACE("v_waitsetTrigger: waitset(0x%x), userData(0x%x) }\n", _this, eventArg);
    OSPL_TRIGGER_EVENT(_this, e, NULL);
    OSPL_UNLOCK(_this);
}

void
v_waitsetTrigger(
    v_waitset _this,
    c_voidp eventArg)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    _waitsetTriggerEvent(_this, NULL, eventArg);
}

void
v_waitsetPrepareDestroy(
    v_waitset _this,
    c_voidp eventArg)
{
    C_STRUCT(v_event) e;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    e.kind = V_EVENT_OBJECT_DESTROYED;
    e.data = NULL;
    e.handled = FALSE;
    e.source = NULL;

    _waitsetTriggerEvent(_this, &e, eventArg);
}

c_bool
v_waitsetAttach (
    v_waitset _this,
    v_observable o,
    c_voidp userData)
{
    c_bool result;
    v_proxy proxy;
    v_handle handle;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    OSPL_LOCK(_this);
    handle = v_publicHandle(v_public(o));
    proxy = _this->observables;
    while (proxy) {
        if (v_handleIsEqual(proxy->source,handle)) break;
        proxy = proxy->next;
    }
    if (!proxy) {
        proxy = v_proxyNew(v_objectKernel(_this), handle, userData);
        proxy->next = _this->observables;
        _this->observables = proxy;
        _this->count++;
    }
    _this->waitDisconnectCount |= WAITSET_BUSY_FLAG;
    OSPL_UNLOCK(_this);
    {
        c_ulong mask = OSPL_GET_EVENT_MASK(_this);
        result = OSPL_ADD_OBSERVER(o,_this, mask, userData);
    }
    /* wakeup blocking threads to evaluate new condition. */
    if (test_condition(handle)) {
        v_waitsetTrigger(_this, NULL);
    }

    OSPL_LOCK(_this);
    _this->waitDisconnectCount &= ~WAITSET_BUSY_FLAG;
    if (_this->waitDisconnectCount > 0) {
        c_condBroadcast(&_this->syncDisconnect);
    }
    OSPL_UNLOCK(_this);

    return result;
}

c_long
v_waitsetDetach (
    v_waitset _this,
    v_observable o)
{
    c_bool removed;
    c_long result;
    v_proxy proxy, prev;
    v_handle handle;
    void* userDataRemoved = NULL;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    handle = v_publicHandle(v_public(o));
    OSPL_LOCK(_this);
    prev = NULL;
    proxy = _this->observables;
    while (proxy) {
        if (v_handleIsEqual(proxy->source,handle)) {
            if (prev) {
                prev->next = proxy->next;
            } else {
                _this->observables = proxy->next;
            }
            proxy->next = NULL;
            c_free(proxy);
            _this->count--;
            break;
        }
        prev = proxy;
        proxy = proxy->next;
    }
    _this->waitDisconnectCount++;
    while ((_this->waitDisconnectCount & WAITSET_BUSY_FLAG) == WAITSET_BUSY_FLAG) {
        c_condWait(&_this->syncDisconnect, &v_observable(_this)->mutex);
    }
    _this->waitDisconnectCount--;
    OSPL_UNLOCK(_this);
    removed = OSPL_REMOVE_OBSERVER(o,_this, V_EVENTMASK_ALL, &userDataRemoved);
    if (removed) {
        result = (c_long) _this->count;
    } else {
        result = -1;
    }
    v_waitsetClearRemovedObserverPendingEvents(_this, userDataRemoved);

    /* wakeup blocking threads to evaluate new condition. */
    v_waitsetTrigger(_this, NULL);
    return result;
}

void
v_waitsetClearRemovedObserverPendingEvents(
    v_waitset _this,
    void* userDataRemoved)
{
    /* check if any events for this removed observer are pending, if so they
     * must be trashed to avoid crashes, the memory might not be accessible after
     * this call has ended!
     */
    if(userDataRemoved)
    {
        v_waitsetEvent eventList;
        v_waitsetEvent event;
        v_waitsetEvent prevEvent = NULL;

        OSPL_LOCK(_this);
        EVENT_TRACE("v_waitsetClearRemovedObserverPendingEvents: waitset(0x%x), userData(0x%x)\n",
                    _this, userDataRemoved);
        eventList = v_waitsetEvent(v_waitsetEventList(_this));
        event = eventList;
        while (event)
        {
            /* if the userdata of the removed observer matched the userdata of
             * the found event, then this event pertains to the observer we
             * just removed and we must undertake action!
             */
            if (userDataRemoved == event->userData) {
                /* We need to remove the event from the linked list, to accomplish
                 * this we need to link the previous event (if any) to the next
                 * event, which basically cuts out the now invalid event from
                 * the event list. If the now invalid list was the head of the
                 * event list, then make the head of the event list equal to the
                 * next event
                 */
                if (prevEvent) {
                    prevEvent->next = event->next;
                } else {
                    v_waitsetEventList(_this) = event->next;
                }
                /* Free the memory of the event */
                event->next = NULL; /* otherwise entire list is freed! */
                c_free(event);
                /* set the event pointer to the correct next event pointer */
                if (prevEvent) {
                    event = prevEvent->next;
                } else {
                    event = v_waitsetEventList(_this);
                }
            } else {
                prevEvent = event;
                event = event->next;
            }
        }
        OSPL_UNLOCK(_this);
    }
}

c_ulong
v_waitsetCount(
    v_waitset _this)
{
    c_ulong count;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_waitset));

    OSPL_LOCK(_this);
    count = _this->count;
    OSPL_UNLOCK(_this);

    return count;
}
