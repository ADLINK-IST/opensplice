/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "v__observer.h"

#include "v__observable.h"
#include "v__participant.h"
#include "v__dataReader.h"
#include "v__groupStream.h"
#include "v__waitset.h"
#include "v__service.h"
#include "v__spliced.h"
#include "v__query.h"
#include "v__serviceManager.h"
#include "v__topic.h"
#include "v__topicAdapter.h"
#include "v__writer.h"
#include "v__processInfo.h"
#include "v__kernel.h"

#include "v_public.h"
#include "v_event.h"

#include "vortex_os.h"
#include "os_report.h"
#include "os_process.h"

void
v_observerInit(
    v_observer o)
{
    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    c_mutexInit(c_getBase(o), &o->mutex);  /* mutex to protect attributes */
    c_condInit(c_getBase(o), &o->cv, &o->mutex); /* condition variable */
    o->waitCount = 0;                     /* number of waiting threads */
    o->eventMask = 0;                     /* specifies, interested events */
    o->eventFlags = 0;                    /* ocurred events */
    o->eventData = NULL;                  /* general post box for derived classes */
    v_observableInit(v_observable(o));
}

void
v_observerFree(
    v_observer o)
{
    assert(C_TYPECHECK(o,v_observer));

    c_mutexLock(&o->mutex);
    o->eventFlags |= V_EVENT_OBJECT_DESTROYED;
    c_condBroadcast(&o->cv);
    c_mutexUnlock(&o->mutex);

    v_observableFree(v_observable(o));
}

void
v_observerDeinit(
    v_observer o)
{
    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    v_observableDeinit(v_observable(o));
}

c_ulong
v_observerGetEventFlags(
    v_observer o)
{
    c_ulong flags;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    flags = o->eventFlags;
    o->eventFlags &= V_EVENT_OBJECT_DESTROYED;

    EVENT_TRACE("v_observerGetEventFlags(_this = 0x%x, flags = 0x%x)\n",
                o, flags);

    return flags;
}

/**
 * PRE: v_observerLock has been called.
 *
 * When the text 'intentionally no break' is set after a case label
 * the class specified by the label has not implemented the notify
 * method.
 */
void
v_observerNotify(
    v_observer _this,
    v_event event,
    c_voidp userData)
{
    /* This Notify method is part of the observer-observable pattern.
     * It is designed to be invoked when _this object as observer receives
     * an event from an observable object.
     * It must be possible to pass the event to the subclass of itself by
     * calling <subclass>Notify(_this, event, userData).
     * This implies that _this cannot be locked within any Notify method
     * to avoid deadlocks.
     * For consistency _this must be locked by v_observerLock(_this) before
     * calling this method.
     */

    c_ulong trigger = 0;
    c_bool  notify  = TRUE;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_observer));

    /* The observer will be made insensitive to event as soon as the
     * observer is deinitialized. However it may be that destruction
     * of the observer has started before the deinit of the observer
     * is called. In that case the V_EVENT_OBJECT_DESTROYED flag will
     * be set to avoid processing of incomming events.
     */
    if ((_this->eventFlags & V_EVENT_OBJECT_DESTROYED) == 0) {
        /* The observer is valid so the event can be processed.
          */

        if (event != NULL ) {
            trigger = event->kind & _this->eventMask;
        } else {
            /* NULL-event always notifies observers */
            trigger = V_EVENT_TRIGGER;
        }

        EVENT_TRACE("v_observerNotify: %s(0x%x), event(0x%x), mask(0x%x), trigger(0x%x) userData(0x%x)\n",
                    v_objectKindImage(_this), _this, event, _this->eventMask, trigger, userData);

        if ((_this->eventFlags & trigger) && (trigger != V_EVENT_TRIGGER)) {
            notify = FALSE;
        }
        _this->eventFlags |= trigger; /* store event before trigger is given to waiting threads or subclasses are notified*/

        /* The following code invokes the observers subclass specific
         * notification method.
         * This is a bit strange that the observer has knowledge about
         * the subclass notification methods, a better model is that
         * subclasses register the notification method to the observer
         * instead. The reason for this model is that registering a
         * function pointer is only valid in the process scope and this
         * method will typically be called from another process.
         */
        if (trigger != 0) {
            switch (v_objectKind(_this)) {
            case K_DATAREADER:
                v_dataReaderNotify(v_dataReader(_this), event, userData);
            break;
            case K_STATUSCONDITION:
                v_observableNotify(v_observable(_this), event);
            break;
            case K_WAITSET:
                v_waitsetNotify(v_waitset(_this), event, userData);
            break;
            case K_PARTICIPANT:
                v_participantNotify(v_participant(_this), event, userData);
            break;
            case K_TOPIC:
                v_topicNotify(v_topic(_this), event, userData);
            break;
            case K_TOPIC_ADAPTER:
                v_topicAdapterNotify(v_topicAdapter(_this), event, userData);
            break;
            case K_QUERY:
                /* v_queryNotify(v_query(_this), event, userData); */
            break;
            case K_SPLICED: /* intentionally no break */
            case K_SERVICE:
            case K_NETWORKING:
            case K_DURABILITY:
            case K_NWBRIDGE:
            case K_CMSOAP:
            case K_RNR:
                v_serviceNotify(v_service(_this), event, userData);
            break;
            case K_SERVICEMANAGER:
                v_serviceManagerNotify(v_serviceManager(_this), event, userData);
            break;
            case K_WRITER: /* no action for the following observers */
            case K_PUBLISHER:
            case K_SUBSCRIBER:
            case K_GROUPQUEUE:
            case K_LISTENER:
            break;
            default:
                OS_REPORT(OS_ERROR,"Kernel Observer",V_RESULT_INTERNAL_ERROR,
                            "Notify called on an unknown observer type: %d",
                            v_objectKind(_this));
                assert(FALSE);
            break;
            }
            /*
            * Only trigger condition variable if at least
            * one thread is waiting AND the event is seen for the first time.
            */
            if ((_this->waitCount > 0) && notify)
            {
                EVENT_TRACE("v_observerNotify(%s(0x%x), event(0x%x)) => signal blocking threads\n",
                    v_objectKindImage(_this), _this, event);

                c_condBroadcast(&_this->cv);
            }
        }
    } else { /* else observer object destroyed, skip notification */
        EVENT_TRACE("v_observerNotify: Event(0x%x) is ignored for %s(0x%x) because it is currently being destroyed\n",
                     event, v_objectKindImage(_this), _this);
    }
}

c_ulong
v__observerWait(
    v_observer o)
{
    v_result result;
    c_ulong flags = 0;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    if (o->eventFlags == 0) {
        o->waitCount++;
        EVENT_TRACE("v__observerWait: Block on %s(0x%x), eventFlags(0x%x)\n",
                     v_objectKindImage(o), o, o->eventFlags);
        result = v_condWait(&o->cv,&o->mutex, OS_DURATION_INFINITE);
        if (result != V_RESULT_OK) {
            OS_REPORT(OS_CRITICAL,"v__observerWait",result,
                        "v_condWait failed with result = %d",
                        result);
        }
        flags = o->eventFlags;
        EVENT_TRACE("v__observerWait: WakeUp %s(0x%x), eventFlags(0x%x)\n",
                     v_objectKindImage(o), o, o->eventFlags);
        o->waitCount--;
    } else {
        flags = o->eventFlags;
        EVENT_TRACE("v__observerWait: Wait fallthrough for %s(0x%x), "
                    "because of pending eventFlags(0x%x)\n",
                     v_objectKindImage(o), o, o->eventFlags);
    }

    /* Reset events but remember destruction event.
     * To avoid any further use of this observer in case of destruction.
     */
    if (o->waitCount == 0)
    {
        o->eventFlags &= V_EVENT_OBJECT_DESTROYED;
    }
    return flags;
}

c_ulong
v_observerWait(
    v_observer o)
{
    c_ulong flags;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    c_mutexLock(&o->mutex);
    flags = v__observerWait(o);
    c_mutexUnlock(&o->mutex);

    return flags;
}

c_ulong
v__observerTimedWait(
    v_observer o,
    const os_duration time)
{
    v_result result = V_RESULT_OK;
    c_ulong flags = 0;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    if (o->eventFlags == 0) {
        o->waitCount++;
        EVENT_TRACE("v__observerTimedWait Block %s(0x%x) eventFlags(0x%x)\n",
                     v_objectKindImage(o), o, o->eventFlags);
        result = v_condWait(&o->cv,&o->mutex, time);
        o->waitCount--;
        if (result == V_RESULT_TIMEOUT) {
            o->eventFlags |= V_EVENT_TIMEOUT;
        }
        flags = o->eventFlags;
        EVENT_TRACE("v__observerTimedWait: WakeUp %s(0x%x) eventFlags(0x%x)\n",
                     v_objectKindImage(o), o, o->eventFlags);

    } else {
        EVENT_TRACE("v__observerTimedWait: Fallthrough %s(0x%x), eventFlags(0x%x)\n",
                     v_objectKindImage(o), o, o->eventFlags);
    }


    /* Reset events but remember destruction event.
     * To avoid any further use of this observer in case of destruction.
     */
    if (o->waitCount == 0) {
        o->eventFlags &= V_EVENT_OBJECT_DESTROYED;
    }
    return flags;
}

c_ulong
v_observerTimedWait(
    v_observer o,
    const os_duration time)
{
    c_ulong flags;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    c_mutexLock(&o->mutex);
    flags = v__observerTimedWait(o, time);
    c_mutexUnlock(&o->mutex);

    return flags;
}

c_ulong
v__observerSetEvent(
    v_observer o,
    c_ulong event)
{
    c_ulong eventMask;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    eventMask = o->eventMask;
    o->eventMask |= event;

    return eventMask;
}

c_ulong
v_observerSetEvent(
    v_observer o,
    c_ulong event)
{
    c_ulong eventMask;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    c_mutexLock(&o->mutex);
    eventMask = v__observerSetEvent(o, event);
    c_mutexUnlock(&o->mutex);

    return eventMask;
}

c_ulong
v_observerClearEvent(
    v_observer o,
    c_ulong event)
{
    c_ulong eventMask;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    c_mutexLock(&o->mutex);
    eventMask = o->eventMask;
    o->eventMask &= ~event;
    c_mutexUnlock(&o->mutex);

    return eventMask;
}

c_ulong
v_observerSetEventMask(
    v_observer o,
    c_ulong mask)
{
    c_ulong eventMask;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    c_mutexLock(&o->mutex);
    eventMask = o->eventMask;
    o->eventMask = mask;
    c_mutexUnlock(&o->mutex);

    return eventMask;
}

c_ulong
v_observerGetEventMask(
    v_observer o)
{
    c_ulong eventMask;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    c_mutexLock(&o->mutex);
    eventMask = o->eventMask;
    c_mutexUnlock(&o->mutex);

    return eventMask;
}

c_voidp
v_observerSetEventData(
    v_observer o,
    c_voidp eventData)
{
    c_voidp oldEventData;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    oldEventData = o->eventData;
    o->eventData = eventData;

    return oldEventData;
}
