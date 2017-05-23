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

#ifndef V__STATUS_H
#define V__STATUS_H

#include "v_status.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

enum v_statusLiveliness {
    V_STATUSLIVELINESS_UNKNOWN,
    V_STATUSLIVELINESS_ALIVE,
    V_STATUSLIVELINESS_NOTALIVE,
    V_STATUSLIVELINESS_DELETED,
    V_STATUSLIVELINESS_COUNT /* always the last! */
};

OS_API void
v_statusNotifyInconsistentTopic (
    v_status _this);

OS_API void
v_statusNotifyAllDataDisposed (
    v_status _this);

OS_API void
v_statusNotifyDataOnReaders(
    v_status _this);

OS_API void
v_statusNotifyDataAvailable (
    v_status _this);

OS_API void
v_statusNotifySampleLost (
    v_status _this,
    c_ulong nrSamplesLost);

OS_API void
v_statusNotifyLivelinessLost (
    v_status _this);

OS_API void
v_statusNotifyOfferedDeadlineMissed (
    v_status _this,
    v_handle instanceHandle);

OS_API void
v_statusNotifyRequestedDeadlineMissed (
    v_status _this,
    v_handle instanceHandle);

OS_API void
v_statusNotifyOfferedIncompatibleQos (
    v_status _this,
    v_policyId id);

OS_API void
v_statusNotifyRequestedIncompatibleQos (
    v_status _this,
    v_policyId id);

OS_API void
v_statusNotifyPublicationMatched(
    v_status _this,
    v_gid    instanceGID,
    c_bool   dispose);

OS_API void
v_statusNotifySubscriptionMatched(
    v_status _this,
    v_gid    instanceGID,
    c_bool   dispose);

OS_API void
v_statusNotifyLivelinessChanged (
    v_status _this,
    c_long activeInc,
    c_long inactiveInc,
    v_gid instanceGID);

OS_API void
v_statusNotifySampleRejected (
    v_status _this,
    v_sampleRejectedKind r,
    v_gid instanceGID);

void
v_statusResetCounters(
    v_status s,
    c_ulong mask);

v_status
v_statusCopyOut(
    v_status s);

#undef OS_API

#endif
