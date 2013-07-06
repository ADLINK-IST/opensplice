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
#include "v__writer.h"

#include "v_public.h"
#include "v_time.h"
#include "v_event.h"

#include "os.h"
#include "os_report.h"

void
v_observerInit(
    v_observer o,
    const c_char *name,
    v_statistics s,
    c_bool enable)
{
    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    c_mutexInit(&o->mutex,SHARED_MUTEX);  /* mutex to protect attributes */
    c_condInit(&o->cv, &o->mutex, SHARED_COND); /* condition variable */
    o->waitCount = 0;                     /* number of waiting threads */
    o->eventMask = 0;                     /* specifies, interested events */
    o->eventFlags = 0;                    /* ocurred events */
    o->eventData = NULL;                  /* general post box for derived classes */
    v_observableInit(v_observable(o),name, s, enable);
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

    c_ulong trigger;

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
            case K_WAITSET:
                v_waitsetNotify(v_waitset(_this), event, userData);
            break;
            case K_PARTICIPANT:
                v_participantNotify(v_participant(_this), event, userData);
            break;
            case K_TOPIC:
                v_topicNotify(v_topic(_this), event, userData);
            break;
            case K_QUERY:
                /* v_queryNotify(v_query(_this), event, userData); */
            break;
            case K_SPLICED: /* intentionally no break */
            case K_SERVICE:
            case K_NETWORKING:
            case K_DURABILITY:
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
            break;
            default:
                OS_REPORT_1(OS_ERROR,"Kernel Observer",0,
                            "Notify called on an unknown observer type: %d",
                            v_objectKind(_this));
            break;
            }

            /*
             * Only trigger condition variable if at least
             * one thread is waiting AND the event is seen for the first time.
             */
            if ((_this->waitCount > 0) &&
                ((trigger == V_EVENT_TRIGGER) || (~_this->eventFlags & trigger)))
            {
                _this->eventFlags |= trigger; /* store event */
                c_condBroadcast(&_this->cv);
            } else {
                _this->eventFlags |= trigger; /* store event */
            }
        }
    } /* else observer object destroyed, skip notification */
}

c_ulong
v__observerWait(
    v_observer o)
{
    os_result result;
    c_ulong flags;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    if (o->eventFlags == 0) {
        o->waitCount++;
        result = c_condWait(&o->cv,&o->mutex);
        assert(result == os_resultSuccess);
        o->waitCount--;
    }
    flags = o->eventFlags;
    /* Reset events but remember destruction event.
     * To avoid any further use of this observer in case of destruction.
     */
    o->eventFlags &= V_EVENT_OBJECT_DESTROYED;

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
    const c_time time)
{
    os_result result = os_resultSuccess;
    c_ulong flags;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    if (o->eventFlags == 0) {
        o->waitCount++;
        result = c_condTimedWait(&o->cv,&o->mutex,time);
        o->waitCount--;
        if (result == os_resultTimeout) {
            o->eventFlags |= V_EVENT_TIMEOUT;
        }
    }
    flags = o->eventFlags;
    /* Reset events but remember destruction event.
     * To avoid any further use of this observer in case of destruction.
     */
    o->eventFlags &= V_EVENT_OBJECT_DESTROYED;

    return flags;
}

c_ulong
v_observerTimedWait(
    v_observer o,
    const c_time time)
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
v_observerSetEvent(
    v_observer o,
    c_ulong event)
{
    c_ulong eventMask;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    c_mutexLock(&o->mutex);
    eventMask = o->eventMask;
    o->eventMask |= event;
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
