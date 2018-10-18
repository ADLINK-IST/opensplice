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
#ifndef U_READER_H
#define U_READER_H


#if defined (__cplusplus)
extern "C" {
#endif
#include "v_readerSample.h"

typedef v_actionResult u_actionResult;       /* unsigned long */
typedef void *u_readerActionArg;

typedef u_actionResult
(*u_readerAction)(
    c_object o,
    u_readerActionArg copyArg);

/* Because of the include chain and the availability of u_readerAction;
 * we need to include u_entity here, after u_readerAction declaration.
 * To prevent [extern "C"] nesting, we do it outside that. */
#if defined (__cplusplus)
}
#endif
#include "u_entity.h"
#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_reader(o) ((u_reader)(o))

OS_API u_result
u_readerGetDeadlineMissedStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_readerGetIncompatibleQosStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_readerGetSampleRejectedStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_readerGetLivelinessChangedStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_readerGetSampleLostStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_readerGetSubscriptionMatchStatus(
    const u_reader _this,
    u_bool reset,
    u_statusAction action,
    void *arg);

OS_API u_result
u_readerGetMatchedPublications (
    const u_reader _this,
    u_publicationInfo_action action,
    void *arg);

OS_API u_result
u_readerGetMatchedPublicationData (
    const u_reader _this,
    u_instanceHandle publication_handle,
    u_publicationInfo_action action,
    void *arg);

OS_API u_result
u_readerRead (
    const u_reader _this,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

OS_API u_result
u_readerTake (
    const u_reader _this,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

OS_API u_result
u_readerReadInstance (
    const u_reader _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

OS_API u_result
u_readerTakeInstance (
    const u_reader _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

OS_API u_result
u_readerReadNextInstance (
    const u_reader _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

OS_API u_result
u_readerTakeNextInstance (
    const u_reader _this,
    u_instanceHandle handle,
    u_sampleMask mask,
    u_readerAction action,
    void *actionArg,
    const os_duration timeout);

/** \brief Protect against process termination during a copy-out operation.
 *
 * This method is used to suspend the termination request of the domain
 * during the copying of the data to protect the application for data
 * corruption. When the copy-out action is finished
 * u_readerProtectCopyOutExit must be called.
 *
 * Note that this call may only be used when reading from shared resources.
 */
OS_API u_result
u_readerProtectCopyOutEnter(
    u_entity _this);

/** \brief Unprotect against process termination after a copy-out operation.
 *
 * This method is used to release the protection of the domain when the
 * copy-out operation has finished which was set by a call to
 * u_readerProtectCopyOutEnter
 */
OS_API void
u_readerProtectCopyOutExit(
    u_entity _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
