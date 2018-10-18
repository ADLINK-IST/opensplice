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
#ifndef V_DATAWRITERINSTANCE_H
#define V_DATAWRITERINSTANCE_H

#include "v_kernel.h"
#include "v_state.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_writerInstance</code> cast methods.
 *
 * This method casts an object to a <code>v_writerInstance</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_writerInstance</code> or
 * one of its subclasses.
 */
#define v_writerInstance(_this) (C_CAST(_this,v_writerInstance))

#define v_writerInstanceTemplate(_this) ((v_writerInstanceTemplate)(_this))

#define v_writerInstanceHead(_this) \
        (v_writerSample(v_writerInstanceTemplate(_this)->sample))

#define v_writerInstanceSetHead(_this,_sample) \
        (v_writerInstanceTemplate(_this)->sample = \
         c_keep(v_writerSample(_sample)))

#define v_writerInstanceTail(_this) \
        (v_writerSample(v_writerInstance(_this)->last))

#define v_writerInstanceSetTail(_this,_sample) \
        (v_writerInstance(_this)->last = v_writerSample(_sample))

#define v_writerInstanceState(_this) \
        (v_instanceState(_this))

#define v_writerInstanceSetState(_this,_state) \
        (v_stateSet(v_instanceState(_this),_state))

#define v_writerInstanceTestState(_this,_state) \
        (v_stateTest(v_instanceState(_this),_state))

#define v_writerInstanceResetState(_this,_state) \
        (v_stateClear(v_instanceState(_this),_state))

#define v_writerInstanceWriter(_this) \
        (v_writer(v_instanceEntity(_this)))

/* Evaluates to TRUE (c_bool) when the writerInstance has a pending resend for. */
#define v__writerInstanceHasResendsPending(i) ((c_bool)(v_writerInstanceTail(i) != NULL))

typedef c_bool (*v_writerInstanceWalkAction)(v_writerSample sample, c_voidp arg);

OS_API v_writerInstance
v_writerInstanceNew(
    v_writer writer);

OS_API void
v_writerInstanceInit (
    v_writerInstance instance,
    v_writer writer);

OS_API void
v_writerInstanceFree(
    v_writerInstance instance);

OS_API void
v_writerInstanceDeinit(
    v_writerInstance instance);

OS_API void
v_writerInstanceSetKey (
    v_writerInstance instance,
    v_message message);

OS_API v_message
v_writerInstanceCreateMessage(
    v_writerInstance _this);

OS_API v_message
v_writerInstanceCreateMessage_s(
    v_writerInstance _this);

OS_API v_writerSample
v_writerInstanceInsert(
    v_writerInstance instance,
    v_writerSample sample);

OS_API v_writerSample
v_writerInstanceRemove(
    v_writerInstance instance,
    v_writerSample sample);

OS_API c_bool
v_writerInstanceWalk(
    v_writerInstance instance,
    v_writerInstanceWalkAction action,
    c_voidp arg);

#undef OS_API

#endif
