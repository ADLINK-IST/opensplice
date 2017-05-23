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
#ifndef U_WRITER_H
#define U_WRITER_H

/** \file u_writer.h
 *  \brief The User Layer Writer class.
 *
 * The User Layer Writer class is a proxy to the kernel writer.
 * A writer is associated to one Topic and provides a type specific
 * interface to publish instances.
 * The writer creates messages from the given application data and
 * publishes it instantaneously or passes the message to the publisher
 * which will publish it in an asynchronous manner (this optimisation
 * is not supported yet and will be controlled via QoS).
 *
 */

#include "u_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * brief The writers copy method prototype.
 *
 * This prototype specifies the signature of the method the writer will use
 * to transform application data into kernel data.
 * This method is provided by the application.
 */
typedef v_copyin_result (*u_writerCopy)(c_type type, const void *data, void *to);
typedef u_bool          (*u_writerAction)(u_writer writer, void *arg);
typedef void            (*u_writerCopyKeyAction)(const void *data, void *to);

#include "u_instanceHandle.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_writer(w) \
        ((u_writer)u_objectCheckType(u_object(w), U_WRITER))

#define u_writerEnable(_this) u_entityEnable(u_entity(u_writer(_this)))

/**
 * \brief The User Layer Writer constructor.
 *
 * The constructor creates a new writer in the scope of the specified publisher.
 * A name can be specified, which can be useful for control and monitoring
 * purposes.
 * The name is optional and doesn't have to be unique.
 * The QoS value specified is assigned to the writer and controls its behaviour.
 * The Topic specifies the writer's data type.
 * In addition the writer constructor requires a copy method that it will use
 * to translate application data into a message in kernel space.
 * The copy needs to be executed within protected area because the message is
 * created in the kernel therefore the copy must be performed by the writer.
 *
 * \param p    The Publisher scope wherein the writer will be created.
 * \param name The optional name that can be associated to the created writer
 *             for control
 *             and monitoring facilities.
 * \param t    The Topic that specifies the type of data for the writer.
 * \param copy The application defined copy method that transforms application
 *             data into writer messages.
 * \param qos  The application defined Quality of Service values.
 * \return     The newly created writer or NULL if an error has occured.
 */

OS_API u_writer
u_writerNew (
    const u_publisher publisher,
    const os_char *name,
    const u_topic topic,
    const u_writerQos qos);

OS_API u_result
u_writerWrite (
    const u_writer _this,
    u_writerCopy copy,
    void *data,
    os_timeW timestamp,
    u_instanceHandle handle);

OS_API u_result
u_writerDispose (
    const u_writer _this,
    u_writerCopy copy,
    void *data,
    os_timeW timestamp,
    u_instanceHandle handle);

OS_API u_result
u_writerWriteDispose (
    const u_writer _this,
    u_writerCopy copy,
    void *data,
    os_timeW timestamp,
    u_instanceHandle handle);

OS_API u_result
u_writerRegisterInstance (
    const u_writer _this,
    u_writerCopy copy,
    void *data,
    os_timeW timestamp,
    u_instanceHandle *handle);

OS_API u_result
u_writerUnregisterInstance(
    const u_writer _this,
    u_writerCopy copy,
    void *data,
    os_timeW timestamp,
    u_instanceHandle handle);

OS_API u_result
u_writerAssertLiveliness (
    const u_writer _this);

OS_API u_result
u_writerCopyKeysFromInstanceHandle (
    const u_writer _this,
    u_instanceHandle handle,
    u_writerCopyKeyAction action,
    void *copyArg);

/* The default copy method is only used for DBT test purposes. */

OS_API v_copyin_result
u_writerDefaultCopy (
    c_type type,
    const void *data,
    void *to);

OS_API u_result
u_writerLookupInstance (
    const u_writer _this,
    u_writerCopy copy,
    void *keyTemplate,
    u_instanceHandle *handle);

OS_API u_result
u_writerGetLivelinessLostStatus (
    const u_writer _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_writerGetDeadlineMissedStatus (
    const u_writer _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_writerGetIncompatibleQosStatus (
    const u_writer _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_writerGetPublicationMatchStatus (
    const u_writer _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_writerGetMatchedSubscriptions (
    const u_writer _this,
    u_subscriptionInfo_action action,
    void *arg);

OS_API u_result
u_writerGetMatchedSubscriptionData (
    const u_writer _this,
    u_instanceHandle subscription_handle,
    u_subscriptionInfo_action action,
    void *arg);

OS_API u_result
u_writerWaitForAcknowledgments(
    const u_writer _this,
    os_duration timeout);

OS_API u_result
u_writerGetQos (
    const u_writer _this,
    u_writerQos *qos);

OS_API u_result
u_writerSetQos (
    const u_writer _this,
    const u_writerQos qos);

OS_API u_result
u_resultFromKernelWriteResult (
    v_writeResult vr);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
