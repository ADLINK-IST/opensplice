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
#include "u__serviceManager.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__participant.h"
#include "u__domain.h"
#include "u_user.h"

#include "v_kernel.h"
#include "v_entity.h"
#include "v_service.h"

#include "os_stdlib.h"
#include "os_heap.h"
#include "os_report.h"

static u_result
u__serviceManagerDeinitW(
    void *_this)
{
    return u__entityDeinitW(_this);
}

static void
u__serviceManagerFreeW(
    void *_this)
{
    u__entityFreeW(_this);
}

static u_result
u_serviceManagerInit(
    const u_serviceManager _this,
    const v_serviceManager sm,
    const u_participant participant)
{
    u_result result;

    assert(_this != NULL);

    result = u_entityInit(u_entity(_this), v_entity(sm), u_observableDomain(u_observable(participant)));

    return result;
}

u_serviceManager
u_serviceManagerNew(
    const u_participant participant)
{
    u_result result;
    u_serviceManager m;
    u_domain domain;
    v_kernel kk;
    v_serviceManager sm;

    assert(participant != NULL);

    m = NULL;
    domain = u_participantDomain(participant);
    assert(domain != NULL);
    result = u_observableWriteClaim(u_observable(domain),(v_public *)(&kk), C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(kk);
        sm = v_getServiceManager(kk);
        if (sm != NULL) {
            m = u_objectAlloc(sizeof(*m), U_SERVICEMANAGER, u__serviceManagerDeinitW, u__serviceManagerFreeW);
            if (m != NULL) {
                result = u_serviceManagerInit(m, sm, participant);
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR,"u_serviceManagerNew", result,
                                        "Failed to initiate Service Manager proxy.");
                    u_objectFree (u_object (m));
                    m = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR,"u_serviceManagerNew", U_RESULT_OUT_OF_MEMORY,
                          "Allocation Service Manager proxy failed.");
            }
        } else {
            OS_REPORT(OS_ERROR,"u_serviceManagerNew", U_RESULT_INTERNAL_ERROR,
                      "Retrieval Service Manager failed.");
        }
        u_observableRelease(u_observable(domain), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING,"u_serviceManagerNew", result,
                  "Claim Domain failed.");
    }

    return m;
}

v_serviceStateKind
u_serviceManagerGetServiceStateKind(
    const u_serviceManager _this,
    const os_char *serviceName)
{
    u_result result;
    v_serviceManager kServiceManager;
    v_serviceStateKind kind;

    assert(_this != NULL);
    assert(serviceName != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&kServiceManager), C_MM_RESERVATION_NO_CHECK);
    if (result == U_RESULT_OK) {
        kind = v_serviceManagerGetServiceStateKind(kServiceManager, serviceName);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_NO_CHECK);
    } else {
        kind = STATE_NONE;
        OS_REPORT(OS_WARNING, "u_serviceManagerGetServiceStateKind", result,
                  "Could not claim serviceManager.");
    }

    return kind;
}

c_iter
u_serviceManagerGetServices(
    const u_serviceManager _this,
    const v_serviceStateKind kind)
{
    u_result result = U_RESULT_OK;
    v_serviceManager kServiceManager;
    c_iter names;
    c_iter vNames;
    c_string str;
    c_char *n;

    assert(_this != NULL);

    names = c_iterNew(NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&kServiceManager), C_MM_RESERVATION_NO_CHECK);
    if (result == U_RESULT_OK) {
        vNames = v_serviceManagerGetServices(kServiceManager, kind);
        str = (c_string)c_iterTakeFirst(vNames);
        while (str != NULL) {
            n = os_strdup(str);
            names = c_iterInsert(names, (void *)n);
            str = (c_string)c_iterTakeFirst(vNames);
        }
        c_iterFree(vNames);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_NO_CHECK);
    } else {
        OS_REPORT(OS_WARNING, "u_serviceManagerGetServices", result,
                  "Could not claim serviceManager.");
    }

    return names;
}

c_bool
u_serviceManagerRemoveService(
    const u_serviceManager _this,
    const c_char *serviceName)
{
    u_result result = U_RESULT_OK;
    v_serviceManager kServiceManager;
    c_bool retVal = FALSE;

    assert(_this);
    result = u_observableReadClaim(u_observable(_this), (v_public*)(&kServiceManager), C_MM_RESERVATION_NO_CHECK);
    if (result == U_RESULT_OK) {
        retVal = v_serviceManagerRemoveService(kServiceManager, serviceName);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_NO_CHECK);
    } else {
        OS_REPORT(OS_ERROR, "u_serviceManagerRemoveService", result,
                  "Could not claim serviceManager.");
    }
    return retVal;
}
