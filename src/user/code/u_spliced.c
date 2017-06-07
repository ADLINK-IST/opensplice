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
#include "u__user.h"

#include "u_spliced.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__service.h"
#include "u__domain.h"
#include "u__cfValue.h"

#include "v_entity.h"
#include "v_service.h"
#include "v_spliced.h"
#include "v_serviceManager.h"
#include "v_configuration.h"
#include "v_participant.h"

#include "v_leaseManager.h"
#include "os_report.h"

static u_bool splicedStartedInThisProcess = FALSE;

/**************************************************************
 * Private functions
 **************************************************************/

static u_result
u_splicedInit(
    u_spliced spliced,
    v_spliced ks,
    const u_domain domain)
{
    assert(spliced != NULL);

    return u_serviceInit(u_service(spliced), v_service(ks), domain);
}

static u_result
u__splicedDeinitW(
    void *_this)
{
    return u__serviceDeinitW(_this);
}

static void
u__splicedFreeW(
    void *_this)
{
    u__serviceFreeW(_this);
}

static v_spliced
getKernelSplicedaemon(
    const u_domain k)
{
    u_result r;
    v_kernel kk;
    c_iter participants;
    v_spliced spliced;

    assert(k != NULL);

    r = u_observableReadClaim(u_observable(k),(v_public *)(&kk), C_MM_RESERVATION_NO_CHECK);
    if (r == U_RESULT_OK) {
        assert(kk);
        participants = v_resolveParticipants(kk, V_SPLICED_NAME);
        u_observableRelease(u_observable(k),C_MM_RESERVATION_NO_CHECK);
        assert(c_iterLength(participants) == 1);
        spliced = v_spliced(c_iterTakeFirst(participants));
        c_iterFree(participants);
    } else {
        OS_REPORT(OS_WARNING,"u_splicedNew::getKernelSplicedaemon", r,
                  "Claim Kernel failed.");
        spliced = NULL;
    }
    return spliced;
}

/**************************************************************
 * constructor/destructor
 **************************************************************/

#define SPLICED_NAME "spliced"

u_result
u_splicedNew(
    u_spliced *spliced,
    const os_char *uri)
{
    u_result result;
    u_domain domain;
    v_spliced kSpliced;

    assert(spliced);

    kSpliced = NULL;
    *spliced = NULL;

    result = u_domainNew(&domain, uri);
    if (result == U_RESULT_OK) {
        /* create new proxy to v_spliced object */
        kSpliced = getKernelSplicedaemon(domain);
        if (kSpliced != NULL) {
            *spliced = u_objectAlloc(sizeof(**spliced), U_SPLICED, u__splicedDeinitW, u__splicedFreeW);
            if (*spliced != NULL) {
                result = u_splicedInit(*spliced, kSpliced, domain);
                if (result != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR,"u_splicedNew", result,
                              "Failed to initialize spliced.");
                    u_objectFree (u_object (u_service(*spliced)));
                    *spliced = NULL;
                }
            }
        } else {
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else if (result != U_RESULT_ILL_PARAM)  {
        result = U_RESULT_INTERNAL_ERROR;
    }
    return result;
}

#undef SPLICED_NAME

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
u_result
u_splicedKernelManager(
    const u_spliced spliced)
{
    u_result r;
    v_spliced s;

    assert(spliced);

    r = u_observableReadClaim(u_observable(spliced), (v_public *)(&s), C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(s);
        v_splicedKernelManager(s);
        u_observableRelease(u_observable(spliced), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_splicedKernelManager", r,
                  "Could not claim spliced.");
    }
    return r;
}

u_result
u_splicedBuiltinResendManager(
    const u_spliced spliced)
{
    u_result r;
    v_spliced s;

    assert(spliced);

    r = u_observableReadClaim(u_observable(spliced), (v_public *)(&s), C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(s);
        v_splicedBuiltinResendManager(s);
        u_observableRelease(u_observable(spliced), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_splicedBuiltinResendManager", r,
                  "Could not claim spliced.");
    }
    return r;
}

u_result
u_splicedBuiltinCAndMCommandDispatcher(
    const u_spliced spliced)
{
    u_result r;
    v_spliced s;

    assert(spliced);

    r = u_observableReadClaim(u_observable(spliced), (v_public *)(&s), C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(s);
        v_splicedBuiltinCAndMCommandDispatcher(s);
        u_observableRelease(u_observable(spliced), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_splicedBuiltinCAndMCommandDispatcher", r,
                  "Could not claim spliced.");
    }
    return r;
}

u_result
u_splicedCAndMCommandDispatcherQuit(
   const u_spliced spliced)
{
   u_result r;
   v_spliced s;

   assert(spliced);

   r = u_observableReadClaim(u_observable(spliced), (v_public*)(&s), C_MM_RESERVATION_NO_CHECK);
   if (r == U_RESULT_OK) {
      assert(s);
      v_splicedCAndMCommandDispatcherQuit(s);
      u_observableRelease(u_observable(spliced), C_MM_RESERVATION_NO_CHECK);
   } else {
      OS_REPORT(OS_WARNING,
                "u_splicedBuiltinCAndMCommandDispatcherQuit", r,
                "Could not claim spliced.");
   }
   return r;
}

u_result
u_splicedGarbageCollector(
    const u_spliced spliced)
{
    u_result r;
    v_spliced s;

    assert(spliced);

    r = u_observableReadClaim(u_observable(spliced), (v_public *)(&s),C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(s);
        v_splicedGarbageCollector(s);
        u_observableRelease(u_observable(spliced),C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_splicedGarbageCollector", r,
                  "Could not claim spliced.");
    }
    return r;
}

u_result
u_splicedPrepareTermination(
    const u_spliced spliced)
{
    u_result r;
    v_spliced s;

    assert(spliced);

    r = u_observableReadClaim(u_observable(spliced), (v_public *)(&s), C_MM_RESERVATION_NO_CHECK);
    if (r == U_RESULT_OK) {
        assert(s);
        v_splicedPrepareTermination(s);

        /* Request shutdown of & wakeup cAndMCommandManager thread */
        v_splicedCAndMCommandDispatcherQuit(s);
        u_observableRelease(u_observable(spliced), C_MM_RESERVATION_NO_CHECK);
    } else {
        OS_REPORT(OS_WARNING, "u_splicedPrepareTermination", r,
                  "Could not claim spliced.");
    }
    return r;
}

v_leaseManager
u_splicedGetHeartbeatManager(
    u_spliced spliced,
    c_bool create)
{
    v_leaseManager lm = NULL;
    v_spliced s;
    u_result result;

    result = u_observableReadClaim(u_observable(spliced), (v_public *)(&s), C_MM_RESERVATION_NO_CHECK);
    if(result == U_RESULT_OK)
    {
        assert(s);
        lm = v_splicedGetHeartbeatManager(s, create);
        u_observableRelease(u_observable(spliced), C_MM_RESERVATION_NO_CHECK);
    } else {
         OS_REPORT(OS_WARNING, "u_splicedGetHeartbeatManager", result,
                   "Could not claim spliced.");
    }

    return lm;
}

u_result
u_splicedStartHeartbeat(
    const u_spliced spliced,
    os_duration period,
    os_duration renewal)
{
    u_result r;
    v_spliced s;
    u_bool started;

    assert(spliced);

    r = u_observableReadClaim(u_observable(spliced), (v_public *)(&s),C_MM_RESERVATION_ZERO);
    if (r == U_RESULT_OK) {
        assert(s);
        started = v_splicedStartHeartbeat(s, period, renewal);
        if (started == FALSE) {
            r = U_RESULT_INTERNAL_ERROR;
        }
        u_observableRelease(u_observable(spliced), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_splicedStartHeartbeat", r,
                  "Could not claim spliced.");
    }
    return r;
}

u_result
u_splicedStopHeartbeat(
    const u_spliced spliced)
{
    u_result r;
    v_spliced s;
    u_bool stopped;

    assert(spliced);

    r = u_observableReadClaim(u_observable(spliced), (v_public *)(&s), C_MM_RESERVATION_NO_CHECK);
    if (r == U_RESULT_OK) {
        assert(s);
        stopped = v_splicedStopHeartbeat(s);
        if (stopped == FALSE) {
            r = U_RESULT_INTERNAL_ERROR;
        }
        u_observableRelease(u_observable(spliced), C_MM_RESERVATION_NO_CHECK);
    } else {
        OS_REPORT(OS_WARNING, "u_splicedStopHeartbeat", r,
                  "Could not claim spliced, result was %s.", u_resultImage(r));
    }
    return r;
}

void
u_splicedSetInProcess(c_bool flag)
{
    splicedStartedInThisProcess = flag;
}

u_bool
u_splicedInProcess(void)
{
    return splicedStartedInThisProcess;
}

u_result
u_splicedCleanupProcessInfo(
    const u_spliced spliced,
    os_procId procId)
{
    v_entity entity;
    u_result ures;
    u_handle handle;

    /* Cleanup process garbage; spliced always has access to SHM because it
     * controls the lifecycle of SHM. In case cleanup of resources of another
     * process causes a deadlock, spliced should still be able to stop its own
     * threads, so cleanup by proxy shouldn't be done using u_observableAction(...),
     * because that modifies the protectCount for this process. */
    handle = u_observableHandle(u_observable(spliced));
    if((ures = u_handleClaim(handle, &entity)) == U_RESULT_OK){
        ures = (u_result)v_kernelDetach(v_objectKernel(entity), procId);
        (void) u_handleRelease(u_observableHandle(u_observable(spliced)));
    }
    return ures;
}


u_result
u_splicedDurabilityClientSetup(
    u_spliced spliced,
    c_iter durablePolicies,
    const char* partitionRequest,
    const char* partitionDataGlobal,
    const char* partitionDataPrivate)
{
    v_spliced s;
    u_result result;

    result = u_observableReadClaim(u_observable(spliced), (v_public *)(&s), C_MM_RESERVATION_LOW);
    if(result == U_RESULT_OK) {
        assert(s);
        result = v_splicedDurabilityClientSetup(s,
                                                durablePolicies,
                                                partitionRequest,
                                                partitionDataGlobal,
                                                partitionDataPrivate);
        u_observableRelease(u_observable(spliced), C_MM_RESERVATION_LOW);
    } else {
         OS_REPORT(OS_WARNING, "u_splicedDurabilityClientSetup", result,
                   "Could not claim spliced.");
    }

    return result;
}


void *
u_splicedDurabilityClientMain(
    void *spliced)
{
    u_spliced _this = u_spliced(spliced);
    u_result result;

    result = u_observableAction(u_observable(_this), v_splicedDurabilityClientMain, NULL);
    if (result != U_RESULT_OK) {
        OS_REPORT(OS_WARNING, "u_splicedDurabilityClientMain", 0,
                  "Failed to call main thread.");
    }

    return NULL;
}

u_result
u_splicedDurabilityClientTerminate(
    u_spliced spliced)
{
    v_spliced s;
    u_result result;

    result = u_observableReadClaim(u_observable(spliced), (v_public *)(&s), C_MM_RESERVATION_ZERO);
    if(result == U_RESULT_OK) {
        assert(s);
        v_splicedDurabilityClientTerminate(s);
        u_observableRelease(u_observable(spliced), C_MM_RESERVATION_ZERO);
    } else {
         OS_REPORT(OS_WARNING, "u_splicedDurabilityClientTerminate", result,
                   "Could not claim spliced.");
    }

    return result;
}
