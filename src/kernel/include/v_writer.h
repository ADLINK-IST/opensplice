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

#ifdef OSPL_BUILD_KERNEL
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

#define v_writer_t(scope) \
        c_type(c_resolve(c_getBase(scope), \
                          "kernelModule::v_writer"))

#define v_writerTopic(_this) \
        v_topic(v_writer(_this)->topic)

#define v_writerPublisher(_this) \
        v_publisher(writer->publisher)

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
    v_writerQos qos,
    c_bool enable);

OS_API void
v_writerInit(
    v_writer w,
    v_publisher p,
    const c_char *name,
    v_topic topic,
    v_writerQos qos,
    c_bool enable);

OS_API void
v_writerFree(
    v_writer w);

OS_API void
v_writerDeinit(
    v_writer w);

OS_API v_result
v_writerEnable(
    v_writer writer);

OS_API v_writeResult
v_writerWrite(
    v_writer w,
    v_message o,
    c_time timestamp,
    v_writerInstance instance);

OS_API v_writeResult
v_writerDispose(
    v_writer w,
    v_message o,
    c_time timestamp,
    v_writerInstance instance);

OS_API v_writeResult
v_writerWriteDispose(
    v_writer w,
    v_message o,
    c_time timestamp,
    v_writerInstance instance);

OS_API v_writeResult
v_writerRegister(
    v_writer w,
    v_message o,
    c_time timestamp,
    v_writerInstance *instance);

OS_API v_writeResult
v_writerUnregister(
    v_writer w,
    v_message o,
    c_time timestamp,
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

OS_API void
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
v_writerGetTopicMatchStatus(
    v_writer w,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_writerWaitForAcknowledgments(
	v_writer w,
	v_duration timeout);

OS_API c_bool
v_writerContainsInstance(
    v_writer _this,
    v_writerInstance instance);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
