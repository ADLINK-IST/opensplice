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

#ifdef OSPL_BUILD_KERNEL
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
v_readerGetTopicMatchStatus(
    v_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API c_bool
v_readerWaitForHistoricalData(
    v_reader _this,
    c_time timeout);

OS_API v_historyResult
v_readerWaitForHistoricalDataWithCondition(
    v_reader _this,
    c_char* filter,
    c_char* params[],
    c_ulong paramsLength,
    c_time minSourceTime,
    c_time maxSourceTime,
    struct v_resourcePolicy *resourceLimits,
    c_time timeout);

OS_API void
v_readerNotifyHistoricalDataAvailable(
    v_reader _this);

OS_API c_bool
v_readerWalkEntries(
    v_reader _this,
    c_action action,
    c_voidp arg);

OS_API c_iter
v_readerCollectEntries(
    v_reader r);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
