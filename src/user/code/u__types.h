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

#ifndef U__TYPES_H
#define U__TYPES_H

#include "os.h"
#include "os_heap.h"
#include "c_iterator.h"
#include "u__domain.h"
#include "u_writer.h"
#include "u_reader.h"
#include "u_dispatcher.h"
#include "u_serviceTerminationThread.h"

/*#define PROFILER*/
#ifdef PROFILER
#include "c_laptime.h"
#endif

C_STRUCT(u_entity) {
    u_participant participant;
    u_kind kind;
    u_handle handle;
    /* only use magic for v_gidClaim and v_gidRelease. */
    v_kernel magic;
    v_gid gid;
    c_voidp userData;
    c_bool enabled;
    /* flags indicates how entity is created....
      see U_ECREATE_* in u__entity.h */
    c_ulong flags;
    os_mutex mutex;
    c_ulong refCount;
};

C_STRUCT(u_dispatcher)
{
    C_EXTENDS(u_entity);
    c_iter                   listeners;
    os_mutex                 mutex;
    os_threadId              threadId;
    c_ulong                  event;
    u_dispatcherThreadAction startAction;
    u_dispatcherThreadAction stopAction;
    c_voidp                  actionData;
};

C_STRUCT(u_publisher) {
    C_EXTENDS(u_dispatcher);
    u_participant participant;
    c_iter writers;
};

C_STRUCT(u_writer) {
    C_EXTENDS(u_dispatcher);
    u_publisher publisher;
    u_writerCopy copy;
    os_time autoLeasePeriod;
    os_time resendPeriod;
#ifdef PROFILER
c_laptime t1,t2,t3,t4,t5,t6;
#endif
};

C_STRUCT(u_subscriber) {
    C_EXTENDS(u_dispatcher);
    u_participant participant;
    c_iter readers;
};

C_STRUCT(u_reader) {
    C_EXTENDS(u_dispatcher);
    c_iter queries;
    os_mutex mutex;
};

C_STRUCT(u_dataReader) {
    C_EXTENDS(u_reader);
    u_subscriber subscriber;
    u_topic topic;
    c_iter views;
};

C_STRUCT(u_networkReader) {
    C_EXTENDS(u_reader);
    u_subscriber subscriber;
};

C_STRUCT(u_groupQueue) {
    C_EXTENDS(u_reader);
};

C_STRUCT(u_query) {
    C_EXTENDS(u_dispatcher);
    u_reader source;
    c_char *name;
    q_expr predicate;
};

C_STRUCT(u_dataView) {
    C_EXTENDS(u_reader);
    u_dataReader source;
};

C_STRUCT(u_group) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_partition) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_topic) {
    C_EXTENDS(u_dispatcher);
    u_participant participant;
    c_char *name;
};

C_STRUCT(u_contentFilteredTopic) {
    C_EXTENDS(u_topic);
    u_topic topic;
    c_char *expression;
    c_iter parameters;
};

C_STRUCT(u_waitset) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_participant)
{
    C_EXTENDS(u_dispatcher);
    c_iter topics;
    c_iter publishers;
    c_iter subscribers;
    u_subscriber builtinSubscriber;
    c_ulong      builtinTopicCount;
    os_threadId  threadId;
    os_threadId  threadIdResend;
    u_domain     domain;
};

C_STRUCT(u_service) {
    C_EXTENDS(u_participant);
    c_voidp privateData;
    u_serviceKind serviceKind;
    u_serviceTerminationThread stt;
};

C_STRUCT(u_spliced) {
    C_EXTENDS(u_service);
};

C_STRUCT(u_serviceManager) {
    C_EXTENDS(u_dispatcher);
};

C_STRUCT(u_domain) {
    C_EXTENDS(u_dispatcher);
    v_kernel        kernel;
    os_sharedHandle shm;
    c_iter          participants;
    c_char          *uri;
    c_char          *name;
    os_lockPolicy   lockPolicy;
    os_uint32       protectCount;
    os_int32        id;
};

#endif
