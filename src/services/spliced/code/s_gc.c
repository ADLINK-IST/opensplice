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
#include "s_gc.h"
#include "spliced.h"
#include "s_configuration.h"
#include "s_misc.h"

#include "vortex_os.h"
#include "u_user.h"

C_STRUCT(s_garbageCollector) {
    ut_thread thr;
    os_mutex mtx;
    os_cond cv;
    int active;
    spliced spliced;
};

static void *
garbageCollector(
    void *arg)
{
    s_garbageCollector gc = (s_garbageCollector)arg;
    os_mutexLock(&gc->mtx);
    gc->active++;
    os_condBroadcast(&gc->cv);
    os_mutexUnlock(&gc->mtx);
    /* We can not detect progress here. So, simulate a thread sleep. */
    ut_threadAsleep(gc->thr, UT_SLEEP_INDEFINITELY);
    u_splicedGarbageCollector(splicedGetService(gc->spliced));
    return NULL;
}

s_garbageCollector
s_garbageCollectorNew(
    spliced daemon)
{
    s_garbageCollector gc;
    s_configuration config;
    os_result osr;

    gc = os_malloc(sizeof *gc);

    gc->spliced = daemon;
    gc->active = 0;
    osr = os_mutexInit(&gc->mtx, NULL);
    if (osr != os_resultSuccess) {
        goto err_mutexInit;
    }
    osr = os_condInit(&gc->cv, &gc->mtx, NULL);
    if (osr != os_resultSuccess) {
        goto err_condInit;
    }
    config = splicedGetConfiguration(daemon);
    ut_threadCreate(splicedGetThreads(daemon), &(gc->thr), S_THREAD_GARBAGE_COLLECTOR, &config->garbageCollectorAttribute, garbageCollector, gc);
    if (gc->thr == NULL) {
        goto err_threadCreate;
    }
    return gc;

err_threadCreate:
    os_condDestroy(&gc->cv);
err_condInit:
    os_mutexDestroy(&gc->mtx);
err_mutexInit:
    os_free(gc);
    return NULL;
}

os_boolean
s_garbageCollectorFree(
    s_garbageCollector gc)
{
    os_boolean result = OS_TRUE;
    s_configuration config;
    os_result osr;

    if (gc) { /* gc might be NULL, when spliced has detected other spliced */
        config = splicedGetConfiguration(gc->spliced);
        osr = ut_threadTimedWaitExit(gc->thr, config->serviceTerminatePeriod, NULL);
        if (osr == os_resultSuccess) {
            os_condDestroy(&gc->cv);
            os_mutexDestroy(&gc->mtx);
            os_free(gc);
        } else {
            OS_REPORT(OS_ERROR, OS_FUNCTION, osr,
                "Failed to join thread \"%s\":0x%" PA_PRIxADDR " (%s)",
                ut_threadGetName(gc->thr),
                (os_address)os_threadIdToInteger(ut_threadGetId(gc->thr)),
                os_resultImage(osr));
            result = OS_FALSE;
        }
    }
    return result;
}

void
s_garbageCollectorWaitForActive(
    s_garbageCollector gc)
{
    ut_thread self;

    assert(gc);

    self = ut_threadLookupSelf(splicedGetThreads(gc->spliced));

    os_mutexLock(&gc->mtx);
    while (gc->active == 0) {
        ut_condWait(self, &gc->cv, &gc->mtx);
    }
    os_mutexUnlock(&gc->mtx);
}
