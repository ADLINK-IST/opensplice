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
 * The following methods are provided:
 *
 *    u_writer u_writerNew              (u_publisher p, const c_char *name,
 *                                       u_topic t, u_writerCopy copy,
 *                                       v_writerQos qos);
 *
 *    u_result u_writerFree             (u_writer _this);
 *
 *    u_result u_writerWrite            (u_writer _this, void *data,
 *                                       c_time timestamp, u_instanceHandle h);
 *
 *    u_result u_writerDispose          (u_writer _this, void *data,
 *                                       c_time timestamp, u_instanceHandle h);
 *
 *    u_result u_writerWriteDispose     (u_writer _this, void *data,
 *                                       c_time timestamp, u_instanceHandle h);
 *
 *    u_result u_writerAssertLiveliness (u_writer _this);
 *
 *    c_bool   u_writerDefaultCopy      (c_type type, void *data, void *to);
 */

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"

/**
 * brief The writers copy method prototype.
 *
 * This prototype specifies the signature of the method the writer will use
 * to transform application data into kernel data.
 * This method is provided by the application.
 */
typedef c_bool (*u_writerCopy)(c_type type, void *data, void *to);
typedef c_bool (*u_writerAction)(u_writer writer, c_voidp arg);

#include "u_instanceHandle.h"
#include "v_status.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_writer(w) \
        ((u_writer)u_entityCheckType(u_entity(w), U_WRITER))

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
    u_publisher publisher,
    const c_char *name,
    u_topic topic,
    u_writerCopy copy,
    v_writerQos qos,
    c_bool enable);

OS_API u_result
u_writerInit(
    u_writer _this,
    u_publisher publisher,
    const c_char *name,
    u_writerCopy copy);

OS_API u_result
u_writerFree (
    u_writer _this);

OS_API u_publisher
u_writerPublisher(
    u_writer _this);

OS_API u_result
u_writerWrite (
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle handle);

OS_API u_result
u_writerDispose (
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle handle);

OS_API u_result
u_writerWriteDispose (
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle handle);

OS_API u_result
u_writerRegisterInstance (
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle *handle);

OS_API u_result
u_writerRegisterInstanceTMP(
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle *handle,
    u_writerCopy copy);

OS_API u_result
u_writerUnregisterInstance(
    u_writer _this,
    void *data,
    c_time timestamp,
    u_instanceHandle handle);

OS_API u_result
u_writerAssertLiveliness (
    u_writer _this);

OS_API u_result
u_writerCopyKeysFromInstanceHandle (
    u_writer _this,
    u_instanceHandle handle,
    u_writerAction action,
    void *copyArg);

/* The default copy method is only used for DBT test purposes. */

OS_API c_bool
u_writerDefaultCopy (
    c_type type,
    void *data,
    void *to);

OS_API u_result
u_writerLookupInstance (
    u_writer _this,
    void *keyTemplate,
    u_instanceHandle *handle);

OS_API u_result
u_writerGetLivelinessLostStatus (
    u_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_writerGetDeadlineMissedStatus (
    u_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_writerGetIncompatibleQosStatus (
    u_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_writerGetPublicationMatchStatus (
    u_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_writerGetMatchedSubscriptions (
    u_writer _this,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_writerGetMatchedSubscriptionData (
    u_writer _this,
    u_instanceHandle subscription_handle,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_writerWaitForAcknowledgments(
    u_writer _this,
    c_time timeout);

OS_API c_char *
u_writerTopicName (
    u_writer _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
