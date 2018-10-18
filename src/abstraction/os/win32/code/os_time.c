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
/** \file os/mingw3.2.0/code/os_time.c
 *  \brief WIN32 time management
 *
 * Implements time management for WIN32 by
 * including the common services
 * and implementing WIN32 specific
 * os_timeGet and os_nanoSleep
 */
#include <sys/timeb.h>
#include <time.h>

#include "os__debug.h"
#include "os_win32incs.h"
#include "os_errno.h"
#include "os_thread.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os__time.h"
#include "../common/code/os_time.c"
#include "../common/code/os_time_ctime.c"

/* #define OSPL_OS_TIME_SETCLOCKRES */
#ifdef OSPL_OS_TIME_SETCLOCKRES
/* In order to increase the timer resolution past 1ms, below NT-functions are
 * resolved dynamically (in the single-threaded os_timeModuleInit). */
#ifdef STATUS_SUCCESS
#define OS_NT_API_STATUS_SUCCESS  STATUS_SUCCESS
#else
#define OS_NT_API_STATUS_SUCCESS  (0x00000000L)
#endif
static FARPROC NtQueryTimerResolutionFunc = NULL;
static FARPROC NtSetTimerResolutionFunc = NULL;
static HANDLE NTDLLModuleHandle = NULL;
static ULONG timerRes100ns = 10000; /* 1ms, expressed in 100ns */
#endif /* OSPL_OS_TIME_SETCLOCKRES */

static FARPROC QueryUnbiasedInterruptTimeFunc;
/* GetSystemTimeAsFileTimeFunc is set to the most precise time-function (that
 * returns filetime) available. Up until Windows 7 this is GetSystemTimeAsFileTime.
 * On Windows 8 and later this is GetSystemTimePreciseAsFileTime. */
static FARPROC GetSystemTimeAsFileTimeFunc;

/* GetSystemTimeAsFileTime returns the number of 100ns intervals that have elapsed
 * since January 1, 1601 (UTC). There are 11,644,473,600 seconds between 1601 and
 * the Unix epoch (January 1, 1970 (UTC)), which is the reference that is used for
 * os_timeW. */
#define OS_TIME_FILETIME_UNIXEPOCH_OFFSET_SECS (11644473600)
static HANDLE Kernel32ModuleHandle;

void
os_timeModuleInit(void)
{
    /* Resolve the time-functions from the Kernel32-library. */

    /* This os_timeModuleInit is currently called from DllMain. This means
     * we're not allowed to do LoadLibrary. One exception is "Kernel32.DLL",
     * since that library is per definition loaded (LoadLibrary itself
     * lives there). And since we're only resolving stuff, not actually
     * invoking, this is considered safe. */
    Kernel32ModuleHandle = LoadLibrary("Kernel32.DLL");
    assert(Kernel32ModuleHandle);

    GetSystemTimeAsFileTimeFunc = GetProcAddress(Kernel32ModuleHandle, "GetSystemTimePreciseAsFileTime");
    if(GetSystemTimeAsFileTimeFunc == NULL){
        /* Fallback to GetSystemTimeAsFileTime when Precise variant isn't
         * available. */
        GetSystemTimeAsFileTimeFunc = GetProcAddress(Kernel32ModuleHandle, "GetSystemTimeAsFileTime");
    }
    /* GetSystemTimeAsFileTimeFunc should be available since Windows 2000, so
     * this can't fail. */
    assert(GetSystemTimeAsFileTimeFunc);

    /* Resolve the QueryUnbiasedInterruptTime function so it can be used
     * for the monotonic continuous clock if available. Otherwise fall-back
     * on QueryPerformanceCounter (losing the continuity during sleep/
     * hibernate; QueryPerformanceCounter counts the time spent during
     * sleep on all (and hibernate on some) platforms). */
    QueryUnbiasedInterruptTimeFunc = GetProcAddress(Kernel32ModuleHandle, "QueryUnbiasedInterruptTime");

#ifdef OSPL_OS_TIME_SETCLOCKRES
    /* os_timeModuleInit is called before os_reportInit, currently not possible
     * to do reporting before os_reportInit. See OSPL-6126. */
    /* The default Windows scheduling resolution is 15.6ms. This isn't sufficient
     * for networking configurations, so increase the resolution to the maximally
     * supported resolution.
     * TODO: Optimally this should be set based on knowledge regarding the
     * actually required resolution. A machine may not perform optimally on its
     * maximum resolution. */

    if((NTDLLModuleHandle = LoadLibrary("NTDLL.DLL")) == NULL){
        OS_REPORT(OS_WARNING, "os_timeModuleInit", 0,
            "Failed to load NTDLL.DLL for increasing timer resolution; cannot increase timer resolution.");
        goto err_load_ntdll;
    }

    /* Retrieve function pointers */
    NtQueryTimerResolutionFunc = GetProcAddress(NTDLLModuleHandle, "NtQueryTimerResolution");
    NtSetTimerResolutionFunc = GetProcAddress(NTDLLModuleHandle, "NtSetTimerResolution");

    if((NtQueryTimerResolutionFunc == NULL) || (NtSetTimerResolutionFunc == NULL)) {
        OS_REPORT(OS_WARNING, "os_timeModuleInit", 0,
            "Failed to resolve entry point for NtQueryTimerResolution or NtQueryTimerResolution; cannot increase timer resolution");
        goto err_resolve_funcs;
    }

    {
        ULONG minRes, maxRes, actRes;

        if(NtQueryTimerResolutionFunc(&minRes, &maxRes, &actRes) != OS_NT_API_STATUS_SUCCESS){
            OS_REPORT(OS_WARNING, "os_timeModuleInit", 0,
                "Failed to query timer device for maximum timer resolution. "\
                 "Will try to set %.1fms resolution.", timerRes100ns / 10000.0f);
        } else {
            timerRes100ns = maxRes;
        }

        /* Set the resolution regardless of whether the current actual value is
         * already OK. It is a global Windows setting and may change when another
         * application is closed. */
        if(NtSetTimerResolutionFunc(timerRes100ns, TRUE, &actRes) != OS_NT_API_STATUS_SUCCESS) {
            OS_REPORT(OS_WARNING, "os_timeModuleInit", 0,
                "Failed to set timer device resolution to %dms, actual resolution: %.1fms",
                timerRes100ns / 10000.0f,
                actRes / 10000.0f);
        } else {
            OS_REPORT(OS_INFO, "os_timeModuleInit", 0,
                "Set timer device resolution to %.1fms", timerRes100ns / 10000.0f);
        }
    }
    /* Initialize the elapsed start time and monotonic start time */
    monotonicStartTime = os_timeMGet();
    elapsedStartTime = os_timeEGet();
    return;

err_resolve_funcs:
    NtQueryTimerResolutionFunc = NULL;
    NtSetTimerResolutionFunc = NULL;
    FreeLibrary(NTDLLModuleHandle);
    NTDLLModuleHandle = NULL;
err_load_ntdll:
    return;
#endif /* OSPL_OS_TIME_SETCLOCKRES */
}

void
os_timeModuleExit(void)
{
#ifdef OSPL_OS_TIME_SETCLOCKRES
    /* os_timeModuleExit is called after os_reportExit, currently not possible
     * to do reporting after os_reportExit. See OSPL-6126. */
    /* On Windows the init- and exit are single-threaded */
    if(NtSetTimerResolutionFunc){
        ULONG actRes;
        /* Restore the resolution. */
        if(NtSetTimerResolutionFunc(timerRes100ns, FALSE, &actRes) != OS_NT_API_STATUS_SUCCESS) {
            OS_REPORT(OS_WARNING, "os_timeModuleExit", 0,
                "Failed to restore timer device resolution");
        }
    }

    if(NTDLLModuleHandle){
        NtQueryTimerResolutionFunc = NULL;
        NtSetTimerResolutionFunc = NULL;
        FreeLibrary(NTDLLModuleHandle);
        NTDLLModuleHandle = NULL;
    }
#endif /* OSPL_OS_TIME_SETCLOCKRES */

    if(Kernel32ModuleHandle){
        QueryUnbiasedInterruptTimeFunc = NULL;
        GetSystemTimeAsFileTimeFunc = NULL;
        FreeLibrary(Kernel32ModuleHandle);
        Kernel32ModuleHandle = NULL;
    }
}

/** \brief Suspend the execution of the calling thread for the specified time
 *
 * \b os_nanoSleep suspends the calling thread for the required
 * time by calling \b nanosleep. First it converts the \b delay in
 * \b os_duration definition into a time in \b struct \b timeval definition.
 * In case the \b nanosleep is interrupted, the call is re-enterred with
 * the remaining time.
 */
os_result
ospl_os_sleep (
    os_duration delay)
{
    os_result result = os_resultSuccess;
    DWORD dt;

    if (OS_DURATION_ISPOSITIVE(delay)) {
        dt = (DWORD)(delay/1000000);
        Sleep(dt);
    } else {
        result = os_resultFail;
    }

    return result;
}

static os_time
os__timeDefaultTimeGet(void)
{
    FILETIME ft;
    ULARGE_INTEGER ns100;
    os_time current_time;

    /* GetSystemTime(Precise)AsFileTime returns the number of 100-nanosecond
     * intervals since January 1, 1601 (UTC).
     * GetSystemTimeAsFileTime has a resolution of approximately the
     * TimerResolution (~15.6ms) on Windows XP. On Windows 7 it appears to have
     * sub-millisecond resolution. GetSystemTimePreciseAsFileTime (available on
     * Windows 8) has sub-microsecond resolution.
     *
     * This call appears to be significantly (factor 8) cheaper than the
     * QueryPerformanceCounter (on the systems performance was measured on).
     *
     * TODO: When the API is extended to support retrieval of clock-properties,
     * then the actual resolution of this clock can be retrieved using the
     * GetSystemTimeAdjustment. See for example OSPL-4394.
     */
    assert(GetSystemTimeAsFileTimeFunc);
    GetSystemTimeAsFileTimeFunc(&ft);
    ns100.LowPart = ft.dwLowDateTime;
    ns100.HighPart = ft.dwHighDateTime;
    current_time.tv_sec = (os_timeSec)((ns100.QuadPart / 10000000) - OS_TIME_FILETIME_UNIXEPOCH_OFFSET_SECS);
    current_time.tv_nsec = (os_int32)((ns100.QuadPart % 10000000) * 100);

    return current_time;
}

static os_timeW
os__timeWGet (
    void)
{
    os_time default_time;
    os_timeW current_time;

    default_time = os__timeDefaultTimeGet();
    current_time = OS_TIMEW_INIT(default_time.tv_sec, default_time.tv_nsec);

    return current_time;
}

/** \brief Get high resolution, monotonic time.
 *
 */
os_timeM
os_timeMGet (
    void)
{
    os_timeM t;
    if(QueryUnbiasedInterruptTimeFunc){
        ULONGLONG ubit;

        (void) QueryUnbiasedInterruptTimeFunc(&ubit); /* 100ns ticks */

        t.mt = ubit*100;
        t = OS_TIMEM_NORMALIZE(t);
    } else {
        os_timeE e = os_timeEGet();
        /* Fall-back on QueryPerformanceCounter. */
        t.mt = e.et;
    }
    return t;
}

/** \brief Get high resolution, elapsed time.
 *
 */
os_timeE
os_timeEGet (
    void)
{
    os_timeE t;
    LARGE_INTEGER qpc;
    static LONGLONG qpc_freq;

    /* The QueryPerformanceCounter has a bad reputation, since it didn't behave
     * very well on older hardware. On recent OS's (Windows XP SP2 and later)
     * things have become much better, especially when combined with new hard-
     * ware.
     *
     * There is currently one bug which is not fixed, which may cause forward
     * jumps. This is currently not really important, since a forward jump may
     * be observed anyway due to the system going to standby. There is a work-
     * around available (comparing the progress with the progress made by
     * GetTickCount), but for now we live with a risk of a forward jump on buggy
     * hardware. Since Microsoft does maintain a list of hardware which exhibits
     * the behaviour, it is possible to only have the workaround in place only
     * on the faulty hardware (see KB274323 for a list and more info).
     *
     * TODO: When the API is extended to support retrieval of clock-properties,
     * then the discontinuous nature (when sleeping/hibernating) of this
     * clock and the drift tendency should be reported. See for example
     * OSPL-4394. */

    if (qpc_freq == 0){
        /* This block is idempotent, so don't bother with synchronisation */
        LARGE_INTEGER frequency;

        if(QueryPerformanceFrequency(&frequency)){
            qpc_freq = frequency.QuadPart;
        }
        /* Since Windows XP SP2 the QueryPerformanceCounter is abstracted,
         * so QueryPerformanceFrequency is not expected to ever return 0.
         * That't why there is no fall-back for the case when no
         * QueryPerformanceCounter is available. */
    }
    assert(qpc_freq);

    /* The QueryPerformanceCounter tends to drift quite a bit, so in order to
     * accurately measure longer periods with it, there may be a need to sync
     * the time progression to actual time progression (with a PLL for example
     * as done by EB for CAE). */
    QueryPerformanceCounter(&qpc);

    t = OS_TIMEE_INIT(qpc.QuadPart / qpc_freq,
                      ((qpc.QuadPart % qpc_freq) * 1000000000) / qpc_freq);
    t = OS_TIMEE_NORMALIZE(t);

    return t;
}

