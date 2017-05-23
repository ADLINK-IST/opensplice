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
#include "s_shmMonitor.h"
#include "s_threadsMonitor.h"
#include "s_configuration.h"
#include "os_thread.h"
#include "os_cond.h"
#include "os_mutex.h"
#include "os_sharedmem.h"
#include "os_defs.h"
#include "os_heap.h"
#include "os_report.h"
#include "c_iterator.h"
#include "v_kernel.h"
#include "u_domain.h"
#include "u_spliced.h"
#include "u_entity.h"
#include "spliced.h"

#if 0
  #define TRACE_PKILL printf
#else
  #define TRACE_PKILL(...)
#endif

typedef enum {
    SHM_STATE_UNKNOWN, /* Should only be in this state when determining if
                        * shared memory is clean of unclean. */
    SHM_STATE_UNCLEAN, /* The shared memory monitor was unable to clean up
                        * resources left by a terminated process. */
    SHM_STATE_CLEAN    /* Normal operation, shared memory is clean. */
} s_shmState;

C_STRUCT(s_shmMonitor)
{
    spliced spliceDaemon;
    ut_thread thr;
    os_cond cleanCondition;
    os_mutex mutex;
    os_boolean terminate;
    s_shmState shmState;
};

static void*
shmMonitorMain(
    void* arg)
{
    os_sharedHandle shmHandle;
    u_result cleanupResult;
    os_result result;
    os_duration blockingTime = 10*OS_DURATION_MILLISECOND;
    os_shmClient clients, client;
    s_shmMonitor _this = (s_shmMonitor)arg;
    os_procId ownPID;

    ownPID = os_procIdSelf();
    shmHandle = u_domainSharedMemoryHandle(
                    u_participantDomain(
                            u_participant(
                                    splicedGetService(_this->spliceDaemon))));

    os_mutexLock(&_this->mutex);
    while(_this->terminate == OS_FALSE){
        clients = NULL;
        os_mutexUnlock(&_this->mutex);
        ut_threadAsleep(_this->thr, 1);
        result = os_sharedMemoryWaitForClientChanges(shmHandle, blockingTime, &clients);
        os_mutexLock(&_this->mutex);
        if(result == os_resultSuccess){
            client = clients;
            _this->shmState = SHM_STATE_UNKNOWN;
            while(client){
                if(client->state == OS_SHM_PROC_TERMINATED){
                    if(client->procId != ownPID){
                        OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED, 0,
                            "Detected termination of process %d, that failed "
                            "to clean up its resources before terminating. "
                            "Attempting to clean up its resources now..."
                            , client->procId);

                        os_mutexUnlock(&_this->mutex);

                        /*
                         * Allow the u_splicedCleanupProcessInfo() to take as
                         * long as MAX(leasePeriod, serviceTerminatePeriod).
                         * This is set in the threadsMonitor as the threads
                         * interval.
                         * By indicating that it'll sleep for 1 second, it
                         * is allowed to stay dormant for that 1 second plus
                         * the threads interval.
                         */
                        ut_threadAsleep(_this->thr, 1);
                        cleanupResult = u_splicedCleanupProcessInfo(splicedGetService(_this->spliceDaemon),
                                                                    client->procId);
                        os_mutexLock(&_this->mutex);
                        if(cleanupResult != U_RESULT_OK){
                            OS_REPORT(OS_FATAL, OSRPT_CNTXT_SPLICED, 0,
                                "Cleaning up resources of terminated process "
                                "%d failed, because process was modifying "
                                "shared resources when it terminated, "
                                "stopping domain now...",
                                client->procId);
                            _this->shmState = SHM_STATE_UNCLEAN;
                            os_condSignal(&_this->cleanCondition);

                            splicedSignalTerminate(_this->spliceDaemon, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_NOK);
                        } else {
                            OS_REPORT(OS_INFO, OSRPT_CNTXT_SPLICED, 0,
                                "Successfully cleaned up resources of "
                                "terminated process %d.", client->procId);
                        }
                    } else {
                        OS_REPORT(OS_FATAL, OSRPT_CNTXT_SPLICED, 0,
                            "Detected unexpected detach of kernel by my own "
                            "process, stopping domain now...");

                        _this->shmState = SHM_STATE_UNCLEAN;
                        os_condSignal(&_this->cleanCondition);

                        splicedSignalTerminate(_this->spliceDaemon, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_NOK);
                    }
                    ut_threadAwake(_this->thr);
                }
                client = client->next;
            }
            os_shmClientFree(clients);
            if (_this->shmState == SHM_STATE_UNKNOWN) {
                _this->shmState = SHM_STATE_CLEAN;
                os_condSignal(&_this->cleanCondition);
            }

        } else if (result == os_resultUnavailable) {
           /* client list is empty so we need to give up some cpu time
              in order that it can be initialised on non timesliced systems
              e.g. vxworks kernel builds */
            ut_sleep(_this->thr, 100*OS_DURATION_MICROSECOND);
        }
    }
    os_mutexUnlock(&_this->mutex);
    return NULL;
}


s_shmMonitor
s_shmMonitorNew(
    spliced splicedaemon)
{
    s_shmMonitor _this;
    os_result result;
    s_configuration config;

    assert(splicedaemon);

    config = splicedGetConfiguration(splicedaemon);
    assert(config);

    _this = os_malloc(sizeof *_this);

    _this->spliceDaemon = splicedaemon;
    _this->terminate = OS_FALSE;
    _this->thr = NULL;
    _this->shmState = SHM_STATE_CLEAN;

    result = os_mutexInit(&_this->mutex, NULL);
    if(result != os_resultSuccess){
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0, "Failed to init shm monitor mutex");
        goto err_shmMonitor_mtx;
    }
    result = os_condInit(&_this->cleanCondition, &_this->mutex, NULL);
    if(result != os_resultSuccess){
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0, "Failed to init shm monitor cleanCondition");
        goto err_shmMonitor_clean_cnd;
    }
    ut_threadCreate(splicedGetThreads(splicedaemon), &(_this->thr), "shmMonitor", &config->leaseRenewAttribute, shmMonitorMain, _this);
    if (_this->thr == NULL) {
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0, "Failed to start shared memory monitor");
        goto err_shmMonitor_thr;
    }
    return _this;

/* Error handling */
err_shmMonitor_thr:
    os_condDestroy(&_this->cleanCondition);
err_shmMonitor_clean_cnd:
    os_mutexDestroy(&_this->mutex);
err_shmMonitor_mtx:
    os_free(_this);

    return NULL;
}

void
s_shmMonitorFree(
    s_shmMonitor _this)
{
    if (_this != NULL) {
        os_mutexLock(&_this->mutex);
        _this->terminate = OS_TRUE;
        os_mutexUnlock(&_this->mutex);
        if (_this->thr != NULL) {
            (void)ut_threadWaitExit(_this->thr, NULL);
            _this->thr = NULL;
        }
        os_mutexDestroy(&_this->mutex);
        os_condDestroy(&_this->cleanCondition);
        os_free(_this);
    }
}

/**
 * \brief This operation will return whether or not the shared memory is clean.
 *
 * This operation will block as long as the shared memory monitor is determining
 * if terminated processes left resources and if it can remove those leaked
 * resources. When unable the shared memory will be unclean.
 *
 * \param _this The shmMonitor instance.
 *
 * \return This operation will return TRUE is the shared memory is clean.
 *         Otherwise it will return FALSE.
 */
c_bool
s_shmMonitorIsClean(s_shmMonitor _this)
{
    c_bool result;
    os_duration pollDelay = 100*OS_DURATION_MILLISECOND;

    os_mutexLock(&_this->mutex);
    while ((_this->terminate == OS_FALSE) && (_this->shmState == SHM_STATE_UNKNOWN)) {
        (void) os_condTimedWait(&_this->cleanCondition, &_this->mutex, pollDelay);
    }
    result = (_this->shmState == SHM_STATE_CLEAN) ? TRUE : FALSE;
    os_mutexUnlock(&_this->mutex);

    return result;
}
