/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef V_DATAREADER_H
#define V_DATAREADER_H

/** \file kernel/include/v_dataReader.h
 *  \brief This file defines the interface of DataReader objects.
 *
 * Objects implement a data storage of instances and data samples.
 * This interface provides read access to instance and samples
 * and provides acces to meta data on status and data.
 * The data is inserted into the storage by the kernel.
 *
 */

#include "v_kernel.h"
#include "v_reader.h"
#include "v_readerQos.h"
#include "v_dataReaderSample.h"

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

typedef c_bool (*v_dataReaderInstanceAction)(v_dataReaderInstance instance, c_voidp arg);
OS_API const char*
v_dataReaderResultString(
    v_dataReaderResult result);

/**
 * \brief The <code>v_dataReader</code> cast method.
 *
 * This method casts an object to a <code>v_dataReader</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dataReader</code> or
 * one of its subclasses.
 */
#define v_dataReader(o) (C_CAST(o,v_dataReader))


OS_API c_bool
v_dataReaderWalkInstances (
    v_dataReader _this,
    v_dataReaderInstanceAction action,
    c_voidp arg);

OS_API v_dataReader
v_dataReaderNew(
    v_subscriber subscriber,
    const c_char *name,
    q_expr OQLexpr,
    c_value params[],
    v_readerQos qos,
    c_bool enable);

OS_API void
v_dataReaderFree(
    v_dataReader _this);

OS_API v_result
v_dataReaderEnable(
    v_dataReader _this);

OS_API c_type
v_dataReaderInstanceType(
    v_dataReader _this);

OS_API c_type
v_dataReaderSampleType(
    v_dataReader _this);

OS_API v_topic
v_dataReaderGetTopic(
    v_dataReader _this);

OS_API v_dataReaderInstance
v_dataReaderLookupInstance (
    v_dataReader _this,
    v_message keyTemplate);

OS_API c_bool
v_dataReaderContainsInstance (
    v_dataReader _this,
    v_dataReaderInstance instance);

OS_API c_bool
v_dataReaderRead(
    v_dataReader _this,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataReaderTake(
    v_dataReader _this,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataReaderReadInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataReaderTakeInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataReaderReadNextInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_bool
v_dataReaderTakeNextInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

OS_API c_long
v_dataReaderNotReadCount(
    v_dataReader _this);

OS_API v_result
v_dataReaderSetNotReadThreshold(
    v_dataReader _this,
    c_long threshold);

OS_API c_bool
v_dataReaderUpdateSampleLost(
    v_reader _this,
    c_ulong missedSamples);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
