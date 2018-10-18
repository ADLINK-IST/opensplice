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
#include "s_threadsMonitor.h"
#include "s_configuration.h"
#include "report.h"
#include "spliced.h"
#include "ut_thread.h"

//#define S_DEBUG_THREADS
#ifdef S_DEBUG_THREADS
  #define TRACE_THREADS printf
  static os_duration s_periodicThreadReport = OS_DURATION_INIT(2, 0);
#else
  #define TRACE_THREADS(...)
#endif

const char* s_main_tread_name = "ospl_spliced";

#define S_REPORT_BUFFER_SIZE (512)
typedef struct  {
    char bfr[S_REPORT_BUFFER_SIZE];
    os_uint32 pos;
} s_threadReportArg;

typedef struct {
    os_boolean stop;
    os_mutex mtx;
    os_cond cnd;
} s_terminate;

C_STRUCT(s_threadsMonitor)
{
    spliced spliced;
    ut_thread main;
    ut_thread watchdog;
    ut_threads threads;
    os_duration interval;
    s_terminate terminate;
};

/* number of seconds before going into error state. */
static const os_duration POLL_LIMIT = OS_DURATION_INIT(1, 0);
static const os_duration POLL_INITIAL = OS_DURATION_INIT(1*60, 0);

static void
s_threadsReport(
        ut_thread thread,
        os_boolean alive,
        os_boolean changed,
        const char *info,
        const char *caller,
        void *reportdata,
        void *userdata);

static void*
s_threadsWatchdogMain(
    void* arg);


void
s_threadsMonitorSetInterval(
    spliced splicedaemon)
{
    s_configuration config;
    ut_threads threads;
    os_duration interval;

    assert(splicedaemon);

    config = splicedGetConfiguration(splicedaemon);
    threads = splicedGetThreads(splicedaemon);
    assert(config);
    /* Set allowed thread progress delay interval.
     * This will be the thread checking polling interval as well.
     */
    interval = (os_durationCompare(config->leasePeriod, config->serviceTerminatePeriod) == OS_MORE) ?
                                config->leasePeriod :
                                config->serviceTerminatePeriod;
    interval = (os_durationCompare(interval, POLL_LIMIT) == OS_MORE) ?
                                interval :
                                POLL_LIMIT;
    ut_threadsSetInterval(threads, interval);
}

s_threadsMonitor
s_threadsMonitorNew(
    spliced splicedaemon)
{
    s_threadsMonitor _this;
    os_result result;
    s_configuration config;

    assert(splicedaemon);

    config = splicedGetConfiguration(splicedaemon);
    assert(config);

    _this = os_malloc(OS_SIZEOF(s_threadsMonitor));
    memset(_this, 0, OS_SIZEOF(s_threadsMonitor));

    _this->spliced = splicedaemon;
    _this->threads = splicedGetThreads(splicedaemon);
    _this->main = ut_threadLookupSelf(_this->threads);

    _this->interval = POLL_INITIAL;
    ut_threadsSetInterval(_this->threads, _this->interval);

    result = os_mutexInit(&_this->terminate.mtx, NULL);
    if(result != os_resultSuccess){
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0, "Failed to init threads monitor mutex");
        goto err_threadsMonitor_mtx;
    }
    result = os_condInit(&_this->terminate.cnd, &_this->terminate.mtx, NULL);
    if(result != os_resultSuccess){
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0, "Failed to init threads monitor condition");
        goto err_threadsMonitor_cnd;
    }

    /* Use the shared memory monitor thread attribute.
     * We will mainly be monitoring that thread anyway.
     */
    ut_threadCreate(splicedGetThreads(splicedaemon), &(_this->watchdog), "threadsWatchdog", &config->shmMonitorAttribute, s_threadsWatchdogMain, _this);
    if (_this->watchdog == NULL) {
        OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0, "Failed to start threads watchdog");
        goto err_threadsMonitor_thr;
    }

    return _this;

/* Error handling */
err_threadsMonitor_thr:
    os_condDestroy(&_this->terminate.cnd);
err_threadsMonitor_cnd:
    os_mutexDestroy(&_this->terminate.mtx);
err_threadsMonitor_mtx:
    os_free(_this);

    return NULL;
}

os_boolean
s_threadsMonitorFree(
    s_threadsMonitor _this)
{
    os_boolean result = OS_TRUE;
    s_configuration config;
    os_result osr;

    if (_this != NULL) {
        config = splicedGetConfiguration(_this->spliced);
        /* Stop watchdog thread. */
        os_mutexLock(&_this->terminate.mtx);
        _this->terminate.stop = OS_TRUE;
        os_condSignal(&_this->terminate.cnd);
        os_mutexUnlock(&_this->terminate.mtx);

        /* Wait for the watchdog thread. */
        osr = ut_threadTimedWaitExit(_this->watchdog, config->serviceTerminatePeriod, NULL);
        if (osr != os_resultSuccess) {
            OS_REPORT(OS_ERROR, OS_FUNCTION, osr,
                "Failed to join thread \"%s\":0x%" PA_PRIxADDR " (%s)",
                ut_threadGetName(_this->watchdog),
                (os_address)os_threadIdToInteger(ut_threadGetId(_this->watchdog)),
                os_resultImage(osr));
            result = OS_FALSE;
        } else {
            _this->watchdog = NULL;
            /* Delete variables. */
            os_mutexDestroy(&_this->terminate.mtx);
            os_condDestroy(&_this->terminate.cnd);
            os_free(_this);
        }
    }
    return result;
}

static void
s_threadsReport(
        ut_thread thread,
        os_boolean alive,
        os_boolean changed,
        const char *info,
        const char *caller,
        void *reportdata,
        void *userdata)
{
    spliced splicedaemon = (spliced)userdata;
    s_threadReportArg *report = (s_threadReportArg*)reportdata;

#ifdef S_DEBUG_THREADS
    static os_timeM lastReport = {OS_TIME_ZERO};
    if (thread != NULL) {
        /* Add this thread info to the overall report. */
        os_uint32 start = report->pos;
        report->pos += ut_threadToString(thread,
                                         alive,
                                         info,
                                         &(report->bfr[report->pos]),
                                         S_REPORT_BUFFER_SIZE - report->pos);
        /* Display this thread when its progress changed. */
        if (changed) {
            s_printEvent(splicedaemon, S_RPTLEVEL_SEVERE, "Thread deadlocked: %s\n", &(report->bfr[start]));
            OS_REPORT(OS_ERROR, caller, 0, "Thread deadlocked: %s", &(report->bfr[start]));
            TRACE_THREADS("Thread deadlocked: %s\n", &(report->bfr[start]));
        }
    } else {
        /* Now we can display the total information. */
        os_timeM tnow = os_timeMGet();
        os_duration dt = os_timeMDiff(tnow, lastReport);
        if (os_durationCompare(dt, s_periodicThreadReport) == OS_MORE) {
            (void)snprintf(&(report->bfr[report->pos]), S_REPORT_BUFFER_SIZE - report->pos, " [%s]", alive ? "OK" : "NOK");
            s_printEvent(splicedaemon, S_RPTLEVEL_FINE, "Threads info: %s\n", report->bfr);
            TRACE_THREADS("Threads info: %s\n", report->bfr);
            lastReport = tnow;
        }
    }
#endif

    if ((thread != NULL) && (changed) && (!alive)) {
#ifndef S_DEBUG_THREADS
        ut_threadToString(thread, alive, info, report->bfr, sizeof(report->bfr));
        s_printEvent(splicedaemon, S_RPTLEVEL_SEVERE, "Thread deadlocked: %s\n", report->bfr);
        OS_REPORT(OS_ERROR, caller, 0, "Thread deadlocked: %s", report->bfr);
        TRACE_THREADS("Thread deadlocked: %s\n", report->bfr);
#endif

        /* When a thread fails to make progress, you'd still like to shutdown
         * as much and as cleanly as possible.
         * This can be done by signaling the spliced to terminate when a thread
         * deadlocks (see s_threadsWatchdogMain()).
         *
         * However, the main thread is different because that is the thread that
         * does the cleanup. All bets are off when the main thread deadlocks.
         */
        if (strcmp(ut_threadGetName(thread), s_main_tread_name) == 0) {
            s_printEvent(splicedaemon, S_RPTLEVEL_SEVERE, "The spliced main thread found not-responding. "
                                                          "A clean shutdown is highly unlikely. "
                                                          "Best action left: abort!\n");
            OS_REPORT(OS_FATAL, caller, 0, "The spliced main thread found not-responding. "
                                           "A clean shutdown is highly unlikely. "
                                           "Best action left: abort!");
            TRACE_THREADS("The spliced main thread found not-responding. "
                          "A clean shutdown is highly unlikely. "
                          "Best action left: abort!\n");
            abort();
        }
    }
}


static void*
s_threadsWatchdogMain(
    void* arg)
{
    s_threadsMonitor _this = (s_threadsMonitor)arg;
    s_threadReportArg report;
    ut_thread self;
    os_boolean ok;
    os_result osr;

    assert(_this);

    self = ut_threadLookupSelf(_this->threads);

    os_mutexLock(&_this->terminate.mtx);

    while(_this->terminate.stop == OS_FALSE) {
        /* Sleep until polling time or termination. */
        osr = ut_condTimedWait(_this->watchdog, &_this->terminate.cnd, &_this->terminate.mtx, POLL_LIMIT);
        if ((osr == os_resultSuccess) ||
            (_this->terminate.stop == OS_TRUE)) {
            /* No action required */
        } else if (osr == os_resultTimeout) {
            /* Condition timed out: poll the threads progress. */
            report.bfr[0] = '\0';
            report.pos = 0;
            ok = ut_threadsAllIsWell(_this->threads, s_threadsReport, &report);
            if (!ok) {
                /* A thread was found deadlocked. */
                if (!splicedIsDoingSystemHalt(_this->spliced)) {
                    /* Try to cleanup as much as possible. */
                    s_printEvent(_this->spliced, S_RPTLEVEL_WARNING, "A spliced thread found not-responding. "
                                                                     "The splice daemon will try to shutdown.\n");
                    OS_REPORT(OS_WARNING, ut_threadGetName(self), 0, "A spliced thread found not-responding. "
                                                                     "The splice daemon will try to shutdown.");
                    TRACE_THREADS("A spliced thread found not-responding. "
                                  "The splice daemon will try to shutdown.\n");
                    splicedSignalTerminate(_this->spliced, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_NOK);
                }
            }
        } else {
            /* Condition problem... */
            _this->terminate.stop = OS_TRUE;
            OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0, "Threads watchdog failed to wait for condition: %s\n", os_resultImage(osr));
        }
    }
    os_mutexUnlock(&_this->terminate.mtx);
    return NULL;
}
