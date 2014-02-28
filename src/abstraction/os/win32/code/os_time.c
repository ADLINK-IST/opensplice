/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <sys/timeb.h>
#include <time.h>

#include "os__debug.h"
#include "os__service.h"
#include "os_thread.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "../common/code/os_time.c"

/* #define OSPL_OS_TIME_SETCLOCKRES */
#ifdef OSPL_OS_TIME_SETCLOCKRES
/* In order to increase the timer resolution past 1ms, below NT-functions are
 * resolved dynamically (in the single-threaded os_timeModuleInit. */
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

static os_time (*_ospl_clockGet)(void) = NULL;
static os_time _ospl_time = {0, 0};
static LONGLONG _ospl_freq = 0; /* frequency of high performance counter */
static LONGLONG _ospl_time_offset = 0;
static int _ospl_time_synched = 0;
static CRITICAL_SECTION _ospl_ctimeLock;

/* default millisecond timer */
static os_time
_msTimeGet(void)
{
    os_time rt;
    struct __timeb64 timebuffer;

    _ftime64(&timebuffer);
    rt.tv_sec = (os_timeSec)timebuffer.time;
    rt.tv_nsec = timebuffer.millitm * 1000000;

    return rt;
}

static os_time
_hrTimeGet(void)
{
    LARGE_INTEGER hpt;
    LONGLONG us_time;
    os_time current_time;
    os_time rt;
    LONGLONG *last_clock_offset;

    last_clock_offset = (LONGLONG *)os_threadMemGet(OS_THREAD_CLOCK_OFFSET);
    if (!last_clock_offset) {
    	last_clock_offset = (LONGLONG *)os_threadMemMalloc(OS_THREAD_CLOCK_OFFSET, sizeof(LONGLONG));
    	*last_clock_offset = 0;
    }

    QueryPerformanceCounter(&hpt);
    us_time = hpt.QuadPart - _ospl_time_offset;
    if (us_time < 0) {
    	(*last_clock_offset)++;
    	us_time = *last_clock_offset;
    }
    rt.tv_sec = (os_timeSec)(us_time / _ospl_freq);
    rt.tv_nsec = (os_int32)(((us_time % _ospl_freq)*1000000000)/_ospl_freq);

    current_time.tv_nsec = _ospl_time.tv_nsec + rt.tv_nsec;
    current_time.tv_sec = _ospl_time.tv_sec + rt.tv_sec;
    if (current_time.tv_nsec >= 1000000000) {
        current_time.tv_sec++;
        current_time.tv_nsec = current_time.tv_nsec - 1000000000;
    }

    return current_time;
}

void
os_timeModuleReinit(
    const os_char* domainName)
{
    os_char* pipename;
    struct os_servicemsg request;
    struct os_servicemsg reply;
    BOOL result;
    DWORD nRead;
    DWORD lastError;

    if(!_ospl_time_synched)
    {
        pipename = os_constructPipeName(domainName);
        if(pipename){

            request.kind = OS_SRVMSG_GET_TIME;
            reply.result = os_resultFail;
            reply.kind = OS_SRVMSG_UNDEFINED;

            do{
                result = CallNamedPipe(
                            TEXT(pipename),
                            &request, sizeof(request),
                            &reply, sizeof(reply),
                            &nRead,
                            NMPWAIT_WAIT_FOREVER);

                if(!result)
                {
                    lastError = GetLastError();
                }
                else
                {
                    lastError = ERROR_SUCCESS;
                }
            }
            while((!result) && (lastError == ERROR_PIPE_BUSY));

            if (result && (nRead == sizeof(reply)))
            {
                if ((reply.result == os_resultSuccess) &&
                    (reply.kind == OS_SRVMSG_GET_TIME))
                {
                    _ospl_time = reply._u.time.start_time;
                    _ospl_time_offset = reply._u.time.time_offset;
                    _ospl_time_synched = 1;

                    OS_REPORT_4(OS_DEBUG, "os_timeModuleReinit", 0,
                            "Synched time with domain '%s'. time=%d,%d, offset=%lld",
                            domainName, _ospl_time.tv_sec, _ospl_time.tv_nsec,
                            _ospl_time_offset);
                }
            } else {
                OS_REPORT_2(OS_ERROR, "os_timeModuleReinit", 0,
                        "Time synchronisation with domain '%s' failed (reason: %d)",
                        domainName, lastError);
            }
            os_free(pipename);
        }
    }
    return;
}

void
os_timeModuleInit(void)
{
    LARGE_INTEGER frequency, qpc;
    struct __timeb64 timebuf;

    InitializeCriticalSection(&_ospl_ctimeLock);

    if (QueryPerformanceFrequency(&frequency) != 0)
    {
        _ospl_freq = frequency.QuadPart;
        _ospl_clockGet = _hrTimeGet;

        QueryPerformanceCounter(&qpc);
        _ftime64_s(&timebuf);

        _ospl_time.tv_sec = (os_timeSec)timebuf.time;
        _ospl_time.tv_nsec = timebuf.millitm * 1000000;
        _ospl_time_offset = qpc.QuadPart;
    }
    else
    {
        /* no high performance timer available!
         * so we fall back to a millisecond clock.
         */
        _ospl_clockGet = _msTimeGet;
        OS_REPORT_1(OS_WARNING, "os_timeModuleInit", 0,
                "No high-resolution timer found (reason: %s), "\
                "switching to millisecond resolution.", GetLastError());
    }

#ifdef OSPL_OS_TIME_SETCLOCKRES
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
            OS_REPORT_1(OS_WARNING, "os_timeModuleInit", 0,
                "Failed to query timer device for maximum timer resolution. "\
                 "Will try to set %.1fms resolution.", timerRes100ns / 10000.0f);
        } else {
            timerRes100ns = maxRes;
        }

        /* Set the resolution regardless of whether the current actual value is
         * already OK. It is a global Windows setting and may change when another
         * application is closed. */
        if(NtSetTimerResolutionFunc(timerRes100ns, TRUE, &actRes) != OS_NT_API_STATUS_SUCCESS) {
            OS_REPORT_2(OS_WARNING, "os_timeModuleInit", 0,
                "Failed to set timer device resolution to %dms, actual resolution: %.1fms",
                timerRes100ns / 10000.0f,
                actRes / 10000.0f);
        } else {
            OS_REPORT_1(OS_INFO, "os_timeModuleInit", 0,
                "Set timer device resolution to %.1fms", timerRes100ns / 10000.0f);
        }
    }
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
}

/** \brief Suspend the execution of the calling thread for the specified time
 *
 * \b os_nanoSleep suspends the calling thread for the required
 * time by calling \b nanosleep. First it converts the \b delay in
 * \b os_time definition into a time in \b struct \b timeval definition.
 * In case the \b nanosleep is interrupted, the call is re-enterred with
 * the remaining time.
 */
os_result
os_nanoSleep (
    os_time delay)
{
    os_result result = os_resultSuccess;
    DWORD dt;

    assert (delay.tv_nsec >= 0);
    assert (delay.tv_nsec < 1000000000);

    if (delay.tv_sec >= 0 ) {
        dt = (DWORD)delay.tv_sec * 1000 + delay.tv_nsec / 1000000;
        Sleep(dt);
    } else {
        /* Negative time-interval should return illegal param error */
        result = os_resultFail;
    }

    return result;
}

/** \brief Get the current time
 *
 * \b os_timeGet gets the current time by calling
 * \b clock_gettime with clock ID \b CLOCK_REALTIME
 * and converting the result in \b struct
 * \b timespec format into \b os_time format.
 */
os_time
os_timeGet(
    void)
{
    os_time result;

    if (_ospl_clockGet) {
        result = _ospl_clockGet();
    } else {
        /* This is not supposed to happen! */
        assert(_ospl_clockGet != NULL);
        result = _msTimeGet();
    }

    return result;
}

/** \brief Set the user clock
 *
 * \b os_timeSetUserClock sets the current time source
 * get function.
 */
void
os_timeSetUserClock(
    os_time (*userClock)(void))
{
    if (userClock) {
        _ospl_clockGet = userClock;
    } else {
        LARGE_INTEGER frequency;
        /* Never set the null pointer, but choose a default
         * in stead */
        if (QueryPerformanceFrequency(&frequency) != 0) {
            _ospl_clockGet = _hrTimeGet;
        } else { /* no high performance timer available!
                    so we fall back to a millisecond clock.
                  */
            _ospl_clockGet = _msTimeGet;
        }
    }
}

/** \brief Get high resolution time
 *
 * Possible Results:
 * - returns "a high resolution time (not necessarily real time)"
 */
os_time
os_hrtimeGet(void)
{
    os_time current_time;
    LARGE_INTEGER timebuffer;

    QueryPerformanceCounter(&timebuffer);
    current_time.tv_sec = (os_timeSec)(timebuffer.QuadPart / _ospl_freq);
    current_time.tv_nsec = (os_int32)(((timebuffer.QuadPart % _ospl_freq)*1000000000)/_ospl_freq);

    return current_time;
}

/** \brief Translate calendar time into readable string representation
 *
 * Possible Results:
 * - returns buf if buf != NULL
 * - returns NULL if buf == NULL
 */
char *
os_ctime_r(
    os_time *t,
    char *buf)
{
    /* Using win32 ctime here */
    if (buf) {
        EnterCriticalSection(&_ospl_ctimeLock);
        os_strcpy(buf, ctime(&t->tv_sec));
        LeaveCriticalSection(&_ospl_ctimeLock);
    }
    return buf;

}
