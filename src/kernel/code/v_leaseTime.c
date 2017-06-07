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


#include "v_event.h"
#include "v__leaseTime.h"

#include "vortex_os.h"


void
v_leaseTimeUpdate(
    v_leaseTime *t,
    os_duration leaseDuration)
{
    assert(t);
    assert(t->_d == V_LEASE_KIND_MONOTONIC || t->_d == V_LEASE_KIND_ELAPSED);

    if (t->_d == V_LEASE_KIND_MONOTONIC) {
        t->_u.tm = os_timeMAdd(os_timeMGet(), leaseDuration);
    } else {
        t->_u.te = os_timeEAdd(os_timeEGet(), leaseDuration);
    }
}

void
v_leaseTimeInit(
    v_leaseTime *t,
    v_leaseKind kind,
    os_duration leaseDuration)
{
    t->_d = kind;
    v_leaseTimeUpdate(t, leaseDuration);
}

v_leaseTime
v_leaseTimeGet(
    v_leaseKind kind)
{
    v_leaseTime t;

    assert(kind == V_LEASE_KIND_MONOTONIC || kind == V_LEASE_KIND_ELAPSED);

    t._d = kind;
    if (t._d == V_LEASE_KIND_MONOTONIC) {
        t._u.tm = os_timeMGet();
    } else {
        t._u.te = os_timeEGet();
    }

    return t;
}

v_leaseTime
v_leaseTimeInfinite(
    v_leaseKind kind)
{
    v_leaseTime t;

    assert(kind == V_LEASE_KIND_MONOTONIC || kind == V_LEASE_KIND_ELAPSED);

    t._d = kind;
    if (kind == V_LEASE_KIND_MONOTONIC) {
        t._u.tm = OS_TIMEM_INFINITE;
    } else {
        t._u.te = OS_TIMEE_INFINITE;
    }

    return t;
}

os_compare
v_leaseTimeCompare(
    v_leaseTime t1,
    v_leaseTime t2)
{
    os_compare eq;

    assert(t1._d == t2._d);
    assert(t1._d == V_LEASE_KIND_MONOTONIC || t1._d == V_LEASE_KIND_ELAPSED);

    if (t1._d == V_LEASE_KIND_MONOTONIC) {
        eq = os_timeMCompare(t1._u.tm, t2._u.tm );
    } else {
        eq = os_timeECompare(t1._u.te, t2._u.te);
    }

    return eq;
}

os_duration
v_leaseTimeDiff(
    v_leaseTime t1,
    v_leaseTime t2)
{
    os_duration delta;

    assert(t1._d == t2._d);
    assert(t1._d == V_LEASE_KIND_MONOTONIC || t1._d == V_LEASE_KIND_ELAPSED);

    if (t1._d == V_LEASE_KIND_MONOTONIC) {
        delta = os_timeMDiff(t1._u.tm, t2._u.tm);
    } else {
        delta = os_timeEDiff(t1._u.te, t2._u.te);
    }

    return delta;
}

v_leaseTime
v_leaseTimeAdd(
    v_leaseTime t,
    os_duration duration)
{
    assert(t._d == V_LEASE_KIND_MONOTONIC || t._d == V_LEASE_KIND_ELAPSED);

    if (t._d == V_LEASE_KIND_MONOTONIC) {
        os_timeMAdd(t._u.tm, duration);
    } else {
        os_timeEAdd(t._u.te, duration);
    }

    return t;
}
