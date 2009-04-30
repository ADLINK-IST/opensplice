/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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
#include "u_kernel.h"
#include "u_writer.h"
#include "u_reader.h"
#include "u_dispatcher.h"

/*#define PROFILER*/
#ifdef PROFILER
#include "c_laptime.h"
#endif

C_STRUCT(u_entity) {
    u_participant participant;
    u_kind kind;
    v_handle handle;
    /* only use magic for v_gidClaim and v_gidRelease. */
    v_kernel magic;
    v_gid gid;
    c_voidp userData;
    c_bool enabled;
    /* flags indicates how entity is created....
      see U_ECREATE_* in u__entity.h */
    c_ulong flags;
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
};

C_STRUCT(u_writer) {
    C_EXTENDS(u_dispatcher);
    u_writerCopy copy;
    os_time autoLeasePeriod;
    os_time resendPeriod;
#ifdef PROFILER
c_laptime t1,t2,t3,t4,t5,t6;
#endif
};

C_STRUCT(u_subscriber) {
    C_EXTENDS(u_dispatcher);
};

C_STRUCT(u_reader) {
    C_EXTENDS(u_dispatcher);
};

C_STRUCT(u_dataReader) {
    C_EXTENDS(u_reader);
};

C_STRUCT(u_networkReader) {
    C_EXTENDS(u_reader);
};

C_STRUCT(u_groupQueue) {
    C_EXTENDS(u_reader);
};

C_STRUCT(u_query) {
    C_EXTENDS(u_reader);
    u_reader source;
    c_char *name;
    q_expr predicate;
};

C_STRUCT(u_dataView) {
    C_EXTENDS(u_reader);
    u_reader source;
};

C_STRUCT(u_group) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_domain) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_topic) {
    C_EXTENDS(u_dispatcher);
};

C_STRUCT(u_waitset) {
    C_EXTENDS(u_entity);
};

C_STRUCT(u_participant)
{
    C_EXTENDS(u_dispatcher);
    os_threadId  threadId;
    os_threadId  threadIdResend;
    u_kernel     kernel;
};

C_STRUCT(u_service) {
    C_EXTENDS(u_participant);
    c_voidp privateData;
    u_serviceKind serviceKind;
};

C_STRUCT(u_spliced) {
    C_EXTENDS(u_service);
};

C_STRUCT(u_serviceManager) {
    C_EXTENDS(u_dispatcher);
};

#endif
