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
/** \file os/win32/code/os_mutex.c
 *  \brief WIN32 mutual exclusion semaphore
 *
 * Implements mutual exclusion semaphore for WIN32
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include "os_stdlib.h"
#include "os_mutex.h"
#include "os_report.h"
#include "os_init.h"
#include "os__debug.h"
#include "os__service.h"
#include "os__sharedmem.h"

#include <stdio.h>
#include <assert.h>

#include "../common/code/os_mutex_attr.c"


/*** Public functions *****/

/** \brief Sets the priority inheritance mode for mutexes
 *   that are created after this call.
 *
 * Not (yet) supported on this platform
 */
os_result
os_mutexSetPriorityInheritanceMode(
    os_boolean enabled)
{
    /* Priority Inheritance is not supported on this platform (yet) */
    return os_resultSuccess;
}

/** \brief Initialize the mutex taking the mutex attributes
 *         into account
 *
 * \b os_mutexInit calls \b pthread_mutex_init to intialize the
 * posix \b mutex
 *
 * In case the scope attribute is \b OS_SCOPE_SHARED, the posix
 * mutex "pshared" attribute is set to \b PTHREAD_PROCESS_SHARED
 * otherwise it is set to \b PTHREAD_PROCESS_PRIVATE.
 *
 * When in single process mode, a request for a SHARED variable will
 * implictly create a PRIVATE equivalent.  This is an optimisation
 * because there is no need for "shared" multi process variables in
 * single process mode.
 */
os_result
os_mutexInit (
    os_mutex *mutex,
    const os_mutexAttr *mutexAttr)
{
    char *pipename;
    struct os_servicemsg request;
    struct os_servicemsg reply;
    BOOL result;
    DWORD nRead;
    DWORD lastError;
    os_result osr;

    assert(mutex != NULL);

    mutex->scope = mutexAttr->scopeAttr;
    mutex->lockCount = 0;


    /* In single process mode only "private" variables are required */
    if (os_serviceGetSingleProcess ()) {
        mutex->scope = OS_SCOPE_PRIVATE;
    }

    if (mutex->scope == OS_SCOPE_SHARED) {
        pipename = os_createPipeNameFromMutex(mutex); /*os_servicePipeName();*/
        if (pipename == NULL) {
            pipename = os_servicePipeName();
            OS_DEBUG_1("os_mutexInit", "Failed to get a domain name from mutex, using default: %s", pipename);
        }
        request.kind = OS_SRVMSG_CREATE_EVENT;
        reply.result = os_resultFail;
        reply.kind = OS_SRVMSG_UNDEFINED;

        do {
            result = CallNamedPipe(
                TEXT(pipename),
                &request, sizeof(request),
                &reply, sizeof(reply),
                &nRead,
                NMPWAIT_USE_DEFAULT_WAIT);

            if(!result){
                lastError = GetLastError();
            } else {
                lastError = ERROR_SUCCESS;
            }
        } while((!result) && (lastError == ERROR_PIPE_BUSY));


        if (!result || (nRead != sizeof(reply))) {
            OS_DEBUG_4("os_mutexInit", "Failure %d %d %d %d\n", result, GetLastError(), nRead, reply.kind);
            osr = os_resultFail;
        } else {
            if ((reply.result == os_resultSuccess) &&
                (reply.kind == OS_SRVMSG_CREATE_EVENT)) {
                mutex->id = reply._u.id;
                osr = os_resultSuccess;
            } else {
                osr = os_resultFail;
            }
        }
    } else { /* private so don't get one from pool */
        mutex->id = (long)CreateEvent(NULL, FALSE, FALSE, NULL);
        osr = os_resultSuccess;
    }

    return osr;
}

/** \brief Destroy the mutex
 *
 * \b os_mutexDestroy calls \b pthread_mutex_destroy to destroy the
 * posix \b mutex.
 */
os_result
os_mutexDestroy (
    os_mutex *mutex)
{
    char *pipename;
    struct os_servicemsg request;
    struct os_servicemsg reply;
    BOOL result;
    DWORD nRead;
    DWORD lastError;
    os_result osr;

    assert(mutex != NULL);
    /* assert(mutex->lockCount == 0); */


    if (mutex->scope == OS_SCOPE_SHARED) {
        pipename = os_createPipeNameFromMutex(mutex); /*os_servicePipeName();*/
        if (pipename == NULL) {
            pipename = os_servicePipeName();
            OS_DEBUG_1("os_mutexInit", "Failed to get a domain name from mutex, using default: %s", pipename);
        }
        request.kind = OS_SRVMSG_DESTROY_EVENT;
        request._u.id = mutex->id;
        reply.result = os_resultFail;
        reply.kind = OS_SRVMSG_UNDEFINED;

        do{
           result = CallNamedPipe(
               TEXT(pipename),
               &request, sizeof(request),
               &reply, sizeof(reply),
               &nRead,
               NMPWAIT_USE_DEFAULT_WAIT);

           if(!result){
              lastError = GetLastError();
           } else {
              lastError = ERROR_SUCCESS;
           }
        } while((!result) && (lastError == ERROR_PIPE_BUSY));

        if (!result  || (nRead != sizeof(reply))) {
           OS_DEBUG_4("os_mutexDestroy", "Failure %d %d %d %d\n", result, GetLastError(), nRead, reply.kind);
           osr = os_resultFail;
        } else {
           if ((reply.result == os_resultSuccess) &&
               (reply.kind == OS_SRVMSG_DESTROY_EVENT)) {
              osr = os_resultSuccess;
           } else {
              osr = os_resultFail;
           }
        }
    } else { /* private so don't return to pool */
        CloseHandle((HANDLE)mutex->id);
        osr = os_resultSuccess;
    }
    return osr;
}

/** \brief Acquire the mutex
 *
 * \b os_mutexLock calls \b pthread_mutex_lock to acquire
 * the posix \b mutex.
 */
os_result
os_mutexLock(
    os_mutex *mutex)
{
    HANDLE mutexHandle;
    char name[OS_SERVICE_ENTITY_NAME_MAX];
    DWORD result;
    os_result r;
    long lc;

    assert(mutex != NULL);

    r = os_resultSuccess;
    lc = InterlockedIncrement(&mutex->lockCount);
    if (lc > 1) {
        if (mutex->scope == OS_SCOPE_SHARED) {
            if (_snprintf(name, sizeof(name), "%s%s%d%s",
                (os_sharedMemIsGlobal() ? OS_SERVICE_GLOBAL_NAME_PREFIX : ""),
                OS_SERVICE_EVENT_NAME_PREFIX,
                mutex->id,
                os_getShmDomainKeyForPointer(mutex)) <= 0) {
                OS_REPORT_1(OS_ERROR, "mutexLock", 0,
                    "Event name exceeds maximum allowed length (%d)", OS_SERVICE_ENTITY_NAME_MAX);
                return os_resultFail;
            }

            mutexHandle = OpenEvent(EVENT_ALL_ACCESS, FALSE, name);
            if (mutexHandle == NULL) {
                OS_DEBUG_2("os_mutexLock", "Failed to open event with name %s (Error: %d)", name, GetLastError());
                assert(mutexHandle != NULL);
                return os_resultFail;
            }
        } else {
            mutexHandle = (HANDLE)mutex->id;
        }
        result = WaitForSingleObject(mutexHandle, INFINITE);
        assert(result == WAIT_OBJECT_0);
        if (mutex->scope == OS_SCOPE_SHARED) {
            CloseHandle(mutexHandle);
        }
        if (result != WAIT_OBJECT_0) {
            r = os_resultFail;
        }
    }

    return r;
}

/** \brief Try to acquire the mutex, immediately return if the mutex
 *         is already acquired by another thread
 *
 * \b os_mutexTryLock calls \b pthread_mutex_trylock to acquire
 * the posix \b mutex.
 */
os_result
os_mutexTryLock (
    os_mutex *mutex)
{
    os_result r;
    long lc;

    assert(mutex != NULL);

    r = os_resultSuccess;
    lc = InterlockedCompareExchange(&mutex->lockCount, 1, 0);
    if (lc > 0) {
        r = os_resultBusy;
    }
    return r;

}

/** \brief Release the acquired mutex
 *
 * \b os_mutexUnlock calls \b pthread_mutex_unlock to release
 * the posix \b mutex.
 */
os_result
os_mutexUnlock (
    os_mutex *mutex)
{
    HANDLE mutexHandle;
    long lc;
    char name[OS_SERVICE_ENTITY_NAME_MAX];
    BOOL r;
    os_result result;

    assert(mutex != NULL);

    result = os_resultSuccess;
    lc = InterlockedDecrement(&mutex->lockCount);
    if (lc > 0) {
        if (mutex->scope == OS_SCOPE_SHARED) {
            if (_snprintf(name, sizeof(name), "%s%s%d%s",
                (os_sharedMemIsGlobal() ? OS_SERVICE_GLOBAL_NAME_PREFIX : ""),
                OS_SERVICE_EVENT_NAME_PREFIX,
                mutex->id,
                os_getShmDomainKeyForPointer(mutex)) <= 0) {
                OS_REPORT_1(OS_ERROR, "mutexUnlock", 0,
                    "Event name exceeds maximum allowed length (%d)", OS_SERVICE_ENTITY_NAME_MAX);
                return os_resultFail;
            }

            mutexHandle = OpenEvent(EVENT_ALL_ACCESS, FALSE, name);
            if (mutexHandle == NULL) {
                OS_DEBUG_2("os_mutexUnlock", "Failed to open event with name %s (Error: %d)", name, GetLastError());
                assert(mutexHandle != NULL);
                return os_resultFail;
            }
        } else {
            mutexHandle = (HANDLE)mutex->id;
        }
        r = SetEvent(mutexHandle);
        assert(r == TRUE);
        if (mutex->scope == OS_SCOPE_SHARED) {
            CloseHandle(mutexHandle);
        }
        if (r == FALSE) {
            result = os_resultFail;
        }
    }

    return result;
}
