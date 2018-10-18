/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "os_time.h"
#include "os_report.h"

#include "d__misc.h"
#include "d__types.h"
#include "d__thread.h"
#include "u_service.h"

/* Maximum number of threads has a hard-coded limit for simplicity in
   thread management code.  Overestimating it means a little more
   memory is used and that the service lease renewing touches a bit
   more memory (and takes a bit more time). */
#define D_MAX_THREADS 32

#ifndef D_LOG_STACKTRACE
#define log_stacktrace(...)
#else
static void log_stacktrace(const char*, os_threadId);
#endif

static ut_threads d_threads = NULL;

d_thread d_threadLookupId (os_threadId tid)
{
    assert(d_threads);
    return (d_thread)ut_threadLookupId(d_threads, tid);
}

d_thread d_threadLookupSelf (void)
{
    assert(d_threads);
    return (d_thread)ut_threadLookupSelf(d_threads);
}

d_durability d_threadsDurability()
{
    assert(d_threads);
    return (d_durability)ut_threadsUserData(d_threads);
}

os_result d_threadsInit (d_durability durability)
{
    os_result osr = os_resultSuccess;
    d_threads = ut_threadsNew("ospl_durability", UT_SLEEP_INDEFINITELY, D_MAX_THREADS, (void*)durability);
    if (d_threads == NULL) {
        OS_REPORT(OS_ERROR, D_CONTEXT, 0, "Could not create durability threads management.");
        osr = os_resultFail;
    }
    return osr;
}

void d_threadsDeinit (void)
{
    if (d_threads != NULL) {
        ut_threadsFree(d_threads);
        d_threads = NULL;
    }
}

struct d_threadArguments {
    u_domainId_t domainId;
    os_threadRoutine start_routine;
    void *arguments;
};

static void *
d_threadWrapperRoutine(
    void *args)
{
    struct d_threadArguments *ti = args;
    os_threadRoutine start_routine;
    void *arguments;

    assert(ti);

    u_serviceThreadSetDomainId(ti->domainId);

    start_routine = ti->start_routine;
    arguments = ti->arguments;

    os_free(ti);

    return start_routine(arguments);
}

os_result d_threadCreate (os_threadId *threadId, const char *name, const os_threadAttr *threadAttr, os_threadRoutine start_routine, void *arg)
{
    os_result osr = os_resultFail;
    ut_thread thr;
    struct d_threadArguments *ti;

    assert(d_threads);

    ti = os_malloc(sizeof(*ti));
    ti->domainId = u_serviceThreadGetDomainId();
    ti->start_routine = start_routine;
    ti->arguments = arg;

    ut_threadCreate(d_threads, &thr, name, threadAttr, d_threadWrapperRoutine, ti);
    if (thr != NULL) {
        *threadId = ut_threadGetId(thr);
        osr = os_resultSuccess;
    } else {
        os_free(ti);
    }
    return osr;
}

os_result d_threadWaitExit (os_threadId threadId, void **thread_result)
{
    os_result osr = os_resultFail;
    ut_thread thr;
    assert(d_threads);
    thr = ut_threadLookupId(d_threads, threadId);
    if (thr != NULL) {
        osr = ut_threadWaitExit(thr, thread_result);
    }
    return osr;
}

void d_threadLivelinessInit (os_duration min_intv)
{
    assert(d_threads);
    ut_threadsSetInterval(d_threads, min_intv);
}

#define D_REPORT_BUFFER_SIZE (2048)

typedef struct {
    char *buffer;
    os_uint32 size;
    os_uint32 pos;
    os_timeM currentTime;
    os_duration updateInterval;
} d_threadReportArg;

static void
d_threadsReport(
    ut_thread thread,
    os_boolean alive,
    os_boolean changed,
    const char *info,
    const char *caller,
    void *reportdata,
    void *userdata)
{
    static os_timeM lastReport = {OS_TIME_ZERO};
    struct d_durability_s * durability = (struct d_durability_s*)userdata;
    d_threadReportArg *report = (d_threadReportArg*)reportdata;
    os_uint32 length = (report->size > report->pos) ? report->size - report->pos : 0;
    os_uint32 start = report->pos;
    os_boolean print;

    OS_UNUSED_ARG(caller);

    assert(length != 0); /* size of the buffer is too small */

    /* No need to do anything if nothing changed and
     * no periodic thread overview required */
    if (!changed && (os_durationCompare(report->updateInterval, OS_DURATION_ZERO) != OS_MORE)) {
        return;
    }

    /* Decide whether to print an overview now */
    print = (os_durationCompare(os_timeMDiff(report->currentTime, lastReport), report->updateInterval) == OS_MORE);

    /* Always print log message when liveliness of a thread has changed
     * In case a message must be printed periodically we need to build up a string
     * containing the relevant liveliness information for this thread  */
    if (thread != NULL) {
        if (changed || print) {
            report->pos += ut_threadToString(thread,
                                             alive,
                                             info,
                                             &(report->buffer[start]),
                                             length);
        }
        if (changed) {
            d_printTimedEvent(durability, alive ?  D_LEVEL_INFO : D_LEVEL_WARNING, "Progress changed: %s\n", &(report->buffer[start]));
        }
    }
    else if (print) {
        (void)snprintf(&(report->buffer[start]), length, " [%s]", alive ? "OK" : "NOK");
         d_printTimedEvent(durability, alive ? D_LEVEL_INFO : D_LEVEL_WARNING, "Threads info: %s\n", report->buffer);
        /* Remember the last reported liveliness */
        lastReport = report->currentTime;
    }
}

/**
 * \brief              This operation checks if all threads are alive and generates
 *                     a periodic durability report message.
 *
 * \param currentTime : Current timestamp used for determining if report should be generated.
 * \param updateInterval : Minimum period between generated report messages.
 *
 * \return           : TRUE when all threads are alive
 *                     FALSE otherwise
 */
os_boolean d_threadLivelinessCheck (os_timeM currentTime, os_duration updateInterval)
{
    d_threadReportArg arg;
    char buffer[D_REPORT_BUFFER_SIZE];

    assert(d_threads);

    buffer[0] = '\0';
    arg.buffer = &buffer[0];
    arg.size = D_REPORT_BUFFER_SIZE;
    arg.currentTime = currentTime;
    arg.updateInterval = updateInterval;
    arg.pos = 0;

    return ut_threadsAllIsWell(d_threads, d_threadsReport, &arg);
}

const char* d_threadSelfName()
{
    static char t_main[] = "<main>";        /* printed when d_threads not yet initialized (only occurs for main thread) */
    static char t_unknown[] = "<unknown>";  /* printed when d_threads initialized but lookup failed */
    ut_thread thr;
    if (!d_threads) {
        return t_main;
    }
    thr = ut_threadLookupSelf(d_threads);
    if (thr) {
        return ut_threadGetName(thr);
    }
    return t_unknown;
}

#ifdef D_LOG_STACKTRACE

#include <execinfo.h>
#include <os_atomics.h>

static pa_uint32_t log_stacktrace_flag = PA_UINT32_INIT(0);

static void log_stacktrace_sigh (int sig __attribute__ ((unused)))
{
  void *stk[64];
  char **strs;
  int i, n;
  struct d_durability_s *durability;
  const char *thrName;

  assert(d_threads);
  durability = (struct d_durability_s*)ut_threadsUserData(d_threads);
  thrName = d_threadSelfName();

  d_printTimedEvent(durability, D_LEVEL_FINEST, thrName, "-- stack trace follows --\n");
  n = backtrace (stk, (int) (sizeof (stk) / sizeof (*stk)));
  strs = backtrace_symbols (stk, n);
  for (i = 0; i < n; i++) {
      d_printTimedEvent(durability, D_LEVEL_FINEST, thrName, "%s\n", strs[i]);
  }

  free (strs);
  d_printTimedEvent(durability, D_LEVEL_FINEST, thrName, "-- end of stack trace -- \n");
  pa_inc32 (&log_stacktrace_flag);
}

static void log_stacktrace (const char *name, os_threadId tid)
{
    const os_duration d = OS_DURATION_INIT(0, 1000000);
    struct sigaction act, oact;
    struct d_durability_s *durability;
    const char *thrName;

    assert(d_threads);
    durability = (struct d_durability_s*)ut_threadsUserData(d_threads);
    thrName = d_threadSelfName();

    d_printTimedEvent(durability, D_LEVEL_FINEST, thrName, "-- stack trace of %s requested --\n", name);
    act.sa_handler = log_stacktrace_sigh;
    act.sa_flags = 0;
    (void)sigfillset (&act.sa_mask);
    while (!pa_cas32 (&log_stacktrace_flag, 0, 1))
      ospl_os_sleep (d);
    sigaction (SIGXCPU, &act, &oact);
    pthread_kill (tid, SIGXCPU);
    while (!pa_cas32 (&log_stacktrace_flag, 2, 3))
      ospl_os_sleep (d);
    sigaction (SIGXCPU, &oact, NULL);
    pa_st32 (&log_stacktrace_flag, 0);
}

#endif /* D_LOG_STACKTRACE */
