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
#ifndef V_KERNEL_H
#define V_KERNEL_H

/** \file kernel/include/v_kernel.h
*  \brief This file defines the interface
*
*/

#include "kernelModule.h"

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

#define V_KERNEL_VERSION "Kernel 4.1.0"

#define C_REPORT(s) printf(s)

/**
* \brief The <code>c_object</code> cast method.
*
* This method casts an object to a <code>c_object</code> object.
*/
#define v_object(o)  ((v_object)(o))
/**
* \brief The <code>v_kernel</code> cast method.
*
* This method casts an object to a <code>v_kernel</code> object.
* Before the cast is performed, if compiled with the NDEBUG flag not set,
* the type of the object is checked to be <code>v_kernel</code> or
* one of its subclasses.
*/
#define v_kernel(o)  (C_CAST(o,v_kernel))
/**
* \brief The <code>v_context</code> cast method.
*
* This method casts an object to a <code>v_context</code> object.
* Before the cast is performed, if compiled with the NDEBUG flag not set,
* the type of the object is checked to be <code>v_context</code> or
* one of its subclasses.
*/
#define v_context(o) (C_CAST(o,v_context))

#define v_objectKernel(o) (v_kernel(v_object(o)->kernel))
#define v_objectKind(o)   (v_object(o)->kind)

#define v_kernelType(_this,_kind) (v_kernel(_this)->type[_kind])

typedef enum {
    V_RESULT_UNDEFINED,
    V_RESULT_OK,
    V_RESULT_INTERRUPTED,
    V_RESULT_NOT_ENABLED,
    V_RESULT_OUT_OF_MEMORY,
    V_RESULT_INTERNAL_ERROR,
    V_RESULT_ILL_PARAM,
    V_RESULT_CLASS_MISMATCH,
    V_RESULT_DETACHING,
    V_RESULT_TIMEOUT,
    V_RESULT_INCONSISTENT_QOS,
    V_RESULT_IMMUTABLE_POLICY,
    V_RESULT_PRECONDITION_NOT_MET,
    V_RESULT_ALREADY_DELETED,
    V_RESULT_UNSUPPORTED
} v_result;

OS_API c_bool
v_isEnabledStatistics (
    v_kernel _this,
    const char *categoryName);

OS_API v_kernel
v_kernelNew (
    c_base b,
    const c_char *name,
    v_kernelQos qos);

OS_API v_kernel
v_kernelAttach (
    c_base b,
    const c_char *name);

OS_API void
v_kernelDetach (
    v_kernel _this);

OS_API c_long
v_kernelUserCount (
    v_kernel _this);

OS_API v_partition
v_addPartition (
    v_kernel _this,
    v_partition p);

OS_API v_partition
v_removePartition (
    v_kernel _this,
    v_partition p);

OS_API c_iter
v_resolvePartitions (
    v_kernel _this,
    const c_char *name);

OS_API v_topic
v__addTopic (
    v_kernel _this,
    v_topic t);

OS_API v_topic
v_removeTopic (
    v_kernel _this,
    v_topic t);

OS_API v_topic
v_lookupTopic (
    v_kernel _this,
    const char *name);

OS_API c_iter
v_resolveTopics (
    v_kernel _this,
    const c_char *expr);

OS_API v_participant
v_addParticipant (
    v_kernel _this,
    v_participant p);

OS_API v_participant
v_removeParticipant (
    v_kernel _this,
    v_participant p);

OS_API c_iter
v_resolveParticipants (
    v_kernel _this,
    const c_char *name);

OS_API v_object
v_objectNew (
    v_kernel _this,
    v_kind kind);

OS_API v_object
v_new (
    v_kernel _this,
    c_type type);

OS_API v_serviceManager
v_getServiceManager (
    v_kernel _this);

OS_API v_configuration
v_getConfiguration (
    v_kernel _this);

OS_API v_configuration
v_setConfiguration (
    v_kernel _this,
    v_configuration config);

OS_API void
v_enableStatistics (
    v_kernel _this,
    const char *categoryName);

OS_API void
v_disableStatistics (
    v_kernel _this,
    const char *categoryName);

OS_API c_bool
v_kernelCheckHandleServer (
    v_kernel _this,
    c_address serverId);

OS_API void
v_writeBuiltinTopic (
    v_kernel _this,
    enum v_infoId id,
    v_message msg);
void
v_writeDisposeBuiltinTopic (
    v_kernel _this,
    enum v_infoId id,
    v_message msg);
void
v_unregisterBuiltinTopic (
    v_kernel _this,
    enum v_infoId id,
    v_message msg);

OS_API v_result
v_kernelCreatePersistentSnapshot(
    v_kernel _this,
    const c_char * partition_expression,
    const c_char * topic_expression,
    const c_char * uri);

OS_API v_accessMode
v_kernelPartitionAccessMode(
    v_kernel _this,
    v_partitionPolicy partition);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
