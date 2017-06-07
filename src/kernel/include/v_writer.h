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
#ifndef V_WRITER_H
#define V_WRITER_H

#include "v_kernel.h"
#include "v_entity.h"
#include "v_publisher.h"
#include "v_topic.h"
#include "v_status.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_writer</code> cast methods.
 *
 * This method casts an object to a <code>v_writer</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_writer</code> or
 * one of its subclasses.
 */
#define v_writer(o) (C_CAST(o,v_writer))
#define v_writerResendItem(o) (C_CAST(o,v_writerResendItem))
#define v_writerEotSample(o) (C_CAST(o,v_writerEotSample))

#define V_RESENDITEM_WRITERSAMPLE    (1)
#define V_RESENDITEM_WRITEREOTSAMPLE (2)

#define v_writerTopic(_this) \
        v_topic(v_writer(_this)->topic)

#define v_writerPublisher(_this) \
        v_publisher(_this->publisher)

#define v_writerParticipant(_this) \
        v_publisherParticipant(v_writerPublisher(_this))

OS_API const char*
v_writeResultString(
    v_writeResult result);

OS_API v_writer
v_writerNew(
    v_publisher p,
    const c_char *name,
    v_topic topic,
    v_writerQos qos);

OS_API void
v_writerInit(
    v_writer w,
    v_publisher p,
    const c_char *name,
    v_topic topic,
    v_writerQos qos);

OS_API void
v_writerFree(
    v_writer w);

OS_API void
v_writerDeinit(
    v_writer w);

OS_API v_writerQos
v_writerGetQos (
    v_writer _this);

OS_API v_result
v_writerSetQos (
    v_writer _this,
    v_writerQos qos);

OS_API v_result
v_writerEnable(
    v_writer writer);

OS_API v_writeResult
v_writerWrite(
    v_writer w,
    v_message o,
    os_timeW timestamp,
    v_writerInstance instance);

OS_API v_writeResult
v_writerDispose(
    v_writer w,
    v_message o,
    os_timeW timestamp,
    v_writerInstance instance);

OS_API v_writeResult
v_writerWriteDispose(
    v_writer w,
    v_message o,
    os_timeW timestamp,
    v_writerInstance instance);

OS_API v_writeResult
v_writerRegister(
    v_writer w,
    v_message o,
    os_timeW timestamp,
    v_writerInstance *instance);

OS_API v_writeResult
v_writerUnregister(
    v_writer w,
    v_message o,
    os_timeW timestamp,
    v_writerInstance instance);

OS_API c_bool
v_writerRead (
    v_writer writer,
    c_action action,
    c_voidp arg);

OS_API v_writerInstance
v_writerLookupInstance(
    v_writer w,
    v_message keyTemplate);

OS_API c_bool
v_writerCheckInstanceConsistency(
    v_writer writer,
    v_message message,
    v_writerInstance instance);

OS_API v_message
v_writerCreateInstanceMessage(
    v_writer writer,
    v_writerInstance instance);

OS_API c_bool
v_writerPublish(
    v_writer w,
    v_partition d);

OS_API c_bool
v_writerUnPublish(
    v_writer w,
    v_partition d);

OS_API c_bool
v_writerResend(
    v_writer w);

OS_API void
v_writerAssertLiveliness(
    v_writer w);

OS_API v_result
v_writerGetLivelinessLostStatus(
    v_writer w,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_writerGetDeadlineMissedStatus(
    v_writer w,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_writerGetIncompatibleQosStatus(
    v_writer w,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_writerGetPublicationMatchedStatus(
    v_writer w,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_writerWaitForAcknowledgments(
    v_writer w,
    os_duration timeout);

OS_API c_bool
v_writerContainsInstance(
    v_writer _this,
    v_writerInstance instance);

/* This function is made public accessible only
 * for the purpose to be used in design based testing.
 */
OS_API void
v_writerCoherentBegin (
    v_writer _this,
    c_ulong *transactionId);

/* This function is made public accessible only
 * for the purpose to be used in design based testing.
 */
OS_API v_result
v_writerCoherentEnd (
    v_writer _this,
    c_ulong publisherId,
    c_ulong transactionId,
    c_array tidList);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
