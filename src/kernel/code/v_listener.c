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
#include "v__listener.h"
#include "v_public.h"
#include "v__status.h"
#include "v_event.h"
#include "v__kernel.h"

#include "os_report.h"
#include "os_heap.h"

#if 0
#define _TRACE_EVENTS_ printf
#else
#define _TRACE_EVENTS_(...)
#endif

v_listener
v_listenerNew(
    v_participant p,
    c_bool combine)
{
    v_listener _this;
    v_kernel kernel;

    assert(C_TYPECHECK(p,v_participant));

    kernel = v_objectKernel(p);
    _this = v_listener(v_objectNew(kernel,K_LISTENER));
    if (_this != NULL) {
        v_publicInit(v_public(_this));
        (void)c_mutexInit(c_getBase(_this), &_this->mutex);
        c_condInit(c_getBase(_this), &_this->cv, &_this->mutex);
        _this->participant = p;
        _this->eventList = NULL;
        _this->lastEvent = NULL;
        v_participantAdd(p, v_object(_this));
        _this->terminate = FALSE;
        _this->waitCount = 0;
        _this->combine = combine;
    }

    return _this;
}

static void
v_listenerEventDeinit(v_listenerEvent event)
{
    c_free(event->eventData);
    event->eventData = NULL;
    event->next = NULL;
}

void
v_listenerFree(
   v_listener _this)
{
    v_participant p;
    v_listenerEvent event;
    os_duration delay;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_listener));
    p = v_participant(_this->participant);
    assert(p != NULL);

    c_mutexLock(&_this->mutex);

    /* wakeup blocking threads to evaluate new condition. */
    /* remove all events */
    while (_this->eventList != NULL) {
        event = _this->eventList;
        _this->eventList = event->next;
        v_listenerEventDeinit(event);
        c_free(event);
    }
    _this->eventList = NULL;
    c_free(_this->lastEvent);
    _this->lastEvent = NULL;
    _this->terminate = TRUE;
    c_condBroadcast(&_this->cv);
    c_mutexUnlock(&_this->mutex);

    delay = OS_DURATION_INIT(0, 1000);
    while (_this->waitCount > 0 && !p->processIsZombie) {
        ospl_os_sleep(delay);
    }

    v_participantRemove(p, v_object(_this));
    _this->participant = NULL;
    v_publicFree(v_public(_this));
}

void
v_listenerDeinit(
    v_listener _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_listener));
    if (_this == NULL) {
        return;
    }
    v_publicDeinit(v_public(_this));
}

void
v_listenerFlush(
    v_listener _this,
    v_eventMask events,
    c_voidp userData)
{
    v_listenerEvent event, *prev;

    if (_this == NULL) {
        return;
    }
    assert(C_TYPECHECK(_this,v_listener));
    c_mutexLock(&_this->mutex);
    /* wakeup blocking threads to evaluate new condition. */
    /* remove all events */
    prev = &_this->eventList;
    event = _this->eventList;
    while (event != NULL) {
        if (event->userData == userData) {
            event->kind &= ~events;
        }
        if (event->kind == 0) {
            if (event == _this->lastEvent) {
                _this->lastEvent = c_keep(_this->lastEvent->next);
                v_listenerEventDeinit(event);
                c_free(event);
            }
            *prev = event->next;
            v_listenerEventDeinit(event);
            c_free(event);
            event = *prev;
        } else {
            prev = &event->next;
            event = event->next;
        }
    }
    c_condBroadcast(&_this->cv);
    c_mutexUnlock(&_this->mutex);
}

void
v_listenerNotify(
    v_listener _this,
    v_event e,
    v_entity listener)
{
   /* The userData argument is typical the user layer entity to which
    * this listener is attached.
    * The userData is passed to the v_listenerWait operation.
    * The eventData is optional for passing additional data,
    * typically status information is passed.
    */
    v_listenerEvent event,found;
    v_status status;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_listener));

    c_mutexLock(&_this->mutex);
    if (e != NULL) {
        found = NULL;
        event = NULL;
        if (_this->combine &&
            ((e->kind & V_EVENT_DATA_AVAILABLE) || (e->kind & V_EVENT_ON_DATA_ON_READERS)) )
        {
            /* Optimization for data availability: look for pending event and combine
             * to avoid triggering after data is already read on behalf of the first
             * trigger.
             */
            c_voidp source = v_publicGetUserData(v_public(e->source));
            found = _this->eventList;
            while (found != NULL) {
                if ((found->source == source) &&
                    (found->kind == e->kind)) {
                    break;
                } else {
                    found = found->next;
                }
            }
        }
        if (found == NULL) {
            event = c_new(v_kernelType(v_objectKernel(_this),K_LISTENEREVENT));
            event->next = NULL;
            event->kind = e->kind;
            event->source = v_publicGetUserData(v_public(e->source));
            /* For all events except for data availability
             * copy the entity status to keep the actual value until the
             * data is processed. The aforementioned events do not affect
             * status, so there is no need to copy the value nor reset any
             * counters.
             */
            if (event->kind == V_EVENT_DATA_AVAILABLE) {
                c_free(event->eventData);
                event->eventData = c_keep(e->data);
            } else {
                status = v_entityStatus(v_entity(e->source));
                c_free(event->eventData);
                event->eventData = v_statusCopyOut(status);
                v_statusResetCounters(status, e->kind);
                c_free(status);
            }
        }
    } else {
        event = c_new(v_kernelType(v_objectKernel(_this),K_LISTENEREVENT));
        event->next = NULL;
        event->kind = V_EVENT_TRIGGER;
        event->source = NULL;
        event->eventData = NULL;
    }
    if (event) {
        /* insert constructed listener event in the listeners event list. */
        if (_this->lastEvent) {
            _this->lastEvent->next = c_keep(event);
            c_free(_this->lastEvent);
        } else {
            assert(_this->eventList == NULL);
            _this->eventList = c_keep(event);
        }
        _this->lastEvent = event /* transfer ref */;
        if (listener) {
            event->listenerData = listener->listenerData;
            event->userData = v_publicGetUserData(v_public(listener)); /* Typical the language binding entity */
        } else {
            event->listenerData = NULL;
            event->userData = NULL; /* Typical the language binding entity */
        }
    }
    _TRACE_EVENTS_("v_listenerNotify: listener 0x%x, events 0x%x\n", _this, (e?e->kind:V_EVENT_TRIGGER));
    c_condBroadcast(&_this->cv);
    c_mutexUnlock(&_this->mutex);
}

v_result
v_listenerWait(
    v_listener _this,
    v_listenerAction action,
    c_voidp arg,
    const os_duration time)
{
    v_listenerEvent event, eventList;
    v_result result = V_RESULT_OK;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_listener));

    _TRACE_EVENTS_("v_listenerWait: listener 0x%x\n", _this);

    c_mutexLock(&_this->mutex);
    _this->waitCount++;
    eventList = _this->eventList;

    if (_this->terminate == TRUE) {
        result = V_RESULT_ALREADY_DELETED;
    } else if (eventList == NULL) {
        _TRACE_EVENTS_("v_listenerWait: listener 0x%x, initial event list is empty so block\n", _this);
        result = v_condWait(&_this->cv, &_this->mutex, time);
        if (_this->terminate == TRUE) {
            result = V_RESULT_ALREADY_DELETED;
        }
        eventList = _this->eventList;
    }
    _this->eventList = NULL;
    c_free(_this->lastEvent);
    _this->lastEvent = NULL;
    _this->waitCount--;
    assert(_this->waitCount + 1 > _this->waitCount);
    c_mutexUnlock(&_this->mutex);

    if (result == V_RESULT_ALREADY_DELETED) {
        action = NULL;
    }
    if (eventList != NULL) {
        /* If we timed out but did find some events, we consider
         * it success, so that a timeout means no events were
         * processed and success means some events were processed.
         */
        result = V_RESULT_OK;
    } else {
        _TRACE_EVENTS_("v_listenerWait: listener 0x%x, event list is empty so timeout\n", _this);
    }
    event = eventList;
    while (event) {
        _TRACE_EVENTS_("v_listenerWait process event: "
                       "listener 0x%x, event 0x%x\n",
                       _this, event->kind);

        if (action && event->kind != V_EVENT_TRIGGER) {
            v_kernelProtectStrictReadOnlyEnter();
            action(event, arg);
            v_kernelProtectStrictReadOnlyExit();
        }

        /* If this event is a destroy event then we must remove all
         * events from the same source that have arrived after this event.
         * Generation of new event are disabled but there may still be
         * events in the event list.
         * So scan the rest of the list for any events.
         */
        if (event->kind & V_EVENT_OBJECT_DESTROYED) {
            v_listenerEvent prev = event;
            v_listenerEvent e = event->next;

            while (e) {
                if (e->source == event->source) {
                    prev->next = e->next;
                    v_listenerEventDeinit(e);
                    c_free(e);
                    e = prev->next;
                } else {
                    prev = e;
                    e = e->next;
                }
            }
            /* Now we are sure that no events exist anymore from this source. */
        }
        eventList = event->next;
        /* free the Status object previously allocated by v_listenerNotify. */
        v_listenerEventDeinit(event);
        c_free(event);
        event = eventList;
    }
    return result;
}

