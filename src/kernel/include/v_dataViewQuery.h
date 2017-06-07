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
#ifndef V_DATAVIEWQUERY_H
#define V_DATAVIEWQUERY_H

/** \file kernel/include/v_dataViewQuery.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_event.h"
#include "v_dataView.h"
#include "v_dataViewSample.h"
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
 * \brief The <code>v_dataViewQuery</code> cast method.
 *
 * This method casts an object to a <code>v_dataViewQuery</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataViewQuery</code> or
 * one of its subclasses.
 */
#define v_dataViewQuery(q) (C_CAST(q,v_dataViewQuery))

OS_API v_dataViewQuery
v_dataViewQueryNew(
    v_dataView view,
    const os_char *name,
    const os_char *expression,
    const os_char *params[],
    const os_uint32 nrOfParams,
    const os_uint32 sampleMask);

OS_API void
v_dataViewQueryFree(
    v_dataViewQuery _this);

OS_API c_bool
v_dataViewQueryTest(
    v_dataViewQuery _this,
    v_queryAction action,
    c_voidp args);

OS_API v_result
v_dataViewQueryRead(
    v_dataViewQuery _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataViewQueryTake(
    v_dataViewQuery _this,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataViewQueryReadInstance(
    v_dataViewQuery _this,
    v_dataViewInstance i,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataViewQueryTakeInstance(
    v_dataViewQuery _this,
    v_dataViewInstance i,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataViewQueryReadNextInstance(
    v_dataViewQuery _this,
    v_dataViewInstance i,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataViewQueryTakeNextInstance(
    v_dataViewQuery _this,
    v_dataViewInstance i,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API void
v_dataViewQueryDeinit(
    v_dataViewQuery _this);

OS_API c_bool
v_dataViewQueryNotifyDataAvailable(
    v_dataViewQuery _this,
    v_event e);

OS_API c_bool
v_dataViewQuerySetParams(
    v_dataViewQuery _this,
    const os_char *params[],
    const os_uint32 nrOfParams);

OS_API c_bool
v_dataViewQueryTestSample(
    v_dataViewQuery _this,
    v_dataViewSample sample);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
