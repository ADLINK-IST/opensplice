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
#ifndef V_DATAVIEWINSTANCE_H
#define V_DATAVIEWINSTANCE_H

/** \file kernel/include/v_dataViewInstance.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"
#include "v_dataViewSample.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
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
     v_readerSampleAction action,
     c_voidp arg);

OS_API c_bool
v_dataViewInstanceTakeSamples(
     v_dataViewInstance _this,
     c_query query,
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
     c_query query);

#undef OS_API              

#endif
