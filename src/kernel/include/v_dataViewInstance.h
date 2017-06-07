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
#ifndef V_DATAVIEWINSTANCE_H
#define V_DATAVIEWINSTANCE_H

/** \file kernel/include/v_dataViewInstance.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_dataViewSample.h"
#include "os_if.h"
#include "v_query.h"
#include "v_dataReaderInstance.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_dataViewInstance</code> cast method.
 *
 * This method casts an object to a <code>v_dataViewInstance</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataViewInstance</code> or
 * one of its subclasses.
 */
#define v_dataViewInstance(_this)         (C_CAST(_this,v_dataViewInstance))

#define v_dataViewInstanceTemplate(_this) ((v_dataViewInstanceTemplate)(_this))

#define v_dataViewInstanceEmpty(_this) \
        (v_dataViewInstance(_this)->sampleCount == 0)

#define v_dataViewInstanceSampleCount(_this) \
        (v_dataViewInstance(_this)->sampleCount)

#define v_dataViewInstanceState(_this) \
        (v_dataViewInstance(_this)->instanceState)

#define v_dataViewInstanceView(_this) \
        (v_dataViewInstance(_this)->dataView)


typedef c_bool 
(*v_dataViewInstanceAction)(
    v_dataViewInstance _this,
    c_voidp arg);

OS_API v_dataViewInstance
v_dataViewInstanceNew(
     v_dataView dataView,
     v_readerSample sample);

OS_API void
v_dataViewInstanceDeinit(
     v_dataViewInstance _this);

OS_API v_writeResult
v_dataViewInstanceWrite (
     v_dataViewInstance _this,
     v_readerSample sample);

OS_API void
v_dataViewInstanceWipe(
    v_dataViewInstance _this);

OS_API v_dataViewSample
v__dataViewInstanceWrite(
    v_dataViewInstance instance,
    v_dataViewSample sample,
    v_dataViewSample position);

OS_API void
v_dataViewInstanceRemove(
     v_dataViewInstance _this);

/** Getter and setter for users of the kernel */
OS_API c_voidp
v_dataViewInstanceGetUserData(
     v_dataViewInstance _this);

OS_API void
v_dataViewInstanceSetUserData(
     v_dataViewInstance _this,
     c_voidp userData);

/** New methods that will replace depricated methods */

OS_API c_bool
v_dataViewInstanceReadSamples(
     v_dataViewInstance _this,
     c_query query,
     v_state sampleMask,
     v_readerSampleAction action,
     c_voidp arg);

OS_API c_bool
v_dataViewInstanceTakeSamples(
     v_dataViewInstance _this,
     c_query query,
     v_state sampleMask,
     v_readerSampleAction action,
     c_voidp arg);

OS_API void
v_dataViewInstanceWalkSamples(
     v_dataViewInstance _this,
     v_readerSampleAction action,
     c_voidp arg);

OS_API c_bool
v_dataViewInstanceRemoveSample(
     v_dataViewInstance _this,
     v_dataViewSample sample);

OS_API void
v_dataViewInstancePurge(
     v_dataViewInstance _this);

OS_API c_bool
v_dataViewInstanceTest(
     v_dataViewInstance _this,
     c_query query,
     v_state sampleMask,
     v_queryAction action,
     c_voidp args);

OS_API v_actionResult
v_dataViewSampleReadTake(
    v_dataViewSample sample,
    v_readerSampleAction action,
    c_voidp arg,
    c_bool consume);

#undef OS_API              

#endif
