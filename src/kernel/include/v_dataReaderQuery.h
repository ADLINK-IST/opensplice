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
#ifndef V_DATAREADERQUERY_H
#define V_DATAREADERQUERY_H

/** \file kernel/include/v_dataReaderQuery.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_event.h"
#include "v_dataReader.h"
#include "v_dataReaderSample.h"
#include "v_query.h"

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
 * \brief The <code>v_dataReaderQuery</code> cast method.
 *
 * This method casts an object to a <code>v_dataReaderQuery</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataReaderQuery</code> or
 * one of its subclasses.
 */
#define v_dataReaderQuery(q) (C_CAST(q,v_dataReaderQuery))

OS_API v_dataReaderQuery
v_dataReaderQueryNew(
    v_dataReader reader,
    const os_char *name,
    const os_char *expression,
    const os_char *params[],
    const os_uint32 nrOfParams,
    const os_uint32 sampleMask);

OS_API void
v_dataReaderQueryFree(
    v_dataReaderQuery _this);

OS_API c_bool
v_dataReaderQueryTest(
    v_dataReaderQuery _this,
    v_queryAction action,
    c_voidp args);

OS_API c_bool
v_dataReaderQueryTriggerTest(
    v_dataReaderQuery _this);

OS_API v_result
v_dataReaderQueryRead(
    v_dataReaderQuery _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataReaderQueryTake(
    v_dataReaderQuery _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataReaderQueryReadInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance i,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataReaderQueryTakeInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance i,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataReaderQueryReadNextInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance i,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataReaderQueryTakeNextInstance(
    v_dataReaderQuery _this,
    v_dataReaderInstance i,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API void
v_dataReaderQueryDeinit(
    v_dataReaderQuery query);

c_bool
v_dataReaderQueryNotifyDataAvailable(
    v_dataReaderQuery _this,
    v_event e);

OS_API c_bool
v_dataReaderQuerySetParams(
    v_dataReaderQuery _this,
    const os_char *params[],
    const os_uint32 nrOfParams);

OS_API c_bool
v_dataReaderQueryTestSample(
    v_dataReaderQuery _this,
    v_dataReaderSample sample);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
