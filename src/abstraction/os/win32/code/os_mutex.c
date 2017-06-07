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
/** \file os/win32/code/os_mutex.c
 *  \brief WIN32 mutual exclusion semaphore
 *
 * Implements mutual exclusion semaphore for WIN32
 */
#include "os_mutex.h"
#include "os__sync.h"
#include "os_init.h"

#include "../common/code/os_mutex_attr.c"

#ifndef NDEBUG
#define OS__MUTEX_SIG 0x8C11EE5E /* Ate cheese */
#endif

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
    OS_UNUSED_ARG(enabled);
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
_Check_return_
os_result
os_mutexInit(
    _Out_ _When_(return != os_resultSuccess, _Post_invalid_) os_mutex *mutex,
    _In_opt_ const os_mutexAttr *mutexAttr)
{
    os_result r = os_resultFail;

    assert(mutex);

    if(!mutexAttr){
        os_mutexAttr defAttr;

        os_mutexAttrInit(&defAttr);
        mutex->scope = defAttr.scopeAttr;
    } else {
        mutex->scope = mutexAttr->scopeAttr;
    }

    switch(mutex->scope) {
        case OS_SCOPE_PRIVATE:
            r = os_syncLockInitialize(&mutex->lock.priv);
            break;
        case OS_SCOPE_SHARED:
            r = os_syncSharedLockInitialize(&mutex->lock.sha);
            break;
    }

#ifndef NDEBUG
    mutex->signature = OS__MUTEX_SIG;
#endif

    return r;
}

/** \brief Destroy the mutex
 *
 * \b os_mutexDestroy calls \b pthread_mutex_destroy to destroy the
 * posix \b mutex.
 */
void
os_mutexDestroy(
    _Inout_ _Post_invalid_ os_mutex *mutex)
{
    assert(mutex);

#ifndef NDEBUG
    assert(mutex->signature == OS__MUTEX_SIG);
    mutex->signature = 0;
#endif

    switch(mutex->scope) {
        case OS_SCOPE_PRIVATE:
            os_syncLockDelete(&mutex->lock.priv);
            break;
        case OS_SCOPE_SHARED:
            os_syncSharedLockDelete(&mutex->lock.sha);
            break;
    }
}

/** \brief Acquire the mutex
 *
 * \b os_mutexLock calls \b pthread_mutex_lock to acquire
 * the posix \b mutex.
 */
_Check_return_
_When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(&mutex->lock))
os_result
os_mutexLock_s(
    _Inout_ os_mutex *mutex)
{
    assert(mutex);

#ifndef NDEBUG
    assert(mutex->signature == OS__MUTEX_SIG);
#endif

    switch(mutex->scope) {
        case OS_SCOPE_PRIVATE:
            /* Native locks can't fail on Windows, so even the os_mutexLockError
             * will always return os_resultSuccess. */
            os_syncLockAcquireExclusive(&mutex->lock.priv);
            return os_resultSuccess;
        case OS_SCOPE_SHARED:
            return os_syncSharedLockAcquireExclusive(&mutex->lock.sha);
    }

    /* Silence the compiler about the code-path for non-existing enum-labels. */
    return os_resultFail;
}

_Acquires_nonreentrant_lock_(&mutex->lock)
void
os_mutexLock(
    _Inout_ os_mutex *mutex)
{
    if (os_mutexLock_s (mutex) != os_resultSuccess) {
        abort ();
    }
}

/** \brief Try to acquire the mutex, immediately return if the mutex
 *         is already acquired by another thread
 *
 * \b os_mutexTryLock calls \b pthread_mutex_trylock to acquire
 * the posix \b mutex.
 */
_Check_return_
_When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(&mutex->lock))
os_result
os_mutexTryLock (
    _Inout_ os_mutex *mutex)
{
    assert(mutex);

#ifndef NDEBUG
    assert(mutex->signature == OS__MUTEX_SIG);
#endif

    switch(mutex->scope) {
        case OS_SCOPE_PRIVATE:
            return os_syncLockTryAcquireExclusive(&mutex->lock.priv);
        case OS_SCOPE_SHARED:
            return os_syncSharedLockTryAcquireExclusive(&mutex->lock.sha);
    }

    /* Silence the compiler about the code-path for non-existing enum-labels. */
    return os_resultBusy;
}

/** \brief Release the acquired mutex
 *
 * \b os_mutexUnlock calls \b pthread_mutex_unlock to release
 * the posix \b mutex.
 */
_Releases_nonreentrant_lock_(&mutex->lock)
void
os_mutexUnlock (
    _Inout_ os_mutex *mutex)
{
    assert(mutex);

#ifndef NDEBUG
    assert(mutex->signature == OS__MUTEX_SIG);
#endif

    switch(mutex->scope) {
        case OS_SCOPE_PRIVATE:
            os_syncLockReleaseExclusive(&mutex->lock.priv);
            break;
        case OS_SCOPE_SHARED:
            os_syncSharedLockReleaseExclusive(&mutex->lock.sha);
            break;
    }
}
