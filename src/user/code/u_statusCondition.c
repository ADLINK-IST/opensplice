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
#include "u_statusCondition.h"
#include "u__types.h"
#include "u__object.h"
#include "u__entity.h"
#include "u__participant.h"
#include "u__domain.h"
#include "u__observable.h"
#include "v_observer.h"
#include "v_statusCondition.h"
#include "v_entity.h"
#include "v_event.h"
#include "os_report.h"

#include "u__user.h"

#define u_statusConditionClaim(_this, reader) \
        u_observableReadClaim(u_observable(_this), (v_public *)(reader))

#define u_statusConditionRelease(_this) \
        u_observableRelease(u_observable(_this))

static u_result
u__statusConditionDeinitW(
    void *_this)
{
    return u__observableDeinitW(_this);
}

static void
u__statusConditionFreeW(
    void *_this)
{
    u__observableFreeW(_this);
}

u_statusCondition
u_statusConditionNew(
    const u_entity entity)
{
    u_statusCondition _this = NULL;
    u_result result;
    v_entity kEntity;
    v_public kCond;

    assert(entity);

    result = u_observableWriteClaim(u_observable(entity), (v_public *)&kEntity, C_MM_RESERVATION_LOW);
    if (result == U_RESULT_OK) {
        kCond = v_public(v_statusConditionNew(kEntity));
        if (kCond != NULL) {
            _this = u_objectAlloc(sizeof(*_this), U_STATUSCONDITION, u__statusConditionDeinitW, u__statusConditionFreeW);
            if (_this) {
                u_domain domain = u_observableDomain(u_observable(entity));
                v_observerSetEventMask(v_observer(kCond), V_EVENTMASK_ALL);
                result = u_observableInit(u_observable(_this), kCond, domain);
                if (result != U_RESULT_OK) {
                    u_objectFree(_this);
                    _this = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR, "u_statusConditionNew", U_RESULT_INTERNAL_ERROR,
                            "Create user entity failed. "
                            "For Entity (0x%"PA_PRIxADDR")", (os_address)entity);
            }
            c_free(kCond);
        } else {
            OS_REPORT(OS_ERROR, "u_statusConditionNew", U_RESULT_OUT_OF_MEMORY,
                        "Create kernel entity failed. "
                        "For Entity (0x%"PA_PRIxADDR")", (os_address)entity);
        }
        u_observableRelease(u_observable(entity), C_MM_RESERVATION_LOW);
    } else {
        OS_REPORT(OS_ERROR,"u_statusConditionNew", U_RESULT_INTERNAL_ERROR,
                    "Invalid Entity (0x%"PA_PRIxADDR"x) specified.", (os_address)entity);
    }
    return _this;
}

u_result
u_statusCondition_set_mask(
    const u_statusCondition _this,
    u_eventMask eventMask)
{
    u_result result;
    v_public kCond;

    assert(_this);

    result = u_observableReadClaim(u_observable(_this), &kCond, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        v_statusConditionSetMask(v_statusCondition(kCond), eventMask);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_statusCondition_get_triggerValue (
    const u_statusCondition _this,
    u_eventMask *triggerValue)
{
    u_result result;
    v_public kCond;

    assert(_this);
    assert(triggerValue);

    result = u_observableReadClaim(u_observable(_this), &kCond, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        *triggerValue = v_statusConditionGetTriggerValue(v_statusCondition(kCond));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;
}
