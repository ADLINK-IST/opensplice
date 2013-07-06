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

#include "u_waitsetEvent.h"
#include "os.h"
#include "u__types.h"

u_waitsetEvent
u_waitsetEventNew(
    u_entity e,
    c_ulong  k)
{
    u_waitsetEvent event;

    event = (u_waitsetEvent)os_malloc(C_SIZEOF(u_waitsetEvent));
    if (event) {
        event->entity = e;
        event->events = k;
        event->kind   = U_WAITSET_EVENT;
    }

    return event;
}

u_waitsetEvent
u_waitsetHistoryDeleteEventNew(
    u_entity e,
    c_ulong k,
    const c_char* partitionExpr,
    const c_char* topicExpr,
    c_time deleteTime)
{
    u_waitsetEvent event;

    event = (u_waitsetEvent)os_malloc(C_SIZEOF(u_waitsetHistoryDeleteEvent));

    if (event) {
        event->entity                 = e;
        event->events                 = k;
        event->kind                   = U_WAITSET_EVENT_HISTORY_DELETE;

        u_waitsetHistoryDeleteEvent(event)->partitionExpr          = os_strdup(partitionExpr);
        u_waitsetHistoryDeleteEvent(event)->topicExpr              = os_strdup(topicExpr);
        u_waitsetHistoryDeleteEvent(event)->deleteTime.seconds     = deleteTime.seconds;
        u_waitsetHistoryDeleteEvent(event)->deleteTime.nanoseconds = deleteTime.nanoseconds;
    }
    return event;
}

u_waitsetEvent
u_waitsetPersistentSnapshotEventNew(
    u_entity e,
    c_ulong k,
    v_handle source,
    const c_char* partitionExpr,
    const c_char* topicExpr,
    const c_char* uri)
{
    u_waitsetEvent event;

    event = (u_waitsetEvent)os_malloc(C_SIZEOF(u_waitsetPersistentSnapshotEvent));

    if (event) {
        event->entity                 = e;
        event->events                 = k;
        event->kind                   = U_WAITSET_EVENT_PERSISTENT_SNAPSHOT;

        u_waitsetPersistentSnapshotEvent(event)->partitionExpr      = os_strdup(partitionExpr);
        u_waitsetPersistentSnapshotEvent(event)->topicExpr          = os_strdup(topicExpr);
        u_waitsetPersistentSnapshotEvent(event)->uri                = os_strdup(uri);
        u_waitsetPersistentSnapshotEvent(event)->source.server      = source.server;
        u_waitsetPersistentSnapshotEvent(event)->source.index       = source.index;
        u_waitsetPersistentSnapshotEvent(event)->source.serial      = source.serial;
    }
    return event;
}

u_waitsetEvent
u_waitsetConnectWriterEventNew(
    u_entity e,
    c_ulong k,
    v_handle source,
    v_group group)
{
    u_waitsetEvent event;

    event = (u_waitsetEvent)os_malloc(C_SIZEOF(u_waitsetConnectWriterEvent));

    if (event) {
        event->entity                 = e;
        event->events                 = k;
        event->kind                   = U_WAITSET_EVENT_CONNECT_WRITER;

        u_waitsetConnectWriterEvent(event)->group              = group;
        u_waitsetConnectWriterEvent(event)->source.server      = source.server;
        u_waitsetConnectWriterEvent(event)->source.index       = source.index;
        u_waitsetConnectWriterEvent(event)->source.serial      = source.serial;
    }
    return event;

}

u_waitsetEvent
u_waitsetHistoryRequestEventNew(
    u_entity e,
    c_ulong k,
    v_handle source,
    c_char* filter,
    c_array filterParams,
    struct v_resourcePolicy resourceLimits,
    c_time minSourceTimestamp,
    c_time maxSourceTimestamp)
{
    u_waitsetEvent event;
    u_waitsetHistoryRequestEvent hrEvent;
    c_long i;

    event = (u_waitsetEvent)os_malloc(C_SIZEOF(u_waitsetHistoryRequestEvent));

    if (event) {
        hrEvent                     = u_waitsetHistoryRequestEvent(event);
        event->entity               = e;
        event->events               = k;
        event->kind                 = U_WAITSET_EVENT_HISTORY_REQUEST;

        if(filter){
            hrEvent->filter         = os_strdup(filter);
        } else {
            hrEvent->filter         = NULL;
        }
        hrEvent->source.server      = source.server;
        hrEvent->source.index       = source.index;
        hrEvent->source.serial      = source.serial;
        hrEvent->resourceLimits     = resourceLimits;
        hrEvent->minSourceTimestamp = minSourceTimestamp;
        hrEvent->maxSourceTimestamp = maxSourceTimestamp;
        hrEvent->filterParamsCount  = c_arraySize(filterParams);

        if(hrEvent->filterParamsCount > 0){
            hrEvent->filterParams = (c_char**)(os_malloc(
                                hrEvent->filterParamsCount*sizeof(c_char*)));

            for(i=0; i<hrEvent->filterParamsCount; i++){
                hrEvent->filterParams[i] = os_strdup(filterParams[i]);
            }
        } else {
            hrEvent->filterParams = NULL;
        }
    }
    return event;
}

void
u_waitsetEventFree(
    u_waitsetEvent event)
{
    c_long i;

    if (event) {
        switch(event->kind){
            case U_WAITSET_EVENT:
                /*Do nothing*/
                break;
            case U_WAITSET_EVENT_HISTORY_DELETE:
                os_free(u_waitsetHistoryDeleteEvent(event)->partitionExpr);
                os_free(u_waitsetHistoryDeleteEvent(event)->topicExpr);
                break;
            case U_WAITSET_EVENT_HISTORY_REQUEST:
                if(u_waitsetHistoryRequestEvent(event)->filter){
                    os_free(u_waitsetHistoryRequestEvent(event)->filter);
                }
                for(i=0; i<u_waitsetHistoryRequestEvent(event)->filterParamsCount; i++){
                    os_free(u_waitsetHistoryRequestEvent(event)->filterParams[i]);
                }

                if(u_waitsetHistoryRequestEvent(event)->filterParams){
                    os_free(u_waitsetHistoryRequestEvent(event)->filterParams);
                }
                break;
            case U_WAITSET_EVENT_PERSISTENT_SNAPSHOT:
                if(u_waitsetPersistentSnapshotEvent(event)->partitionExpr)
                {
                    os_free(u_waitsetPersistentSnapshotEvent(event)->partitionExpr);
                }
                if(u_waitsetPersistentSnapshotEvent(event)->topicExpr)
                {
                    os_free(u_waitsetPersistentSnapshotEvent(event)->topicExpr);
                }
                if(u_waitsetPersistentSnapshotEvent(event)->uri)
                {
                    os_free(u_waitsetPersistentSnapshotEvent(event)->uri);
                }
                break;
            case U_WAITSET_EVENT_CONNECT_WRITER:
                break;
            default:
                break;
        }
        os_free(event);
    }
}
