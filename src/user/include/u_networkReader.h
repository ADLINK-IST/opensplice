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
#ifndef U_NETWORKREADER_H
#define U_NETWORKREADER_H

#include "u_types.h"
#include "u_reader.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#if defined (__cplusplus)
extern "C" {
#endif

#define u_networkReader(o) \
        ((u_networkReader)u_objectCheckType(u_object(o), U_NETWORKREADER))

OS_API u_networkReader
u_networkReaderNew(
    const u_subscriber s,
    const os_char *name,
    const u_readerQos qos,
    u_bool ignoreReliabilityQoS);
    
OS_API u_result        
u_networkReaderCreateQueue(
    const u_networkReader _this,
    os_uint32 queueSize,
    os_uint32 priority,
    u_bool reliable,
    u_bool P2P,
    os_duration resolution,
    u_bool useAsDefault,
    os_uint32 *queueId, /* out */
    const os_char *name);
    
OS_API u_result        
u_networkReaderTrigger(
    const u_networkReader _this,
    os_uint32 queueId);
    
OS_API u_result        
u_networkReaderRemoteActivityDetected(
    const u_networkReader _this);
    
OS_API u_result        
u_networkReaderRemoteActivityLost(
    const u_networkReader _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
