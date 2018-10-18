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
#include "v__observer.h"

#include "v__observable.h"
#include "v__participant.h"
#include "v__subscriber.h"
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

/* The following three operations are depricated. */
void v_observerLock(v_observer o) { OSPL_LOCK(o); }
void v_observerUnlock(v_observer o) { OSPL_UNLOCK(o); }

void
v_observerInit(
    _Inout_ v_observer o)
{
    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    v_observableInit(v_observable(o));
    c_condInit(c_getBase(o), &o->cv, &v_observable(o)->eventLock); /* condition variable */
    o->waitCount = 0;                     /* number of waiting threads */
    o->eventMask = 0;                     /* specifies, interested events */
    o->eventFlags = 0;                    /* ocurred events */
}

void
v_observerFree(
    v_observer o)
{
    assert(C_TYPECHECK(o,v_observer));

    OSPL_LOCK(o);
    OSPL_BLOCK_EVENTS(o);
    o->eventFlags |= V_EVENT_OBJECT_DESTROYED;
    c_condBroadcast(&o->cv);
    OSPL_UNBLOCK_EVENTS(o);
    OSPL_UNLOCK(o);

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

    OSPL_BLOCK_EVENTS(o);
    flags = o->eventFlags;
    o->eventFlags &= V_EVENT_OBJECT_DESTROYED;
    OSPL_UNBLOCK_EVENTS(o);
    EVENT_TRACE("v_observerGetEventFlags(_this = 0x%x, flags = 0x%x)\n", o, flags);
    return flags;
}

void
v_observerClearEventFlags(
    v_observer o)
{
    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    OSPL_BLOCK_EVENTS(o);
    o->eventFlags &= V_EVENT_OBJECT_DESTROYED;
    OSPL_UNBLOCK_EVENTS(o);
    EVENT_TRACE("v_observerClearEventFlags(_this = 0x%x)\n", o);
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
     * For consistency _this must be locked by OSPL_LOCK(_this) before
     * calling this method.
     */

    c_ulong trigger = 0;
    c_bool  notify  = TRUE;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_observer));

    OSPL_BLOCK_EVENTS(_this);
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
            if (event != NULL) {
                switch (v_objectKind(_this)) {
                case K_WAITSET:
                    v_waitsetNotify(v_waitset(_this), event, userData);
                break;
                case K_PARTICIPANT:
                    v_participantNotify(v_participant(_this), event, userData);
                break;
                case K_TOPIC_ADAPTER:
                    v_topicAdapterNotify(v_topicAdapter(_this), event, userData);
                break;
                case K_SUBSCRIBER:
                    v_subscriberNotify(v_subscriber(_this), event, userData);
                break;
                case K_SPLICED: /* all services, intentionally no break */
                case K_SERVICE:
                case K_NETWORKING:
                case K_DURABILITY:
                case K_NWBRIDGE:
                case K_CMSOAP:
                case K_RNR:
                case K_DBMSCONNECT:
                    v_serviceNotify(v_service(_this), event, userData);
                break;
                case K_WRITER: /* no event passing for the following observers */
                case K_DATAREADER:
                case K_QUERY:
                case K_PUBLISHER:
                case K_TOPIC:
                case K_GROUPQUEUE:
                case K_STATUSCONDITION:
                case K_LISTENER:
                case K_SERVICEMANAGER:
                break;
                default:
                    OS_REPORT(OS_ERROR,"Kernel Observer",V_RESULT_INTERNAL_ERROR,
                                "Notify called on an unknown observer type: %d",
                                v_objectKind(_this));
                    assert(FALSE);
                break;
                }
            }
            /* Only trigger condition variable if at least
             * one thread is waiting AND the event is seen for the first time.
             */
            if ((_this->eventFlags & trigger) && (trigger != V_EVENT_TRIGGER)) {
                notify = FALSE;
            }
            /* store event before trigger is given to waiting threads or subclasses are notified*/
            _this->eventFlags |= trigger;

            if ((_this->waitCount > 0) && notify) {
                EVENT_TRACE("v_observerNotify(%s(0x%x), event(0x%x)) => signal blocking threads\n",
                            v_objectKindImage(_this), _this, event);
                c_condBroadcast(&_this->cv);
            }
        }
    } else { /* else observer object destroyed, skip notification */
        EVENT_TRACE("v_observerNotify: Event(0x%x) is ignored for %s(0x%x) because it is currently being destroyed\n",
                     event, v_objectKindImage(_this), _this);
    }
    OSPL_UNBLOCK_EVENTS(_this);
}

c_ulong
v_observerTimedWaitAction(
    v_observer o,
    const os_duration time,
    const c_action action,
    c_voidp arg)
{
    c_ulong flags = 0;
    v_result result = V_RESULT_OK;
    c_bool relock = FALSE;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    OSPL_BLOCK_EVENTS(o);
    if (o->eventFlags == 0) {
        if (time <= OS_DURATION_ZERO) {
            EVENT_TRACE("v__observerTimedWait: Zero-Timeout %s(0x%x), eventFlags(0x%x)\n",
                         v_objectKindImage(o), o, o->eventFlags);

            result = V_RESULT_TIMEOUT;
        } else {
            /* Unlock the OSPL_LOCK, not allowed to sleep while holding this lock.
             * This lock needs to be re-acquired again after the EVENT_LOCK is released.
             */
            relock = TRUE;
            OSPL_UNLOCK(o);
            o->waitCount++;
            EVENT_TRACE("v__observerTimedWait Block %s(0x%x) eventFlags(0x%x)\n",
                         v_objectKindImage(o), o, o->eventFlags);

            result = v_condWait(&o->cv,&v_observable(o)->eventLock, time);
            o->waitCount--;
            EVENT_TRACE("v__observerTimedWait: WakeUp %s(0x%x) eventFlags(0x%x)\n",
                         v_objectKindImage(o), o, o->eventFlags);
        }
    } else {
        EVENT_TRACE("v__observerTimedWait: Fallthrough %s(0x%x), eventFlags(0x%x)\n",
                     v_objectKindImage(o), o, o->eventFlags);
    }

    if (o->eventFlags != 0) {
        flags = o->eventFlags;
        if (action) {
            (void)action(o, arg);
        }
    } else if (result == V_RESULT_TIMEOUT) {
        flags = V_EVENT_TIMEOUT;
    }

    /* Reset events but remember destruction event.
     * To avoid any further use of this observer in case of destruction.
     */
    if (o->waitCount == 0) {
        o->eventFlags &= V_EVENT_OBJECT_DESTROYED;
    }

    OSPL_UNBLOCK_EVENTS(o);
    if (relock) {
        // Coverity false positve : LOCK is only done when relock is set to TRUE
        // because this function is entered with the lock, we have to relock in case
        // we released the lock in this function
        /* coverity[missing_unlock : FALSE] */
       OSPL_LOCK(o);
    }

    return flags;
}


c_ulong
v_observerTimedWait(
    v_observer o,
    const os_duration time)
{
    return v_observerTimedWaitAction(o, time, NULL, NULL);
}

c_ulong
v_observerWait(
    v_observer o)
{
    c_ulong result;
    OSPL_LOCK(o);
    result = v_observerTimedWaitAction(o, OS_DURATION_INFINITE, NULL, NULL);
    OSPL_UNLOCK(o);
    return result;
}

c_ulong
v_observerSetEvent(
    v_observer o,
    c_ulong event)
{
    c_ulong eventMask;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    OSPL_BLOCK_EVENTS(o);
    eventMask = o->eventMask;
    o->eventMask |= event;
    OSPL_UNBLOCK_EVENTS(o);

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

    OSPL_BLOCK_EVENTS(o);
    eventMask = o->eventMask;
    o->eventMask &= ~event;
    OSPL_UNBLOCK_EVENTS(o);

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

    OSPL_BLOCK_EVENTS(o);
    eventMask = o->eventMask;
    o->eventMask = mask;
    OSPL_UNBLOCK_EVENTS(o);

    return eventMask;
}

c_ulong
v_observerGetEventMask(
    v_observer o)
{
    c_ulong eventMask;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    OSPL_BLOCK_EVENTS(o);
    eventMask = o->eventMask;
    OSPL_UNBLOCK_EVENTS(o);

    return eventMask;
}
