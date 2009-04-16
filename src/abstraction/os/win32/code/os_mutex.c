/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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
#include <os_mutex.h>
#include <code/os__debug.h>
#include <code/os__service.h>

#include <stdio.h>
#include <assert.h>

#include <../common/code/os_mutex_attr.c>

/*** Public functions *****/

/** \brief Initialize the mutex taking the mutex attributes
 *         into account
 *
 * \b os_mutexInit calls \b pthread_mutex_init to intialize the
 * posix \b mutex
 *
 * In case the scope attribute is \b OS_SCOPE_SHARED, the posix
 * mutex "pshared" attribute is set to \b PTHREAD_PROCESS_SHARED
 * otherwise it is set to \b PTHREAD_PROCESS_PRIVATE.
 */
os_result
os_mutexInit (
    os_mutex *mutex, 
    const os_mutexAttr *mutexAttr)
{
    const char *pipename;
    struct os_servicemsg request;
    struct os_servicemsg reply;
    BOOL result;
    DWORD nRead;
    os_result osr;
    
    assert(mutex != NULL);

    pipename = os_servicePipeName();
    mutex->scope = mutexAttr->scopeAttr;
    mutex->lockCount = 0;
    if (mutexAttr->scopeAttr == OS_SCOPE_SHARED) {
        request.kind = OS_SRVMSG_CREATE_EVENT;
        reply.result = os_resultFail;
        reply.kind = OS_SRVMSG_UNDEFINED;
        result = CallNamedPipe(
                     TEXT(pipename),
                     &request, sizeof(request), 
                     &reply, sizeof(reply), 
                     &nRead, 
                     NMPWAIT_WAIT_FOREVER);
        if (!result || (nRead != sizeof(reply))) {
            OS_DEBUG_4("Failure %d %d %d %d\n", result, GetLastError(), nRead, reply.kind);
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
    const char *pipename;
    struct os_servicemsg request;
    struct os_servicemsg reply;
    BOOL result;
    DWORD nRead;
    os_result osr;
    
    assert(mutex != NULL);
    assert(mutex->lockCount == 0);
    
    pipename = os_servicePipeName();
    if (mutex->scope == OS_SCOPE_SHARED) {
        request.kind = OS_SRVMSG_DESTROY_EVENT;
        request._u.id = mutex->id;
        reply.result = os_resultFail;
        reply.kind = OS_SRVMSG_UNDEFINED;
        result = CallNamedPipe(
                     TEXT(pipename),
                     &request, sizeof(request), 
                     &reply, sizeof(reply), 
                     &nRead, 
                     NMPWAIT_WAIT_FOREVER);
        if (!result  || (nRead != sizeof(reply))){
            OS_DEBUG_4("Failure %d %d %d %d\n", result, GetLastError(), nRead, reply.kind);
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
            _snprintf(name, sizeof(name), "%s%d",
                      OS_SERVICE_EVENT_NAME_PREFIX, mutex->id);
            mutexHandle = OpenEvent(EVENT_ALL_ACCESS, FALSE, name);
            if (mutexHandle == NULL) {
                OS_DEBUG_2("os_mutexLock: failed to open mutex %s %d", name, GetLastError());
                assert(FALSE);
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
    if (lc > 1) {
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
            _snprintf(name, sizeof(name), "%s%d",
                      OS_SERVICE_EVENT_NAME_PREFIX, mutex->id);
            mutexHandle = OpenEvent(EVENT_ALL_ACCESS, FALSE, name);
            if (mutexHandle == NULL) {
                OS_DEBUG_2("os_mutexLock: failed to open mutex %s %d", name, GetLastError());
                assert(FALSE);
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
