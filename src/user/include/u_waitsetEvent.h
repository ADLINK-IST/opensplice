/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef U_WAITSETEVENT_H
#define U_WAITSETEVENT_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_waitsetEvent(e)               ((u_waitsetEvent)e)
#define u_waitsetHistoryDeleteEvent(e)  ((u_waitsetHistoryDeleteEvent)e)
#define u_waitsetHistoryRequestEvent(e) ((u_waitsetHistoryRequestEvent)e)
#define u_waitsetPersistentSnapshotEvent(e) ((u_waitsetPersistentSnapshotEvent)e)


typedef enum u_waitsetEventKind_s {
    U_WAITSET_EVENT,
    U_WAITSET_EVENT_HISTORY_DELETE,
    U_WAITSET_EVENT_HISTORY_REQUEST,
    U_WAITSET_EVENT_PERSISTENT_SNAPSHOT
} u_waitsetEventKind;

C_STRUCT(u_waitsetEvent) {
    u_entity entity;
    c_ulong  events;
    u_waitsetEventKind kind;
};

C_STRUCT(u_waitsetHistoryDeleteEvent) {
    C_EXTENDS(u_waitsetEvent);
    c_char* partitionExpr;
    c_char* topicExpr;
    c_time  deleteTime;
};

C_STRUCT(u_waitsetHistoryRequestEvent) {
    C_EXTENDS(u_waitsetEvent);
    v_handle source;
    c_char* filter;
    c_char** filterParams;
    c_long filterParamsCount;
    struct v_resourcePolicy resourceLimits;
    c_time minSourceTimestamp;
    c_time maxSourceTimestamp;
};

C_STRUCT(u_waitsetPersistentSnapshotEvent) {
    C_EXTENDS(u_waitsetEvent);
    v_handle source;
    c_char* partitionExpr;
    c_char* topicExpr;
    c_char* uri;
};

OS_API u_waitsetEvent
u_waitsetEventNew(
    u_entity e, c_ulong k);

OS_API u_waitsetEvent
u_waitsetHistoryDeleteEventNew(
    u_entity e,
    c_ulong k,
    const c_char* partitionExpr,
    const c_char* topicExpr,
    c_time deleteTime);

OS_API u_waitsetEvent
u_waitsetHistoryRequestEventNew(
    u_entity e,
    c_ulong k,
    v_handle source,
    c_char* filter,
    c_array filterParams,
    struct v_resourcePolicy resourceLimits,
    c_time minSourceTimestamp,
    c_time maxSourceTimestamp);

OS_API u_waitsetEvent
u_waitsetPersistentSnapshotEventNew(
    u_entity e,
    c_ulong k,
    v_handle source,
    const c_char* partitionExpr,
    const c_char* topicExpr,
    const c_char* uri);

OS_API void
u_waitsetEventFree(
    u_waitsetEvent event);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

