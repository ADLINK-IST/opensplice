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

#ifndef V__LEASETIME_H
#define V__LEASETIME_H

#include "kernelModuleI.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define v_leaseTimeKind(t)\
    (assert((t)._d == V_LEASE_KIND_MONOTONIC || (t)._d == V_LEASE_KIND_ELAPSED), ((t)._d))

#define v_leaseTimeToTimeE(t)\
    (assert((t)._d == V_LEASE_KIND_ELAPSED), (t)._u.te)

#define v_leaseTimeToTimeM(t)\
    (assert((t)._d == V_LEASE_KIND_MONOTONIC), (t)._u.tm)

#define V_LEASE_TIME_PRINT(t) \
    OS_TIME_PRINT((v_leaseTimeKind(t) == V_LEASE_KIND_MONOTONIC) ? OS_TIMEM_GET_VALUE((t)._u.tm) : OS_TIMEE_GET_VALUE((t)._u.te))

void
v_leaseTimeInit(
    v_leaseTime *t,
    v_leaseKind kind,
    os_duration leaseDuration);

void
v_leaseTimeUpdate(
    v_leaseTime *t,
    os_duration leaseDuration);

v_leaseTime
v_leaseTimeGet(
    v_leaseKind kind);

v_leaseTime
v_leaseTimeInfinite(
    v_leaseKind kind);

os_compare
v_leaseTimeCompare(
    v_leaseTime t1,
    v_leaseTime t2);

os_duration
v_leaseTimeDiff(
    v_leaseTime t1,
    v_leaseTime t2);

v_leaseTime
v_leaseTimeAdd(
    v_leaseTime t,
    os_duration duration);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V__LEASETIME_H */
