/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef V_DATAWRITERINSTANCE_H
#define V_DATAWRITERINSTANCE_H

#include "v_kernel.h"
#include "v_state.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
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

#define v_writerInstanceSetState(_this,_state) \
        (v_stateSet(v_writerInstance(_this)->state,_state))

#define v_writerInstanceTestState(_this,_state) \
        (v_stateTest(v_writerInstance(_this)->state,_state))

#define v_writerInstanceResetState(_this,_state) \
        (v_stateClear(v_writerInstance(_this)->state,_state))

#define v_writerInstanceUnregister(_this) \
        (v_stateSet(v_writerInstance(_this)->state, L_UNREGISTER))

#define v_writerInstanceIsUnregistered(_this) \
        (v_stateTest(v_writerInstance(_this)->state, L_UNREGISTER))

#define v_writerInstanceWriter(_this) \
        (v_writer(v_writerInstance(_this)->writer))

typedef c_bool (*v_writerInstanceWalkAction)(v_writerSample sample, c_voidp arg);

OS_API v_writerInstance
v_writerInstanceNew(
    v_writer writer,
    v_message message);

OS_API void
v_writerInstanceInit (
    v_writerInstance instance,
    v_message message);

OS_API void
v_writerInstanceFree(
    v_writerInstance instance);

OS_API void
v_writerInstanceDeinit(
    v_writerInstance instance);

OS_API v_message
v_writerInstanceCreateMessage(
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

OS_API v_writerSample
v_writerInstanceTakeAll(
     v_writerInstance instance);

#undef OS_API

#endif
