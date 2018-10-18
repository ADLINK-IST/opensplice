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
#pragma once

#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * This file provides a native locks interface for Windows. The os__sync.h interface layer is
 * added to consolidate the implementation for the different mutex- and condition variable
 * implementations. It also allows for dynamic selection of locks at runtime in case at compile time
 * an older Windows version (pre 7) is targeted.
 *
 * Build-flags:
 * OS__SYNC_NOSHARED:
 *                    The sync API by default implements process shared (shared memory) mutexes and
 *                    conditions. If these are not needed one can specify -DOS__SYNC_NOSHARED. The
 *                    initialization functions for shared mutexes and conditions are then replaced
 *                    by macro's representing failure. All the other functions are mapped to abort().
 * OS__SYNC_SHARED_SPINLOCKCOUNT (int):
 *                    If defined, the value will be used for a spinlock optimization for process
 *                    shared mutexes. Proper values need to be determined. High values (4000) for
 *                    shorter critical sections seemed to work OK. Longer critical sections should
 *                    use lower values.
 *                    NOTE: single-core execution is not (yet) detected, so it should always be set
 *                    to 0 (default) on such targets.
 * OS__SYNC_PRIVATE_SPINLOCKCOUNT (int):
 *                    If defined, the value will be used for a spinlock optimization for process
 *                    private mutexes. Proper values need to be determined. High values (4000) for
 *                    shorter critical sections seemed to work OK. Longer critical sections should
 *                    use lower values.
 *                    NOTE: single-core execution is not (yet) detected, so it should always be set
 *                    to 0 (default) on such targets.
 */
#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT

#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

extern int os_syncModuleInitialized;

/**
 * Must be called exactly once before any of the other sync-API calls are invoked.
 */
_Pre_satisfies_( !os_syncModuleInitialized )
_Post_satisfies_( os_syncModuleInitialized )
OS_API
void
os_syncModuleInit(void);

/**
 * Cleans up resources that may be allocated by the sync-API. After this call has been done the API
 * may not be used anymore until os_syncModuleInit() is re-invoked.
 */
_Pre_satisfies_( os_syncModuleInitialized )
_Post_satisfies_( !os_syncModuleInitialized )
OS_API
void
os_syncModuleDeinit(void);

/**
 * Initializes the native lock. Lock must point to natively aligned memory capable of holding at
 * least an os_mutexPrivate struct. Lock may only be used if os_resultSuccess was returned.
 *
 * The lock must be assumed to be process private.
 *
 * Initializing an already successfully initialized lock may result in undefined behaviour.
 *
 * @param lock the lock to initialize
 * @return os_resultSuccess iff the lock was successfully initialized
 */
_Pre_satisfies_( os_syncModuleInitialized )
_Check_return_
OS_API
os_result
os_syncLockInitialize (
    _Out_ _When_(return != os_resultSuccess, _Post_invalid_) void* lock);

/**
 * Acquires lock exclusively.
 *
 * @param lock the lock to obtain
 */
_Pre_satisfies_( os_syncModuleInitialized )
_Acquires_nonreentrant_lock_(lock)
OS_API
void
os_syncLockAcquireExclusive (
    _Inout_ void* lock);

/**
 * Tries to acquire lock exclusively.
 *
 * @param lock the lock to try to obtain
 * @return os_resultSuccess if the lock was obtained, os_resultBusy otherwise
 */
_Pre_satisfies_( os_syncModuleInitialized )
_Check_return_
_When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(lock))
OS_API
os_result
os_syncLockTryAcquireExclusive (
    _Inout_ void* lock);

/**
 * Releases a previously acquired exclusive lock.
 *
 * @param lock the lock to release
 */
_Pre_satisfies_( os_syncModuleInitialized )
_Releases_nonreentrant_lock_(lock)
OS_API
void
os_syncLockReleaseExclusive (
    _Inout_ void* lock);

/**
 * Deletes the lock. The lock must be successfully initialized using os_syncLockInitialize(...).
 * After this call, lock may not be used anymore. Invoking this call on a lock that is still in use
 * may result in undefined behaviour.
 *
 * @param lock the lock to delete
 */
_Pre_satisfies_( os_syncModuleInitialized )
OS_API
void
os_syncLockDelete (
    _Inout_ _Post_invalid_ void* lock);

/**
 * Initializes the native condition variable. Cond must point to natively aligned memory capable of
 * holding at least a os_condPrivate struct.
 *
 * The condition variable must be assumed to be process private.
 *
 * Initializing an already successfully initialized condition may result in undefined behaviour.
 *
 * @param cond the condition variable to initialize
 */
_Pre_satisfies_( os_syncModuleInitialized )
OS_API
void
os_syncCondInitialize(
    _Out_ void *cond);

/**
 * Waits for at most the time specified for the condition to trigger. If the condition triggers,
 * os_resultSuccess is returned. Otherwise os_resultTimeout is returned.
 *
 * If dwMilliseconds is 0, the state of the condition will be checked and if it is not triggered, the
 * call will return immediately with os_resultTimeout. If dwMilliseconds is INFINITE, the call will
 * never return os_resultTimeout.
 *
 * Condition variables are subject to spurious wakeups and stolen wakeups (another thread manages to
 * run before the woken thread). Therefore, you should recheck a predicate after the sleep operation
 * returns.
 *
 * @param cond the condition variable the wait on
 * @param lock the lock that is protecting the condition variable.
 * @param dwMilliseconds the number of milliseconds to wait for the condition to trigger
 *
 * @pre lock is held by the calling thread
 * @post lock is held by the calling thread
 *
 * @return os_resultSuccess if the condition triggered
 * @return os_resultTimeout if timeout expired before the condition triggered
 */
_Pre_satisfies_( os_syncModuleInitialized )
_Requires_exclusive_lock_held_(lock)
_When_(dwMilliseconds == 0xFFFFFFFF, _Ret_range_(os_resultSuccess, os_resultSuccess))
OS_API
os_result
os_syncCondSleep(
    _Inout_ void *cond,
    _Inout_ void *lock,
    _In_ DWORD dwMilliseconds);

/**
 * Wakes one thread waiting on the condition.
 *
 * @param cond the condition to wake a thread on
 */
_Pre_satisfies_( os_syncModuleInitialized )
OS_API
void
os_syncCondWake(
    _Inout_ void *cond);

/**
 * Wakes all waiting threads that are waiting on the condition.
 *
 * @param cond the condition to wake all waiters on
 */
_Pre_satisfies_( os_syncModuleInitialized )
OS_API
void
os_syncCondWakeAll(
    _Inout_ void *cond);

/**
 * Deletes the condition variable. The condition variable must be successfully initialized using
 * os_syncCondInitialize(...). After this call, cond may not be used anymore. Invoking this call
 * on a condition variable that is still in use may result in undefined behaviour.
 *
 * @param cond the condition variable to delete
 */
_Pre_satisfies_( os_syncModuleInitialized )
OS_API
void
os_syncCondDelete(
    _Inout_ _Post_invalid_ void *cond);

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //
#if !OS__SYNC_NOSHARED
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //

/**
 * Initializes the shared native lock. Lock must point to natively aligned memory capable of holding
 * at least an os_syncLockShared struct. Lock may only be used if os_resultSuccess was returned.
 *
 * The lock may only be initialized once across all processes. Initializing an already successfully
 * initialized lock (even when done from another process) may result in undefined behaviour.
 *
 * For locks to be shared across processes, they must be mapped in a the same address within the
 * different processes.
 *
 * @param lock the lock to initialize
 * @return os_resultSuccess iff the lock was successfully initialized
 * @return os_resultFail if initialization failed
 */
_Pre_satisfies_( os_syncModuleInitialized )
_Check_return_
OS_API
os_result
os_syncSharedLockInitialize (
    _Out_ _When_(return != os_resultSuccess, _Post_invalid_) os__syncLockShared *lock);

/**
 * Acquires the shared lock exclusively.
 *
 * @param lock the lock to obtain
 * @return os_resultSuccess if the lock was obtained
 * @return os_resultFail fatal error, re-trying may deadlock or cause undefined behaviour. This can
 *                       occur if for example a process was killed while holding the lock.
 */
_Pre_satisfies_( os_syncModuleInitialized )
_When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(lock))
OS_API
os_result
os_syncSharedLockAcquireExclusive (
    _Inout_ os__syncLockShared *lock);

/**
 * Tries to acquire the shared lock exclusively.
 *
 * @param lock the lock to try to obtain
 * @return os_resultSuccess if the lock was obtained, os_resultBusy otherwise
 */
_Pre_satisfies_( os_syncModuleInitialized )
_Check_return_
_When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(lock))
OS_API
os_result
os_syncSharedLockTryAcquireExclusive (
   _Inout_  os__syncLockShared *lock);

/**
 * Releases a previously acquired exclusive lock.
 *
 * @param lock the lock to release
 */
_Pre_satisfies_( os_syncModuleInitialized )
_Releases_nonreentrant_lock_(lock)
OS_API
void
os_syncSharedLockReleaseExclusive (
    _Inout_ os__syncLockShared *lock);

/**
 * Deletes the lock. The lock must be successfully initialized using os_syncSharedLockInitialize(...).
 * After this call, lock may not be used anymore. Invoking this call on a lock that is still in use
 * may result in undefined behaviour.
 *
 * @param lock the lock to delete
 */
_Pre_satisfies_( os_syncModuleInitialized )
OS_API
void
os_syncSharedLockDelete (
    _In_ _Post_invalid_ os__syncLockShared *lock);

/**
 * Initializes the shared condition variable. Cond must point to natively aligned memory capable of
 * holding at least a os__syncCondShared struct. Cond may only be used if os_resultSuccess was returned.
 *
 * The condition variable may only be initialized once across all processes. Initializing an already
 * successfully initialized condition variable (even when done from another process) may result in
 * undefined behaviour.
 *
 * For condition variables to be shared across processes, they must be mapped in a the same address
 * within the different processes.
 *
 * @param cond the condition variable to initialize
 * @return os_resultSuccess iff the condition variable was successfully initialized
 * @return os_resultFail if initialization failed
 */
_Pre_satisfies_( os_syncModuleInitialized )
_Check_return_
OS_API
os_result
os_syncSharedCondInitialize(
    _Out_ _When_(return != os_resultSuccess, _Post_invalid_) os__syncCondShared *cond);

/**
 * Waits for at most the time specified for the condition to trigger. If the condition triggers,
 * os_resultSuccess is returned. Otherwise os_resultTimeout is returned.
 *
 * If dwMilliseconds is 0, the state of the condition will be checked and if it is not triggered, the
 * call will return immediately with os_resultTimeout. If dwMilliseconds is INFINITE, the call will
 * never return os_resultTimeout.
 *
 * Condition variables are subject to spurious wakeups and stolen wakeups (another thread manages to
 * run before the woken thread). Therefore, you should recheck a predicate after the sleep operation
 * returns.
 *
 * @param cond the condition variable the wait on
 * @param lock the lock that is protecting the condition variable.
 * @param dwMilliseconds the number of milliseconds to wait for the condition to trigger
 *
 * @pre lock is held by the calling thread
 * @post lock is held by the calling thread
 *
 * @return os_resultSuccess if the condition triggered
 * @return os_resultTimeout if timeout expired before the condition triggered
 * @return os_resultFail if fatal error occurred
 */
_Pre_satisfies_( os_syncModuleInitialized )
_Requires_exclusive_lock_held_(lock)
_When_(dwMilliseconds == 0xFFFFFFFF, _Ret_range_(os_resultSuccess, os_resultSuccess))
OS_API
os_result
os_syncSharedCondSleep(
    _Inout_ os__syncCondShared *cond,
    _Inout_ os__syncLockShared *lock,
    _In_ DWORD dwMilliseconds);

/**
 * Wakes one thread waiting on the condition.
 *
 * @param cond the condition to wake a thread on
 */
_Pre_satisfies_( os_syncModuleInitialized )
OS_API
void
os_syncSharedCondWake(
    _Inout_ os__syncCondShared *cond);

/**
 * Wakes all waiting threads that are waiting on the condition.
 *
 * @param cond the condition to wake all waiters on
 */
_Pre_satisfies_( os_syncModuleInitialized )
OS_API
void
os_syncSharedCondWakeAll(
    _Inout_ os__syncCondShared *cond);

/**
 * Deletes the condition variable. The condition variable must be successfully initialized using
 * os_syncSharedCondInitialize(...). After this call, cond may not be used anymore. Invoking this call
 * on a condition variable that is still in use may result in undefined behaviour.
 *
 * @param cond the condition variable to delete
 */
_Pre_satisfies_( os_syncModuleInitialized )
OS_API
void
os_syncSharedCondDelete(
    _In_ _Post_invalid_ os__syncCondShared *cond);

#else

/** Not supported. Build without OS__SYNC_NOSHARED, or with OS__SYNC_NOSHARED=0 flag to get shared lock support */
#define os_syncSharedLockInitialize(m)          (os_resultFail)
/** Not supported; will abort(). Build without OS__SYNC_NOSHARED, or with OS__SYNC_NOSHARED=0 flag to get shared lock support */
#define os_syncSharedLockAcquireExclusive(m)    (abort(), os_resultFail)
/** Not supported; will abort(). Build without OS__SYNC_NOSHARED, or with OS__SYNC_NOSHARED=0 flag to get shared lock support */
#define os_syncSharedLockTryAcquireExclusive(m) (abort(), os_resultFail)
/** Not supported; will abort(). Build without OS__SYNC_NOSHARED, or with OS__SYNC_NOSHARED=0 flag to get shared lock support */
#define os_syncSharedLockReleaseExclusive(m)    abort()
/** Not supported; will abort(). Build without OS__SYNC_NOSHARED, or with OS__SYNC_NOSHARED=0 flag to get shared lock support */
#define os_syncSharedLockDelete(m)              abort()

/** Not supported. Build without OS__SYNC_NOSHARED, or with OS__SYNC_NOSHARED=0 flag to get shared condition support */
#define os_syncSharedCondInitialize(c)          (os_resultFail)
/** Not supported; will abort(). Build without OS__SYNC_NOSHARED, or with OS__SYNC_NOSHARED=0 flag to get shared condition support */
#define os_syncSharedCondSleep(c, m, d)         (abort(), os_resultSuccess)
/** Not supported; will abort(). Build without OS__SYNC_NOSHARED, or with OS__SYNC_NOSHARED=0 flag to get shared condition support */
#define os_syncSharedCondWake(c)                abort()
/** Not supported; will abort(). Build without OS__SYNC_NOSHARED, or with OS__SYNC_NOSHARED=0 flag to get shared condition support */
#define os_syncSharedCondWakeAll(c)             abort()
/** Not supported; will abort(). Build without OS__SYNC_NOSHARED, or with OS__SYNC_NOSHARED=0 flag to get shared condition support */
#define os_syncSharedCondWakeAll(c)             abort()
/** Not supported; will abort(). Build without OS__SYNC_NOSHARED, or with OS__SYNC_NOSHARED=0 flag to get shared condition support */
#define os_syncSharedCondDelete(c)              abort()

#endif /* OS__SYNC_NOSHARED */
#undef OS_API

#if defined (__cplusplus)
}
#endif
