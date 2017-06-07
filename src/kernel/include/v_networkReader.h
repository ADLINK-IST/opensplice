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
#ifndef V_NETWORKREADER_H
#define V_NETWORKREADER_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_networkReader</code> cast method.
 *
 * This method casts an object to a <code>v_networkReader</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_networkReader</code> or
 * one of its subclasses.
 */
#define v_networkReader(o) (C_CAST(o,v_networkReader))

OS_API v_networkReader
v_networkReaderNew(
    v_subscriber subscriber,
    const c_char *name,
    v_readerQos qos,
    c_bool ignoreReliabilityQoS);

OS_API void
v_networkReaderFree(
    v_networkReader reader);

OS_API c_bool
v_networkReaderSubscribe(
    v_networkReader reader,
    v_partition partition);

OS_API c_bool
v_networkReaderUnSubscribe(
    v_networkReader reader,
    v_partition partition);

OS_API void
v_networkReaderDeinit(
    v_networkReader reader);

#define V_WAITRESULT_NONE         (0x00000000)
#define V_WAITRESULT_UNDEFINED    (0x00000001)
#define V_WAITRESULT_MSGWAITING   (V_WAITRESULT_UNDEFINED  << 1)
#define V_WAITRESULT_TIMEOUT      (V_WAITRESULT_MSGWAITING << 1)
#define V_WAITRESULT_TRIGGERED    (V_WAITRESULT_TIMEOUT    << 1)
#define V_WAITRESULT_FAIL         (V_WAITRESULT_TRIGGERED  << 1)

typedef c_ulong v_networkReaderWaitResult;

OS_API v_networkReaderWaitResult
v_networkReaderWait(
    v_networkReader reader,
    c_ulong queueId,
    v_networkQueue *queue);

OS_API v_networkReaderWaitResult
v_networkReaderWaitForTimeout(
    v_networkReader reader,
    c_ulong queueId,
    v_networkQueue *queue);

OS_API v_networkReaderWaitResult
v_networkReaderWaitDelayed(
    v_networkReader reader,
    c_ulong queueId,
    v_networkQueue *queue);

OS_API void
v_networkReaderTrigger(
    v_networkReader reader,
    c_ulong queueId);

OS_API void
v_networkReaderFree(
    v_networkReader reader);


/* Networking-specific functions */

/* Returns queueId */
OS_API c_ulong
v_networkReaderCreateQueue(
    v_networkReader reader,
    c_ulong queueSize,
    c_ulong priority,
    c_bool reliable,
    c_bool P2P,
    os_duration resolution,
    c_bool useAsDefault,
    const c_char *name);

OS_API v_networkReaderEntry
v_networkReaderLookupEntry(
    v_networkReader reader,
    v_group group);

OS_API void
v_networkReaderProbe(
    v_networkReader reader);

OS_API void
v_networkReaderRemoteActivityDetected(
    v_networkReader reader);

OS_API void
v_networkReaderRemoteActivityLost(
    v_networkReader reader);

#undef OS_API

#endif /* V_NETWORKREADER_H */
