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
#ifndef V_BUILTIN_H
#define V_BUILTIN_H

/** \file kernel/include/v_builtin.h
 *  \brief This file defines the interface
 *
 */

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"
#include "v_kernel.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* built-in topic definitions */
#define V_BUILTIN_PARTITION       "__BUILT-IN PARTITION__"

#define V_PARTICIPANTINFO_NAME   "DCPSParticipant"
#define V_CMPARTICIPANTINFO_NAME "CMParticipant"
#define V_TOPICINFO_NAME         "DCPSTopic"
#define V_TYPEINFO_NAME          "DCPSType"
#define V_PUBLICATIONINFO_NAME   "DCPSPublication"
#define V_CMDATAWRITERINFO_NAME  "CMDataWriter"
#define V_SUBSCRIPTIONINFO_NAME  "DCPSSubscription"
#define V_CMDATAREADERINFO_NAME  "CMDataReader"
#define V_CMPUBLISHERINFO_NAME   "CMPublisher"
#define V_CMSUBSCRIBERINFO_NAME  "CMSubscriber"
#define V_DELIVERYINFO_NAME      "DCPSDelivery"
#define V_HEARTBEATINFO_NAME     "DCPSHeartbeat"
#define V_C_AND_M_COMMAND_NAME   "DCPSCandMCommand"

/**
 * \brief The <code>v_builtin</code> cast method.
 *
 * This method casts an object to a <code>v_builtin</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_builtin</code> or
 * one of its subclasses.
 */
#define v_builtin(_this) (C_CAST(_this,v_builtin))

#define v_builtinTopicLookup(_this, _id) \
        (_this == NULL ? NULL : \
        v_topic(v_builtin(_this)->topics[_id]))

#define v_builtinWriterLookup(_this, _id) \
        (_this == NULL ? NULL : \
        v_writer(v_builtin(_this)->writers[_id]))

typedef v_result (*v_publicationInfo_action)  (struct v_publicationInfo *info, c_voidp arg);
typedef v_result (*v_subscriptionInfo_action) (struct v_subscriptionInfo *info, c_voidp arg);

OS_API v_message
v_builtinCreateParticipantInfo (
    v_builtin _this,
    v_participant p);

OS_API v_message
v_builtinCreateCMParticipantInfo (
    v_builtin _this,
    v_participant p);

OS_API v_writeResult
v_builtinWriteHeartbeat(
    v_writer writer,
    c_ulong systemId,
    os_timeW timestamp,
    os_duration duration,
    v_state state);

OS_API const c_char *
v_builtinInfoIdToName(
    v_infoId infoId);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
