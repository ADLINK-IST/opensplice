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
/** \file os/mingw3.2.0/code/os_time.c
 *  \brief WIN32 time management
 *
 * Implements time management for WIN32 by
 * including the common services
 * and implementing WIN32 specific
 * os_timeGet and os_nanoSleep
 */
#include <time.h>

#include "code/os__debug.h"
#include "os__service.h"
#include "os_thread.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "os_init.h"
#include "os_heap.h"

#include "../common/code/os_time.c"

/* GetCurrentFT returns the number of 100ns intervals that have elapsed since
 * January 1, 1601 (UTC). There are 11,644,473,600 seconds between 1601 and the
 * Unix epoch (January 1, 1970 (UTC)), which is the reference that is used for
 * os_timeW. */
#define OS_TIME_FILETIME_UNIXEPOCH_OFFSET_SECS (11644473600)
static HANDLE   CoreDLLLibModuleHandle;
static FARPROC  CeGetRawTimeFunc;

static os_timeW
os__timeWGet(
    void)
{
    FILETIME ft;
    ULARGE_INTEGER ns100;
    os_timeW current_time;

    /* GetCurrentFT returns the number of 100-nanosecond
     * intervals since January 1, 1601 (UTC). */
    GetCurrentFT(&ft);
    ns100.LowPart = ft.dwLowDateTime;
    ns100.HighPart = ft.dwHighDateTime;

    current_time = OS_TIMEW_INIT((ns100.QuadPart / 10000000) - OS_TIME_FILETIME_UNIXEPOCH_OFFSET_SECS,
                                 (ns100.QuadPart % 10000000) * 100);

    return current_time;
}

/** \brief Get high resolution, monotonic time.
 *
 */
os_timeM
os_timeMGet(
    void)
{
    os_timeM t;
    LARGE_INTEGER qpc;
    static LONGLONG qpc_freq;

    /* The QueryPerformanceCounter on WinCE may be mapped by an OEM to high-
     * resolution performance counter hardware. If such hardware isn't available,
     * the QueryPerformanceFrequency will be 1000, because the API then defaults
     * to GetTickCount.
     *
     * Only in case of actual high-resolution performance counter hardware, will
     * this call have a resolution better than 1ms. It is expected to be prone
     * to drifting in either case and whether time spent during suspend is
     * excluded is unknown. The fall-back on GetTickCount at least implies that
     * for that situation, time spent in suspended mode isn't included.
     *
     * TODO: When the API is extended to support retrieval of clock-properties,
     * then the discontinuous nature (when sleeping/hibernating) of this
     * clock and the drift tendency should be reported when it can be detected.
     * See for example OSPL-4394. */
    if (qpc_freq == 0){
        /* This block is idempotent, so don't bother with synchronisation */
        LARGE_INTEGER frequency;

        if(QueryPerformanceFrequency(&frequency)){
            qpc_freq = frequency.QuadPart;
        }
        /* On WinCE the QueryPerformanceCounter defaults to GetTickCount when it
         * is not available, so QueryPerformanceFrequency is not expected to ever
         * return 0. That't why there is no fall-back for the case when no
         * QueryPerformanceCounter is available.
         * In case no actual high-resolution hardware is present, the returned
         * frequency will be 1000. */
    }
    assert(qpc_freq);

    /* The QueryPerformanceCounter tends to drift quite a bit, so in order to
     * accurately measure longer periods with it, there may be a need to sync
     * the time progression to actual time progression (with a PLL for example
     * as done by EB for CAE). */
    QueryPerformanceCounter(&qpc);

    t = OS_TIMEM_INIT(qpc.QuadPart / qpc_freq,
                      ((qpc.QuadPart % qpc_freq) * 1000000000) / qpc_freq) ;

    return t;
}

/** \brief Get high resolution, elapsed time.
 *
 */
os_timeE
os_timeEGet(void)
{
    os_timeE t;

    if(CeGetRawTimeFunc){
        ULONGLONG ubit;

        /* The CeGetRawTime is monotonic and guaranteed to run at the speed of
         * the RTC. It provides an elapsed time measurement. It is available on
         * Windows Embedded CE 6.0 and later.
         *
         * Resolution can be 1ms at best, since the returned value is in ms. */
        (void) CeGetRawTimeFunc(&ubit); /* ms */

        t.et = ubit*OS_TIME_MILLISECOND;
        t = OS_TIMEE_NORMALIZE(t);
    } else {
        os_timeM mt = os_timeMGet();
        t.et = mt.mt;
    }

    return t;
}

void
os_timeModuleInit(void)
{
    /* Resolve the time-functions from the coredll-library. */

    /* This os_timeModuleInit is currently called from DllMain. This means
     * we're not allowed to do LoadLibrary. One exception is "coredll.lib",
     * since that library is per definition loaded (LoadLibrary itself
     * lives there). And since we're only resolving stuff, not actually
     * invoking, this is considered safe. */
    CoreDLLLibModuleHandle = LoadLibrary("coredll.lib");
    assert(CoreDLLLibModuleHandle);

    /* Resolve the CeGetRawTime function so it can be used for the elapsed
     * clock if available (Windows Embedded CE 6.0 and later). */
    CeGetRawTimeFunc = GetProcAddress(CoreDLLLibModuleHandle, "CeGetRawTime");
}


void
os_timeModuleExit(void)
{
    if(CoreDLLLibModuleHandle){
        CeGetRawTimeFunc = NULL;
        FreeLibrary(CoreDLLLibModuleHandle);
        CoreDLLLibModuleHandle = NULL;
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
        /* Negative time-interval should return illegal param error */
        result = os_resultFail;
    }

    return result;
}

/** \brief Translate calendar time into readable string representation
 *
 * Possible Results:
 * - returns buf if buf != NULL
 * - returns NULL if buf == NULL
 */
os_size_t
os_ctimeW_r(
    os_timeW *t,
    char *buf,
    os_size_t bufsz)
{
    os_size_t result = 0;
    SYSTEMTIME systemTime;
    FILETIME systemTimeSince1601;
    DWORD64 dw64Time;
    DWORD64 dw64MAXDWORD = MAXDWORD;
    wchar_t format[32];
    char *fstr;

    /* Using win32 ctime here */
    if (buf)
    {
        int iSizeOfBuffer;

        dw64Time = (t->wt)/100;

        systemTimeSince1601.dwHighDateTime = (dw64Time / (dw64MAXDWORD+1));
        systemTimeSince1601.dwLowDateTime = (dw64Time % (dw64MAXDWORD+1));
        FileTimeToSystemTime(&systemTimeSince1601, &systemTime);

        /* Time is in UTC here */
        GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &systemTime, L"yyyy'-'MM'-'dd'T'HH':'mm':'ss", format, 32);
        /* convert wide str to multi byte str */
        fstr = wce_wctomb(format);
        result = snprintf(buf, bufsz, "%s", fstr);
        os_free(fstr);
    }
    return result;

}
