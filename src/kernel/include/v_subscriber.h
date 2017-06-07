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
#ifndef V_SUBSCRIBER_H
#define V_SUBSCRIBER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_kernel.h"
#include "v_entity.h"
#include "v_partition.h"
#include "v_reader.h"
#include "v_participant.h"
#include "v_subscriberQos.h"
#include "v_dataReader.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_subscriber</code> cast methods.
 *
 * This method casts an object to a <code>v_subscriber</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_subscriber</code> or
 * one of its subclasses.
 */
#define v_subscriber(o) (C_CAST(o,v_subscriber))

#define v_subscriberParticipant(_this) \
        v_participant(_this->participant)

OS_API v_subscriber
v_subscriberNew(
    v_participant participant,
    const c_char *name,
    v_subscriberQos qos,
    c_bool enable);

OS_API void
v_subscriberFree(
    v_subscriber _this);

OS_API v_subscriberQos
v_subscriberGetQos(
    v_subscriber _this);

OS_API v_result
v_subscriberSetQos(
    v_subscriber _this,
    v_subscriberQos qos);

OS_API v_result
v_subscriberEnable(
    v_subscriber _this);

OS_API void
v_subscriberDeinit(
   v_subscriber _this);

OS_API void
v_subscriberSubscribe(
    v_subscriber _this,
    const c_char *partitionExpr);

OS_API c_iter
v_subscriberLookupPartitions(
    v_subscriber _this,
    const c_char *partitionExpr);

OS_API c_bool
v_subscriberCheckPartitionInterest(
    v_subscriber _this,
    v_partition partition);

OS_API v_result
v_subscriberAddReader(
    v_subscriber _this,
    v_reader reader);

OS_API void
v_subscriberRemoveReader(
    v_subscriber _this,
    v_reader reader);

OS_API c_iter
v_subscriberLookupReadersByTopic(
    v_subscriber _this,
    const c_char *topicName);

OS_API v_presentationKind
v_subscriberAccessScope(
    v_subscriber _this);

OS_API v_result
v_subscriberBeginAccess(
    v_subscriber _this);

OS_API v_result
v_subscriberEndAccess(
    v_subscriber _this);

typedef c_bool (*v_dataReaderAction)(v_dataReader reader, c_voidp arg);

OS_API v_result
v_subscriberGetDataReaders(
    v_subscriber _this,
    v_sampleMask mask,
    v_dataReaderAction action,
    c_voidp actionArg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
