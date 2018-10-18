/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef V_DATAVIEW_H
#define V_DATAVIEW_H

/** \file kernel/include/v_dataView.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_reader.h"

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
 * \brief The <code>v_dataView</code> cast method.
 *
 * This method casts an object to a <code>v_dataView</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataView</code> or
 * one of its subclasses.
 */
#define v_dataView(o) (C_CAST(o,v_dataView))
/**
 * \brief The <code>v_dataViewSample</code> cast method.
 *
 * This method casts an object to a <code>v_dataViewSample</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataViewSample</code> or
 * one of its subclasses.
 */
#define v_dataViewSample(_this) (C_CAST(_this,v_dataViewSample))

#define v_dataViewInstance(_this) (C_CAST(_this,v_dataViewInstance))
#define v_dataViewInstanceTemplate(_this) ((v_dataViewInstanceTemplate)(_this))
#define v_dataViewSampleTemplate(_this) ((v_dataViewSampleTemplate)(_this))

#define v_dataView_t(scope) \
        c_type(c_resolve(c_getBase(scope), \
                          "kernelModuleI::v_dataView"))

OS_API v_dataView
v_dataViewNew(
    v_dataReader reader,
    const c_char *name,
    v_dataViewQos qos,
    c_bool enable);

OS_API void
v_dataViewFree(
    v_dataView dataView);

OS_API v_dataViewQos
v_dataViewGetQos(
    v_dataView _this);

OS_API v_result
v_dataViewSetQos(
    v_dataView _this,
    v_dataViewQos qos);

OS_API v_dataReader
v_dataViewGetReader(
    v_dataView dataView);

OS_API v_actionResult
v_dataViewWrite(
    v_dataView dataView,
    v_readerSample sample);

OS_API v_result
v_dataViewRead(
    v_dataView dataView,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataViewTake(
    v_dataView dataView,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataViewReadInstance(
    v_dataView dataView,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataViewTakeInstance(
    v_dataView dataView,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataViewReadNextInstance(
    v_dataView dataView,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataViewTakeNextInstance(
    v_dataView dataView,
    v_dataViewInstance instance,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_dataViewInstance
v_dataViewLookupInstance (
    v_dataView view,
    v_message keyTemplate);

OS_API c_bool
v_dataViewContainsInstance (
    v_dataView view,
    v_dataViewInstance instance);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
