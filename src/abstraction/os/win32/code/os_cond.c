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
/** \file os/win32/code/os_cond.c
 *  \brief WIN32 condition varaibles
 *
 * Implements condition variables
 */

#define _WIN32_WINNT 0x0400

#include "os_cond.h"
#include "os_stdlib.h"
#include "os_report.h"
#include "os_init.h"
#include "os__debug.h"
#include "os__service.h"
#include "os__sharedmem.h"

#include <stdio.h>
#include <assert.h>

#include "../common/code/os_cond_attr.c"

#define BROADCAST_BIT_MASK (0x40000000)
#define SIGNAL_BIT_MASK    (0x20000000)
#define WAITCOUNT_MASK     (0x9fffffff)

#define NOTIFIED_BY_BROADCAST(state) ((state&BROADCAST_BIT_MASK)==BROADCAST_BIT_MASK)
#define WAITCOUNT(state) (state&WAITCOUNT_MASK)


/************ PRIVATE ****************/
#ifndef NTDDI_WIN8
static LONG
#ifdef _WIN64
InterlockedAndAquire(
#else
InterlockedAnd(
#endif
    __inout LONG volatile *t,
    __in LONG mask)
{
    LONG j;
    LONG i;
    j = *t;
    do {
        i = j;
        j = InterlockedCompareExchange(t,i&mask,i);

    } while (i != j);
    return j;
}

static LONG
#ifdef _WIN64
InterlockedOrAquire(
#else
InterlockedOr(
#endif
    __inout LONG volatile *t,
    __in LONG mask)
{
    LONG j;
    LONG i;
    j = *t;
    do {
        i = j;
        j = InterlockedCompareExchange(t,i|mask,i);
    } while (i != j);
    return j;
}
#else

#if !defined(_M_AMD64) && !defined(_M_IA64) && !defined(_M_X64)
#include <intrin.h>

#pragma intrinsic (_InterlockedAnd)
#define InterlockedAnd _InterlockedAnd

#pragma intrinsic (_InterlockedOr)
#define InterlockedOr _InterlockedOr
#endif

#endif

static os_result
getSem(
    os_cond* cond)
{
    const char *pipename;
    struct os_servicemsg request;
    struct os_servicemsg reply;
    BOOL result;
    DWORD nRead;
    os_result osr;
    DWORD lastError;

    pipename = os_createPipeNameFromCond(cond);
    if (pipename == NULL) {
       pipename = os_servicePipeName();
       OS_DEBUG_1("getSem", "Failed to get a domain name from cond, using default %s", pipename);
    }
    request.kind = OS_SRVMSG_CREATE_SEMAPHORE;
    reply.result = os_resultFail;
    reply.kind = OS_SRVMSG_UNDEFINED;

    do{
        result = CallNamedPipe(
                 pipename,
                 &request, sizeof(request),
                 &reply, sizeof(reply),
                 &nRead,
                 NMPWAIT_WAIT_FOREVER);
        if(!result){
           lastError = GetLastError();
        } else {
            lastError = ERROR_SUCCESS;
        }
    } while((!result) && (lastError == ERROR_PIPE_BUSY));

    if (!result || (nRead != sizeof(reply))) {
        OS_DEBUG_4("getSem", "Failure %d %d %d %d\n", result, GetLastError(), nRead, reply.kind);
        osr = os_resultFail;
    } else {
        if ((reply.result == os_resultSuccess) &&
            (reply.kind == OS_SRVMSG_CREATE_SEMAPHORE)) {
            cond->qId = reply._u.id;
            osr = os_resultSuccess;
        } else {
            osr = os_resultFail;
        }
    }
    return osr;
}

static os_result
returnSem(
    os_cond* cond)
{
    const char *pipename;
    struct os_servicemsg request;
    struct os_servicemsg reply;
    BOOL result;
    DWORD nRead;
    os_result osr;

    pipename = os_createPipeNameFromCond(cond);
    if (pipename == NULL) {
       pipename = os_servicePipeName();
       OS_DEBUG_1("returnSem", "Failed to get a domain name from cond, using default %s", pipename);
    }
    request.kind = OS_SRVMSG_DESTROY_SEMAPHORE;
    request._u.id = cond->qId;
    reply.result = os_resultFail;
    reply.kind = OS_SRVMSG_UNDEFINED;
    result = CallNamedPipe(
                 pipename,
                 &request, sizeof(request),
                 &reply, sizeof(reply),
                 &nRead,
                 NMPWAIT_WAIT_FOREVER);
    if ((reply.result == os_resultSuccess) &&
        (reply.kind = OS_SRVMSG_DESTROY_SEMAPHORE)) {
        osr = os_resultSuccess;
    } else {
        OS_DEBUG_4("returnSem", "Failure %d %d %d %d\n", result, GetLastError(), nRead, reply.kind);
        osr = os_resultFail;
    }

    return osr;
}

static os_result
condTimedWait(
    os_cond *cond,
    os_mutex *mutex,
    DWORD timeout)
{
    HANDLE hQueue;
    HANDLE hMtx;
    char name[OS_SERVICE_ENTITY_NAME_MAX];
    DWORD wsr;
    LONG c;
    LONG lockCount;
    os_result osr;
    os_result result;

    assert(cond != NULL);
    assert(mutex != NULL);

    result = os_resultSuccess;
    if (cond->scope == OS_SCOPE_SHARED) {
        if (_snprintf(name, sizeof(name), "%s%s%d%s",
            (os_sharedMemIsGlobal() ? OS_SERVICE_GLOBAL_NAME_PREFIX : ""),
            OS_SERVICE_SEM_NAME_PREFIX,
            cond->qId,
            os_getShmDomainKeyForPointer((void*)cond)) <= 0) {
            OS_REPORT_1(OS_ERROR, "condTimedWait", 0,
                "Semaphore name exceeds maximum allowed length (%d)", OS_SERVICE_ENTITY_NAME_MAX);
            return os_resultFail;
        }

        hQueue = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, name);
        if (hQueue == NULL) {
            OS_DEBUG_2("condTimedWait", "OpenSemaphore with name %s failed (Error: %d)", name, (int)GetLastError());
            assert(0);
            return os_resultFail;
        }

        if (_snprintf(name, sizeof(name), "%s%s%d%s",
            (os_sharedMemIsGlobal() ? OS_SERVICE_GLOBAL_NAME_PREFIX : ""),
            OS_SERVICE_EVENT_NAME_PREFIX,
            mutex->id,
            os_getShmDomainKeyForPointer((void*)cond)) <= 0) {
            OS_REPORT_1(OS_ERROR, "condTimedWait", 0,
                "Event name exceeds maximum allowed length (%d)", OS_SERVICE_ENTITY_NAME_MAX);
            return os_resultFail;
        }

        hMtx = OpenEvent(EVENT_ALL_ACCESS, FALSE, name);
        if (hMtx == NULL) {
            OS_DEBUG_2("condTimedWait", "OpenEvent with name %s failed (Error: %d)", name, (int)GetLastError());
            CloseHandle(hQueue);
            assert(0);
            return os_resultFail;
        }
    } else {
        hQueue  = (HANDLE)cond->qId;
        hMtx  = (HANDLE)mutex->id;
    }

    InterlockedIncrement(&cond->state);
    lockCount = InterlockedDecrement(&mutex->lockCount);
    if (lockCount > 0) {
        wsr = SignalObjectAndWait(hMtx, hQueue, timeout, FALSE);
    } else {
        wsr = WaitForSingleObject(hQueue, timeout);
    }
    assert((wsr == WAIT_OBJECT_0) || (wsr == WAIT_FAILED) || (wsr == WAIT_ABANDONED) || (wsr == WAIT_TIMEOUT));
    if (wsr == WAIT_TIMEOUT) {
        result = os_resultTimeout;
    } else if (wsr != WAIT_OBJECT_0) {
        result = os_resultFail;
    }

    c = InterlockedDecrement(&cond->state);
    osr = os_mutexLock(mutex);
    if (osr != os_resultSuccess) {
        result = osr;
    }
    if (cond->scope == OS_SCOPE_SHARED) {
        CloseHandle(hQueue);
        CloseHandle(hMtx);
    }

    return result;
}

static os_result
condSignal(
    os_cond *cond,
    long mask)
{
    char name[OS_SERVICE_ENTITY_NAME_MAX];
    HANDLE hQueue;
    DWORD result;
    long oldState;
    os_result osr;

    assert(cond != NULL);

    osr = os_resultSuccess;

    if (cond->scope == OS_SCOPE_SHARED) {

        if (_snprintf(name, sizeof(name), "%s%s%d%s",
            (os_sharedMemIsGlobal() ? OS_SERVICE_GLOBAL_NAME_PREFIX : ""),
            OS_SERVICE_SEM_NAME_PREFIX,
            cond->qId,
            os_getShmDomainKeyForPointer((void*)cond)) <= 0) {
            OS_REPORT_1(OS_ERROR, "condSignal", 0,
                "Semaphore name exceeds maximum allowed length (%d)", OS_SERVICE_ENTITY_NAME_MAX);
            return os_resultFail;
        }

        hQueue = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, name);
        if (hQueue == NULL) {
            OS_DEBUG_2("condSignal", "OpenSemaphore with name %s failed (Error: %d)", name, (int)GetLastError());
            assert(0);
            return os_resultFail;
        }

    } else {
        hQueue = (HANDLE)cond->qId;
    }

    oldState = InterlockedOr(&cond->state, mask);
    if (oldState == 0) { /* no waiters */
        InterlockedAnd(&cond->state, ~mask);
        return osr;
    }

    if (mask == BROADCAST_BIT_MASK) {
        result = ReleaseSemaphore(hQueue, oldState, 0);
    } else {
        result = ReleaseSemaphore(hQueue, 1, 0);
    }
    InterlockedAnd(&cond->state, ~mask);

    if (cond->scope == OS_SCOPE_SHARED) {
        CloseHandle(hQueue);
    }

    return osr;
}

/************ PUBLIC *****************/

/** \brief Initialize the condition variable taking the condition
 *         attributes into account
 *
 * \b os_condInit calls \b pthread_cond_init to intialize the posix condition
 * variable.
 *
 * In case the scope attribute is \b OS_SCOPE_SHARED, the posix
 * condition variable "pshared" attribute is set to \b PTHREAD_PROCESS_SHARED
 * otherwise it is set to \b PTHREAD_PROCESS_PRIVATE.
 *
 * When in single process mode, a request for a SHARED variable will
 * implictly create a PRIVATE equivalent.  This is an optimisation
 * because there is no need for "shared" multi process variables in
 * single process mode.
 */
os_result
os_condInit (
    os_cond *cond,
    os_mutex *dummymtx,
    const os_condAttr *condAttr)
{
    os_result result;

    assert (cond != NULL);
    assert (condAttr != NULL);

    cond->scope = condAttr->scopeAttr;
    cond->state = 0;

    /* In single process mode only "private" variables are required */
    if (os_serviceGetSingleProcess ()) {
        cond->scope = OS_SCOPE_PRIVATE;
    }

    if (cond->scope == OS_SCOPE_SHARED) {
        result = getSem(cond);
        if (result != os_resultSuccess) {
            return result;
        }
    } else {
        cond->qId = (long)CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
        if ((HANDLE)cond->qId == NULL) {
            return os_resultFail;
        }
    }
    return os_resultSuccess;
}

/** \brief Destroy the condition variable
 *
 * \b os_condDestroy calls \b pthread_cond_destroy to destroy the
 * posix condition variable.
 */
os_result
os_condDestroy(
    os_cond *cond)
{
    os_result result;

    assert (cond != NULL);

    if (cond->scope == OS_SCOPE_SHARED) {
        result = returnSem(cond);
        return result;
    } else {
        CloseHandle((HANDLE)cond->qId);
        result = os_resultSuccess;
    }
    return result;
}

/** \brief Wait for the condition
 *
 * \b os_condWait calls \b pthread_cond_wait to wait
 * for the condition.
 */
os_result
os_condWait (
    os_cond *cond,
    os_mutex *mutex)
{
    return condTimedWait(cond, mutex, INFINITE);
}

/** \brief Wait for the condition but return when the specified
 *         time has expired before the condition is triggered
 *
 * \b os_condTimedWait calls \b pthread_cond_timedwait to
 * wait for the condition with a timeout.
 *
 * \b os_condTimedWait provides an relative time to wait while
 * \b pthread_cond_timedwait expects an absolute time to wakeup.
 * The absolute time is calculated from the current time + the
 * provided relative time.
 *
 * \b os_condTimedWait will repeat \b pthread_cond_timedwait in case of an
 * interrupted system call. Because the time which is passed onto
 * \b pthread_cond_timedwait is absolute, no remaining time must be
 * calculated.
 */
os_result
os_condTimedWait (
    os_cond *cond,
    os_mutex *mutex,
    const os_time *time)
{
    DWORD wait_time;

    assert (cond != NULL);
    assert (mutex != NULL);
    assert (time != NULL);

    wait_time = (DWORD)time->tv_sec * 1000 + time->tv_nsec / 1000000;

    return condTimedWait(cond, mutex, wait_time);
}

/** \brief Signal the condition and wakeup one thread waiting
 *         for the condition
 *
 * \b os_condSignal calls \b pthread_cond_signal to signal
 * the condition.
 */
os_result
os_condSignal (
    os_cond *cond)
{
    return condSignal(cond, SIGNAL_BIT_MASK);
}

/** \brief Signal the condition and wakeup all thread waiting
 *         for the condition
 *
 * \b os_condBroadcast calls \b pthread_cond_broadcast to broadcast
 * the condition.
 */
os_result
os_condBroadcast (
    os_cond *cond)
{
    return condSignal(cond, BROADCAST_BIT_MASK);
}
