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
#include "Entity.h"
#include "ListenerDispatcher.h"
#include "StatusUtils.h"
#include "ReportUtils.h"
#include "u_observable.h"
#include "os_defs.h"

#if 0
#define TRACE_EVENT printf
#else
#define TRACE_EVENT(...)
#endif

os_schedClass
DDS::OpenSplice::ListenerDispatcher::scheduling_class (
    const DDS::SchedulingQosPolicy &scheduling)
{
    os_schedClass osSchedClass = OS_SCHED_DEFAULT;

    switch (scheduling.scheduling_class.kind) {
        case DDS::SCHEDULE_TIMESHARING:
            osSchedClass = OS_SCHED_TIMESHARE;
            break;
        case DDS::SCHEDULE_REALTIME:
            osSchedClass = OS_SCHED_REALTIME;
            break;
        default:
            assert (scheduling.scheduling_class.kind == DDS::SCHEDULE_DEFAULT);
            break;
    }

    return osSchedClass;
}

os_int32
DDS::OpenSplice::ListenerDispatcher::scheduling_priority (
    const DDS::SchedulingQosPolicy &scheduling)
{
    os_int32 osSchedPriority = scheduling.scheduling_priority;


    if (scheduling.scheduling_priority_kind.kind == DDS::PRIORITY_RELATIVE) {
       osSchedPriority += os_procAttrGetPriority ();
    }

    return osSchedPriority;
}

void
DDS::OpenSplice::ListenerDispatcher::event_handler (
    v_listenerEvent event,
    c_voidp argument)
{
    DDS::OpenSplice::Entity *listeningEntity;
    DDS::OpenSplice::Entity *sourceEntity;
    void *cppObject;
    u_object uObject;

    OS_UNUSED_ARG(argument);
    assert(event);

    if (event->kind & V_EVENT_TRIGGER) {
        /* Nothing to deliver so ignore. */
        TRACE_EVENT("static::ListenerDispatcher::eventHandler: received V_EVENT_TRIGGER\n");
        return;
    }

    /* Extract source entity from event source. */
    uObject = u_object(event->source);
    assert (uObject != NULL);
    cppObject = u_observableGetUserData(u_observable(uObject));
    assert (cppObject != NULL);
    sourceEntity = reinterpret_cast<DDS::OpenSplice::Entity*>(cppObject);
    assert (sourceEntity != NULL);

    /* Extract listening entity instance from user data. */
    uObject = u_object(event->userData);
    assert (uObject != NULL);
    cppObject = u_observableGetUserData(u_observable(uObject));
    assert (cppObject != NULL);
    listeningEntity = reinterpret_cast<DDS::OpenSplice::Entity*>(cppObject);
    assert (listeningEntity != NULL);

    TRACE_EVENT("static::ListenerDispatcher::eventHandler(0x%08lx, 0x%08lx)\n", listeningEntity, event->kind);

    /* Keep the entities alive during notification. */
    (void)DDS::Entity::_duplicate(sourceEntity);
    (void)DDS::Entity::_duplicate(listeningEntity);

    /* Call entity to notify its listener of the current event. */
    listeningEntity->nlReq_notify_listener(sourceEntity, event->kind, event->eventData);

    if (event->kind & (V_EVENT_OBJECT_DESTROYED | V_EVENT_PREPARE_DELETE)) {
        /* source entity is being destroyed but waiting until all events are processed.
         * So make sure no more notify listeners is called after this call for this entity.
         * And wakeup entity that it can continue with its destruction.
         */
        (void)sourceEntity->nlReq_notify_listener_removed();
    }
    /* Release the duplicated notification entities. */
    DDS::release(listeningEntity);
    DDS::release(sourceEntity);
}
