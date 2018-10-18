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
#include "u_listener.h"
#include "u__user.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "v_listener.h"
#include "v_entity.h"
#include "os_report.h"

static u_result
u__listenerDeinitW(
    void *_this)
{
    return u__observableDeinitW(_this);
}

static void
u__listenerFreeW(
    void *_this)
{
    u__observableFreeW(_this);
}

static u_result
u_listenerInit(
    const u_listener _this,
    v_public listener,
    u_domain domain)
{
    assert(_this);
    return u_observableInit(u_observable(_this), listener, domain);
}

u_listener
u_listenerNew(
    const u_entity p,
    u_bool combine)
{
    u_listener _this = NULL;
    u_result result;
    v_public vObject;
    v_entity entity, next;

    assert(p != NULL);

    result = u_observableWriteClaim(u_observable(p),(v_public *)(&entity), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        while ((next = v_entityOwner(entity)) != NULL) entity = next;
        assert(entity);
        vObject = v_public(v_listenerNew(v_participant(entity),combine));
        if (vObject != NULL) {
            _this = u_objectAlloc(sizeof(*_this), U_LISTENER, u__listenerDeinitW, u__listenerFreeW);
            if (_this != NULL) {
                u_domain domain = u_observableDomain(u_observable(p));
                result = u_listenerInit(_this, vObject, domain);
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR, "u_listenerNew", result,
                              "Listener initialization failed. "
                              "For Participant (0x%"PA_PRIxADDR")", (os_address)p);
                    u_objectFree (u_object (_this));
                    _this = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR, "u_listenerNew", U_RESULT_INTERNAL_ERROR,
                            "Create user proxy failed. "
                            "For Participant (0x%"PA_PRIxADDR")", (os_address)p);
            }
            c_free(vObject);
        } else {
            OS_REPORT(OS_ERROR, "u_listenerNew", U_RESULT_INTERNAL_ERROR,
                        "Create kernel entity failed. "
                        "For Participant (0x%"PA_PRIxADDR")", (os_address)p);
        }
        u_observableRelease(u_observable(p), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_listenerNew", result,
                    "Claim Participant (0x%"PA_PRIxADDR") failed.", (os_address)p);
    }
    return _this;
}

u_result
u_listenerNotify(
    const u_listener _this)
{
    u_result r = U_RESULT_OK;
    v_listener kl;

    assert(_this);

    r = u_observableReadClaim(u_observable(_this), (v_public *)(&kl), C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(kl);
        v_listenerNotify(kl, NULL, NULL);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return r;
}

u_result
u_listenerTrigger(
    const u_listener _this)
{
    u_result result = U_RESULT_OK;
    v_listener vListener;

    assert (_this != NULL);

    result = u_observableTriggerClaim(u_observable(_this), (v_public *)&vListener, C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert (vListener != NULL);
        v_listenerNotify(vListener, NULL, NULL);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return result;
}

u_result
u_listenerWait (
    const u_listener _this,
    u_listenerAction action,
    void *arg,
    os_duration timeout)
{
    u_result r;
    v_listener kl;

    assert(_this);
    assert(action);

    r = u_observableReadClaim(u_observable(_this), (v_public *)(&kl), C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(kl);
        r = u_resultFromKernel(v_listenerWait(kl,action,arg,timeout));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    }
    return r;
}
