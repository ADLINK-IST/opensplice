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
#ifndef U_SUBSCRIBER_H
#define U_SUBSCRIBER_H

#include "u_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_subscriber(o) \
        ((u_subscriber)u_objectCheckType(u_object(o),U_SUBSCRIBER))

typedef c_bool (*u_subscriberAction)(u_dataReader reader, void *arg);

OS_API u_subscriber
u_subscriberNew (
    const u_participant _scope,
    const os_char *name,
    const u_subscriberQos qos,
    u_bool enable);

OS_API u_result
u_subscriberGetQos (
    const u_subscriber _this,
    u_subscriberQos *qos);

OS_API u_result
u_subscriberSetQos (
    const u_subscriber _this,
    const u_subscriberQos qos);

OS_API u_result
u_subscriberBeginAccess(
    const u_subscriber _this);

OS_API u_result
u_subscriberEndAccess(
    const u_subscriber _this);

OS_API u_dataReader
u_subscriberCreateDataReader (
    const u_subscriber _this,
    const os_char *name,
    const os_char *expression,
    const c_value params[],
    const u_readerQos qos,
    u_bool enable);

OS_API u_result
u_subscriberGetDataReaders (
    const u_subscriber _this,
    u_sampleMask mask,
    c_iter *readers);

OS_API u_result
u_subscriberGetDataReaderProxies (
    const u_subscriber _this,
    u_sampleMask mask,
    c_iter *readers);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
