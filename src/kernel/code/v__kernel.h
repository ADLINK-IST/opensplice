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
#ifndef V__KERNEL_H
#define V__KERNEL_H

#include "v_kernel.h"

#include "os_if.h"

#if 0
#define TRACE_IGNORE printf("## TRACE(Ignore) : "); printf
#else
#define TRACE_IGNORE(...)
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#define v_kernelGetQos(_this) \
        (c_keep(_this->qos))

v_transactionGroupAdmin
v_kernelTransactionGroupAdmin(
    v_kernel _this);

void
v_kernelGroupTransactionFlush(
    v_kernel _this,
    v_transactionAdmin admin);

c_bool
v_kernelGroupTransactionTryLockAccess(
    v_kernel _this);

void
v_kernelGroupTransactionUnlockAccess(
    v_kernel _this);

void
v_kernelNotifyCoherentPublication(
    v_kernel _this,
    v_message msg);

v_subscriber
v_kernelAddSharedSubscriber (
    v_kernel _this,
    v_subscriber subscriber);

c_ulong
v_kernelRemoveSharedSubscriber (
    v_kernel _this,
    v_subscriber subscriber);

void
v_checkMaxSamplesPerInstanceWarningLevel(
    v_kernel _this,
    c_ulong count);

void
v_checkMaxSamplesWarningLevel(
    v_kernel _this,
    c_ulong count);

void
v_checkMaxInstancesWarningLevel(
    v_kernel _this,
    c_ulong count);

v_result
v_condWait (
    c_cond *cnd,
    c_mutex *mtx,
    const os_duration delay);

/* This call increases the waitcount to signal that a thread is blocked in SHM.
 *
 * @pre v_kernelProtect(...) has been invoked successfully and its accompanying
 * call to v_kernelUnprotect() hasn't been done yet.
 */
_When_(mtx, _Requires_lock_held_(mtx->mtx))
_Pre_satisfies_((mtx && cnd) || (!mtx && !cnd))
OS_API   /* OS_API is included for this call, because it is used in a test.    */
void     /* It should NEVER be invoked otherwise from external context.        */
v__kernelProtectWaitEnter(
    _In_opt_ c_cond *cnd,
    _Inout_opt_ c_mutex *mtx);

/* This call decreases the waitcount to signal that a thread is not blocked in
 * SHM anymore.
 *
 * @pre v__kernelProtectWaitEnter() has been invoked.
 */
void
v__kernelProtectWaitExit(void);

v_spliced
v_kernelGetSpliced(
    v_kernel _this);

v_typeRepresentation
v__addTypeRepresentation (
    v_kernel kernel,
    v_typeRepresentation tr);

v_typeRepresentation
v__removeRepresentation (
    v_kernel kernel,
    v_typeRepresentation tr);

v_typeRepresentation
v__lookupTypeRepresentation (
    v_kernel kernel,
    const c_string typeName,
    v_dataRepresentationId_t dataRepresentationId,
    const v_typeHash typeHash);

v_result
v_kernelDisposeAllData(
    v_kernel kernel,
    c_string partitionExpr,
    c_string topicExpr,
    os_timeW timestamp);

#define v_rxoData(o) (C_CAST(o,v_rxoData))

v_rxoData
v_kernel_rxoDataFromMessageQos(
    v_kernel kernel,
    const v_messageQos qos);

v_rxoData
v_kernel_rxoDataFromPublicationInfo(
    v_kernel kernel,
    const struct v_publicationInfo *info);

v_rxoData
v_kernel_rxoDataFromReaderQos(
    v_kernel kernel,
    const v_readerQos qos);

c_bool
v_rxoDataCompatible(
    v_rxoData offered,
    v_rxoData requested);

/* connect group to all matching readers */
void
v_kernelConnectGroup(
    v_kernel _this,
    v_group g);

/* This operation will update the kernel according to a newly discovered publication and
 * notify matching local subscriptions (synchronously).
 * The given message holds the builtin publication data describing the discovered writer.
 */
void
v_kernelNotifyPublication(
    v_kernel _this,
    v_message msg);

/* This operation will update the kernel according to a newly discovered subscription and
 * notify matching local publications (synchronously).
 * The given message holds the builtin subscription data describing the discovered reader.
 */
void
v_kernelNotifySubscription(
    v_kernel _this,
    v_message msg);

/* This operation will lookup the publication message identified by the given gid.
 * If found the message will be returned and must be freed after use.
 */
v_message
v_kernelLookupPublication(
    v_kernel _this,
    v_gid gid);

#endif /* V__KERNEL_H */
