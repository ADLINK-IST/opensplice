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
#include "v_builtin.h"

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
    _In_ v_subscriber subscriber,
    _In_opt_z_ const c_char *name,
    _In_opt_ q_expr OQLexpr,
    const c_value params[],
    os_uint32 nrOfParams,
    _In_opt_ v_readerQos qos);

OS_API v_dataReader
v_dataReaderNewBySQL (
    v_subscriber subscriber,
    const os_char *name,
    const os_char *expr,
    const c_value params[],
    os_uint32 nrOfParams,
    v_readerQos qos);

OS_API void
v_dataReaderFree(
    v_dataReader _this);

OS_API v_result
v_dataReaderEnable(
    _Inout_ v_dataReader _this);

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

OS_API v_result
v_dataReaderRead(
    v_dataReader _this,
    v_sampleMask mask,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataReaderTake(
    v_dataReader _this,
    v_sampleMask mask,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataReaderReadInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_sampleMask mask,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataReaderTakeInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_sampleMask mask,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataReaderReadNextInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_sampleMask mask,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API v_result
v_dataReaderTakeNextInstance(
    v_dataReader _this,
    v_dataReaderInstance instance,
    v_sampleMask mask,
    v_readerSampleAction action,
    c_voidp arg,
    os_duration timeout);

OS_API c_long
v_dataReaderNotReadCount(
    v_dataReader _this);

OS_API c_ulong
v_dataReaderGetNumberOpenTransactions(
    v_dataReader _this);

OS_API v_result
v_dataReaderSetQos(
    v_dataReader _this,
    v_readerQos qos);

/* This operation will visit all discovered matching publications for this dataReader.
 * The given action routine will be invoked on each publication info message.
 * The signature of the action routine : v_result (*action)(const v_publicationInfo *info, void *arg)
 * Issue: don't like operating on info as being an attribute of a message,
 *        better visit the whole message then it can also be returned as kept ref.
 */
OS_API v_result
v_dataReaderReadMatchedPublications(
    v_dataReader _this,
    v_publicationInfo_action action,
    c_voidp arg);

/* This operation will visit the discovered matching publication for this dataReader identified by the given GID.
 * The given action routine will be invoked on the GID associated publication info message.
 * The signature of the action routine : v_result (*action)(const v_publicationInfo *info, void *arg)
 * Issue: don't like operating on info as being an attribute of a message,
 *        better visit the whole message then it can also be returned as kept ref.
 */
OS_API v_result
v_dataReaderReadMatchedPublicationData(
    v_dataReader _this,
    v_gid publication,
    v_publicationInfo_action action,
    c_voidp arg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
