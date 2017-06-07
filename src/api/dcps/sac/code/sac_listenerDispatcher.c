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

#include "dds_dcps.h"
#include "sac_common.h"
#include "sac_object.h"
#include "sac_entity.h"
#include "sac_publisher.h"
#include "sac_subscriber.h"
#include "sac_topic.h"
#include "sac_dataWriter.h"
#include "sac_dataReader.h"
#include "sac_domainParticipant.h"
#include "sac_listenerDispatcher.h"
#include "u_observable.h"
#include "u_listener.h"
#include "sac_report.h"
#include "vortex_os.h"

#if 0
#define TRACE_EVENT printf
#else
#define TRACE_EVENT(...)
#endif

os_schedClass
DDS_ListenerDispatcher_scheduling_class (
    const DDS_SchedulingQosPolicy *scheduling)
{
    os_schedClass osSchedClass = OS_SCHED_DEFAULT;

    switch (scheduling->scheduling_class.kind) {
        case DDS_SCHEDULE_TIMESHARING:
            osSchedClass = OS_SCHED_TIMESHARE;
            break;
        case DDS_SCHEDULE_REALTIME:
            osSchedClass = OS_SCHED_REALTIME;
            break;
        default:
            assert (scheduling->scheduling_class.kind == DDS_SCHEDULE_DEFAULT);
            break;
    }

    return osSchedClass;
}

os_int32
DDS_ListenerDispatcher_scheduling_priority (
    const DDS_SchedulingQosPolicy *scheduling)
{
    os_int32 osSchedPriority = scheduling->scheduling_priority;

    if (scheduling->scheduling_priority_kind.kind == DDS_PRIORITY_RELATIVE) {
       osSchedPriority += os_procAttrGetPriority ();
    }

    return osSchedPriority;
}

void
DDS_ListenerDispatcher_event_handler (
    v_listenerEvent event,
    c_voidp arg)
{
    DDS_Entity entity;

    OS_UNUSED_ARG(arg);

    if (event->kind & V_EVENT_TRIGGER) {
        /* Nothing to deliver so ignore. */
        TRACE_EVENT("DDS_ListenerDispatcher_event_handler: received V_EVENT_TRIGGER\n");
        return;
    }
    entity = (DDS_Entity)u_observableGetUserData(event->userData);

    TRACE_EVENT("DDS_ListenerDispatcher_event_handler::notify_listener(0x%x, 0x%x)\n", entity, event);

    switch(u_objectKind(event->userData)) {
    case U_PARTICIPANT:
        DDS_DomainParticipant_notify_listener(entity, event);
    break;
    case U_TOPIC:
        DDS_Topic_notify_listener(entity, event);
    break;
    case U_PUBLISHER:
        DDS_Publisher_notify_listener(entity, event);
    break;
    case U_SUBSCRIBER:
        DDS_Subscriber_notify_listener(entity, event);
    break;
    case U_WRITER:
        DDS_DataWriter_notify_listener(entity, event);
    break;
    case U_READER:
        DDS_DataReader_notify_listener(entity, event);
    break;
    default:
        TRACE_EVENT("DDS_ListenerDispatcher_event_handler: unknown userData (u_entity) 0x%x\n", event->userData);
    break;
    }
    if (event->kind & (V_EVENT_OBJECT_DESTROYED | V_EVENT_PREPARE_DELETE)) {
        /* source entity is being destroyed but waiting until all events are processed.
         * So make sure no more notify listeners is called after this call for this entity.
         * And wakeup entity that it can continue with its destruction.
         */
        entity = u_observableGetUserData(u_observable(event->source));
        DDS_Entity_notify_listener_removed(entity);
    }
}
