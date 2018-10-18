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
#ifndef V__DATAVIEWINSTANCE_H
#define V__DATAVIEWINSTANCE_H

/** \file kernel/code/v__dataViewInstance.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_dataView.h"
#include "v__dataViewSample.h"
#include "os_if.h"
#include "v_query.h"
#include "v_dataReaderInstance.h"

/**
 * \brief The <code>v_dataViewInstance</code> cast method.
 *
 * This method casts an object to a <code>v_dataViewInstance</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataViewInstance</code> or
 * one of its subclasses.
 */
#define v_dataViewInstanceEmpty(_this) \
        (v_dataViewInstance(_this)->sampleCount == 0)

typedef c_bool 
(*v_dataViewInstanceAction)(
    v_dataViewInstance _this,
    c_voidp arg);

v_dataViewInstance
v_dataViewInstanceNew(
     v_dataView dataView,
     v_dataViewSample sample);

void
v_dataViewInstanceDeinit(
     v_dataViewInstance _this);

void
v_dataViewInstanceWipe(
    v_dataViewInstance _this);

void
v_dataViewInstanceWrite(
    v_dataViewInstance instance,
    v_dataViewSample sample,
    v_dataViewSample position);

void
v_dataViewInstanceRemove(
     v_dataViewInstance _this);

c_bool
v_dataViewInstanceReadSamples(
     v_dataViewInstance _this,
     c_query query,
     v_state sampleMask,
     v_readerSampleAction action,
     c_voidp arg);

c_bool
v_dataViewInstanceTakeSamples(
     v_dataViewInstance _this,
     c_query query,
     v_state sampleMask,
     v_readerSampleAction action,
     c_voidp arg);

c_bool
v_dataViewInstanceTest(
     v_dataViewInstance _this,
     c_query query,
     v_state sampleMask,
     v_queryAction action,
     c_voidp args);

v_actionResult
v_dataViewSampleReadTake(
    v_dataViewSample sample,
    v_readerSampleAction action,
    c_voidp arg,
    c_bool consume);

#endif
