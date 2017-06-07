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
#ifndef V_READER_H
#define V_READER_H

#include "v_kernel.h"
#include "v_entity.h"
#include "v_status.h"
#include "v_readerSample.h"

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
 * \brief The <code>v_reader</code> cast method.
 *
 * This method casts an object to a <code>v_reader</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_reader</code> or
 * one of its subclasses.
 */
#define v_reader(o) (C_CAST(o,v_reader))

#define v_readerSubscriber(_this) \
        v_subscriber(v_reader(_this)->subscriber)

OS_API v_readerQos
v_readerGetQos(
    v_reader _this);

OS_API v_result
v_readerSetQos (
    v_reader _this,
    v_readerQos qos);

OS_API v_result
v_readerGetDeadlineMissedStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_readerGetIncompatibleQosStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_readerGetSampleRejectedStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_readerGetSampleLostStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_readerGetLivelinessChangedStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_readerGetSubscriptionMatchedStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API v_result
v_readerWaitForHistoricalData(
    v_reader _this,
    os_duration timeout);

OS_API v_result
v_readerWaitForHistoricalDataWithCondition(
    v_reader _this,
    const c_char* filter,
    const c_char* params[],
    c_ulong paramsLength,
    os_timeW minSourceTime,
    os_timeW maxSourceTime,
    c_long max_samples,
    c_long max_instances,
    c_long max_samples_per_instance,
    os_duration timeout);

OS_API void
v_readerNotifyStateChange(
    v_reader _this,
    c_bool complete);

OS_API c_bool
v_readerWalkEntries(
    v_reader _this,
    c_action action,
    c_voidp arg);

OS_API c_iter
v_readerCollectEntries(
    v_reader r);

OS_API c_iter
v_readerGetPartitions(
    v_reader _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
