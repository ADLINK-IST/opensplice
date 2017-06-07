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
#ifndef V_PUBLISHER_H
#define V_PUBLISHER_H

#include "v_kernel.h"
#include "v_writer.h"

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
 * \brief The <code>v_publisher</code> cast method.
 *
 * This method casts an object to a <code>v_publisher</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_publisher</code> or
 * one of its subclasses.
 */
#define v_publisher(o) (C_CAST(o,v_publisher))

#define v_publisherParticipant(_this) \
        v_participant(v_publisher(_this)->participant)

typedef void (*v_writerAction)(v_writer _this, c_voidp arg);

OS_API v_publisher
v_publisherNew (
    v_participant p,
    const c_char *name,
    v_publisherQos qos,
    c_bool enable);

OS_API void
v_publisherFree (
    v_publisher _this);

OS_API void
v_publisherDeinit (
    v_publisher _this);

OS_API v_publisherQos
v_publisherGetQos(
    v_publisher _this);

OS_API v_result
v_publisherSetQos(
    v_publisher _this,
    v_publisherQos qos);

OS_API v_result
v_publisherEnable (
    v_publisher _this);

OS_API void
v_publisherPublish (
    v_publisher _this,
    const c_char *partitionExpr);

OS_API void
v_publisherUnPublish (
    v_publisher _this,
    const c_char *partitionExpr);

OS_API c_iter
v_publisherLookupPartitions (
    v_publisher _this,
    const c_char *partitionExpr);

OS_API c_bool
v_publisherCheckPartitionInterest (
    v_publisher _this,
    v_partition partition);

OS_API v_result
v_publisherAddWriter (
    v_publisher _this,
    v_writer w);

OS_API v_result
v_publisherRemoveWriter (
    v_publisher _this,
    v_writer w);

OS_API c_long
v_publisherWriterCount (
    v_publisher _this);

OS_API c_iter
v_publisherLookupWriters (
    v_publisher _this,
    const c_char *topicExpr);

OS_API void
v_publisherWalkWriters(
    v_publisher _this,
    v_writerAction action,
    c_voidp arg);

OS_API void
v_publisherSuspend (
    v_publisher _this);

OS_API c_bool
v_publisherResume (
    v_publisher _this);

OS_API v_result
v_publisherCoherentBegin (
    v_publisher _this);

OS_API v_result
v_publisherCoherentEnd (
    v_publisher _this);

OS_API void
v_publisherNotifyNewGroup (
    v_publisher _this,
    v_group group);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
