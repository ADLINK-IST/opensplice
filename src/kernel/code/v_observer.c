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
    v_observer o,
    v_event event,
    c_voidp userData)
{
    c_ulong trigger;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    if ((o->eventFlags & V_EVENT_OBJECT_DESTROYED) == 0) {
        /* observer is not destroyed, so continue */
        if (event != NULL ) {
            trigger = event->kind & o->eventMask;
        } else {
            trigger = V_EVENT_TRIGGER; /* NULL-event always notifies observers */
        }

        if (trigger != 0) {
            switch (v_objectKind(o)) {
            case K_DATAREADER:
                v_dataReaderNotify(v_dataReader(o), event, userData);
            break;
            case K_WAITSET:
                v_waitsetNotify(v_waitset(o), event, userData);
            break;
            case K_PARTICIPANT:
                v_participantNotify(v_participant(o), event, userData);
            break;
            case K_GROUPQUEUE:
                v_groupStreamNotify(v_groupStream(o), event, userData);
            break;
            case K_TOPIC:
                v_topicNotify(v_topic(o), event, userData);
            break;
            case K_QUERY:
                /* v_queryNotify(v_query(o), event, userData); */
            break;
            case K_SPLICED: /* intentionally no break */
            case K_SERVICE:
            case K_NETWORKING:
            case K_DURABILITY:
            case K_CMSOAP:
                v_serviceNotify(v_service(o), event, userData);
            break;
            case K_SERVICEMANAGER:
                v_serviceManagerNotify(v_serviceManager(o), event, userData);
            break;
            case K_WRITER: /* no action for the following observers */
            case K_PUBLISHER:
            case K_SUBSCRIBER:
            break;
            default:
                OS_REPORT_1(OS_INFO,"Kernel Observer",0,
                            "Notify an unknown observer type: %d",
                            v_objectKind(o));
            break;
            }

            /* 
             * Only trigger condition variable if at least
             * one thread is waiting AND the event is seen for the first time.
             */
            if ((o->waitCount > 0) &&
                ((trigger == V_EVENT_TRIGGER) || (~o->eventFlags & trigger)))
            {
                o->eventFlags |= trigger; /* store event */
                c_condBroadcast(&o->cv);
            } else {
                o->eventFlags |= trigger; /* store event */
            }
        }
    } /* else observer object destroyed, skip notification */
}

c_ulong
v__observerWait(
    v_observer o)
{
    os_result result = os_resultSuccess;
    c_ulong flags;

    assert(o != NULL);
    assert(C_TYPECHECK(o,v_observer));

    if (o->eventFlags == 0) {
        o->waitCount++;
        result = c_condWait(&o->cv,&o->mutex);
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
    c_time elapsedTime, currentTime, waitTime;
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
