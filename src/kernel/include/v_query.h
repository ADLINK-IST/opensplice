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
#ifndef V_QUERY_H
#define V_QUERY_H

#include "v_kernel.h"
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
 * \brief The <code>v_query</code> cast method.
 *
 * This method casts an object to a <code>v_query</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_query</code> or
 * one of its subclasses.
 */
#define v_query(o) (C_CAST(o,v_query))

typedef c_bool (v_queryAction)(c_object o, c_voidp arg);

OS_API v_query
v_queryNew(
    v_collection source,
    const os_char *name,
    const os_char *predicate,
    const os_char *params[],
    const os_uint32 nrOfParams,
    const os_uint32 sampleMask);

OS_API void
v_queryFree(
    v_query _this);

OS_API void
v_queryDeinit(
    v_query _this);

void
v_queryEnableStatistics(
    v_query _this,
    os_boolean enable);

OS_API v_result
v_queryRead(
    v_query _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_queryTake(
    v_query _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_queryReadInstance(
    v_query _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_queryTakeInstance(
    v_query _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_queryReadNextInstance(
    v_query _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_queryTakeNextInstance(
    v_query _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API c_bool
v_queryTest(
    v_query _this,
    v_queryAction action,
    c_voidp args);

OS_API c_bool
v_queryTriggerTest(
    v_query _this);

OS_API v_collection
v_querySource(
    v_query _this);

OS_API c_bool
v_querySetParams(
    v_query _this,
    const os_char *params[],
    const os_uint32 nrOfParams);

OS_API c_bool
v_queryTestSample(
    v_query _this,
    v_readerSample sample);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_QUERY_H */
