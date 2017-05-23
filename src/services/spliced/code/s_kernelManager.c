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
#include "s_kernelManager.h"
#include "s_misc.h"
#include "s_configuration.h"
#include "spliced.h"

#include "vortex_os.h"
#include "u_user.h"

C_STRUCT(s_kernelManager) {
    ut_thread thr;
    ut_thread resendManager;
    ut_thread cAndMCommandManager;
    os_mutex mtx;
    os_cond cv;
    u_spliced spliced;
    os_uchar active;
    os_uchar expected;
    spliced internal_spliced;
};

#define S_KERNELMANAGER   (1 << 0)
#define S_RESENDMANAGER   (1 << 1)
#define S_CANDMCMDMANAGER (1 << 2)

/**************************************************************
 * Private functions
 **************************************************************/
static void *
kernelManager(
    void *arg)
{
    s_kernelManager km = (s_kernelManager)arg;
    os_mutexLock(&km->mtx);
    km->active |= S_KERNELMANAGER;
    os_condBroadcast(&km->cv);
    os_mutexUnlock(&km->mtx);
    /* We can not detect progress here. So, simulate a thread sleep. */
    ut_threadAsleep(km->thr, UT_SLEEP_INDEFINITELY);
    u_splicedKernelManager(km->spliced);
    splicedSignalTerminate(km->internal_spliced, SPLICED_EXIT_CODE_OK, SPLICED_SHM_OK);
    return NULL;
}

/* Resend manager thread for built-in participant */
static void *
resendManager(
    void *arg)
{
    s_kernelManager km = (s_kernelManager)arg;
    os_mutexLock(&km->mtx);
    km->active |= S_RESENDMANAGER;
    os_condBroadcast(&km->cv);
    os_mutexUnlock(&km->mtx);
    /* We can not detect progress here. So, simulate a thread sleep. */
    ut_threadAsleep(km->resendManager, UT_SLEEP_INDEFINITELY);
    u_splicedBuiltinResendManager(km->spliced);
    return NULL;

}

static void *
cAndMCommandManager(
    void *arg)
{
    s_kernelManager km = (s_kernelManager)arg;
    os_mutexLock(&km->mtx);
    km->active |= S_CANDMCMDMANAGER;
    os_condBroadcast(&km->cv);
    os_mutexUnlock(&km->mtx);
    /* We can not detect progress here. So, simulate a thread sleep. */
    ut_threadAsleep(km->cAndMCommandManager, UT_SLEEP_INDEFINITELY);
    u_splicedBuiltinCAndMCommandDispatcher(km->spliced);
    return NULL;
}


/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/
s_kernelManager
s_kernelManagerNew(
    spliced daemon)
{
    s_kernelManager km;
    s_configuration config;
    os_result osr;

    assert(daemon);

    km = os_malloc(sizeof *km);
    km->internal_spliced = daemon;
    km->spliced = splicedGetService(daemon);
    assert(km->spliced);
    km->expected = km->active = 0;

    if((osr = os_mutexInit(&km->mtx, NULL)) != os_resultSuccess){
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Mutex initialization failed; os_mutexInit returned %s.", os_resultImage(osr));
        goto err_mutexInit;
    }

    if((osr = os_condInit(&km->cv, &km->mtx, NULL)) != os_resultSuccess){
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Condition variable initialization failed; os_condInit returned %s.", os_resultImage(osr));
        goto err_condInit;
    }

    config = splicedGetConfiguration(daemon);
    assert(config);

    ut_threadCreate(splicedGetThreads(daemon), &(km->thr), S_THREAD_KERNELMANAGER,
                &config->kernelManagerAttribute, kernelManager, km);
    if (km->thr == NULL) {
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Failed to start " S_THREAD_KERNELMANAGER " thread.");
        goto err_kmThreadCreate;
    }
    km->expected |= S_KERNELMANAGER;

    ut_threadCreate(splicedGetThreads(daemon), &(km->resendManager), S_THREAD_RESENDMANAGER,
                &config->resendManagerAttribute, resendManager, km);
    if (km->resendManager == NULL) {
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Failed to start " S_THREAD_RESENDMANAGER " thread.");
        goto err_rmThreadCreate;
    }
    km->expected |= S_RESENDMANAGER;

    if (config->enableCandMCommandThread) {
        ut_threadCreate(splicedGetThreads(daemon), &(km->cAndMCommandManager), S_THREAD_C_AND_M_COMMANDMANAGER,
                    &config->cAndMCommandAttribute, cAndMCommandManager, km);
        if (km->cAndMCommandManager == NULL) {
            OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                    "Failed to start " S_THREAD_C_AND_M_COMMANDMANAGER " thread.");
            goto err_cmThreadCreate;
        }
        km->expected |= S_CANDMCMDMANAGER;
    }

    return km;

/* Error handling */
err_cmThreadCreate:
    if(u_splicedPrepareTermination(km->spliced) == U_RESULT_OK) {
        (void) ut_threadWaitExit(km->resendManager, NULL);
    }
err_rmThreadCreate:
    if(u_splicedPrepareTermination(km->spliced) == U_RESULT_OK) {
        (void) ut_threadWaitExit(km->thr, NULL);
    }
err_kmThreadCreate:
    os_condDestroy(&km->cv);
err_condInit:
    os_mutexDestroy(&km->mtx);
err_mutexInit:
    os_free(km);
    return NULL;
}

void
s_kernelManagerFree(
    s_kernelManager km)
{
    assert(km);

    if(km->expected & S_KERNELMANAGER){
        ut_threadWaitExit(km->thr, NULL);
    }
    if(km->expected & S_RESENDMANAGER){
        ut_threadWaitExit(km->resendManager, NULL);
    }
    if(km->expected & S_CANDMCMDMANAGER){
        u_splicedCAndMCommandDispatcherQuit(km->spliced);
        ut_threadWaitExit(km->cAndMCommandManager, NULL);
    }
    os_condDestroy(&km->cv);
    os_mutexDestroy(&km->mtx);
    os_free(km);
}

void
s_kernelManagerWaitForActive(
    s_kernelManager km)
{
    os_duration delay = 1*OS_DURATION_SECOND;
    os_timeM start;
    os_timeM cur;
    ut_thread self;

    assert(km);

    self = ut_threadLookupSelf(splicedGetThreads(km->internal_spliced));

    os_mutexLock(&km->mtx);
    cur = os_timeMGet();
    start = cur;
    while ((km->active != km->expected) &&
           (os_timeMDiff(cur, start) < 20*OS_DURATION_SECOND)) {
        (void)ut_condTimedWait(self, &km->cv, &km->mtx, delay);
        cur = os_timeMGet();
    }
    os_mutexUnlock(&km->mtx);
}

/**************************************************************
 * Public functions
 **************************************************************/
