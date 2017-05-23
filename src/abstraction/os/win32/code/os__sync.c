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

/**
 * This file implements the native locks interface for Windows. This file provides:
 * - Dynamic lock-type detection (SRWLOCK, CRITICAL_SECTION) if at build time Windows XP or Vista
 *   are targeted. In that case at runtime the fastest process private lock available is selected,
 *   in order to benefit from performance improvements on Windows 7 and on.
 * - Dynamic condition variable detection (CONDITION_VARIABLE) if at build time Windows XP or Vista
 *   are targeted. In that case at runtime support for CONDITION_VARIABLE's is detected,
 *   in order to benefit from performance improvements on Windows 7 and on.
 * - Windows kernel-events based mutex- and condition variable implementation for Windows XP and/or
 *   Vista which both don't implement the full interface natively.
 * - Wrapper layer (os_syncLock* and os_syncCond*) for invoking the dynamically selected locks and
 *   condition variables.
 * - Process shared scope mutex- and condition variable implementation (os_syncSharedLock* and
 *   os_syncSharedCond*) which is used on all Windows versions.
 * - Process private caching for Event-handles used for the shared mutex- and condition variables
 *   implementation. The caching uses CRITICAL_SECTION's (exclusive) or SRWLOCK's (shared) when
 *   available. This is detected at runtime for builds targeting older Windows (XP and Vista),
 *   otherwise SRWLOCK's are directly used (see os_privateLock*).
 */
#if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x0502
/* Minimum Windows Server 2003 SP1, Windows XP SP2 == _WIN32_WINNT_WS03 (0x0502) */
#error _WIN32_WINNT should be at least _WIN32_WINNT_WS03 (0x0502)
#endif
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Sddl.h>
#include <assert.h>

#include "os__sync.h"
#include "os_heap.h"

/* Workaround for issue with _Ret_maybenull_ on older Visual Studio's */
#if _MSC_VER < 1700
#undef _Ret_maybenull_
#define _Ret_maybenull_
#endif

/* TRUE iff it is safe to invoke functions */
int os_syncModuleInitialized = 0;

#ifndef OS__SYNC_SHARED_SPINLOCKCOUNT
# define OS__SYNC_SHARED_SPINLOCKCOUNT (0)
#endif
#ifndef OS__SYNC_PRIVATE_SPINLOCKCOUNT
# define OS__SYNC_PRIVATE_SPINLOCKCOUNT (0)
#endif

#define OS__SYNC_BROADCAST (1 << 31)

#ifndef NDEBUG
#define OS__SYNC_HT_SIG 0xDEFFED
#endif

/* The Interlocked* functions are from the Windows API. The Windows API normally selects the best
 * implementation (e.g., compiler intrinsic, etc.) when available. There is no official guarantee
 * that this shortcut (i.e., checking if macro is defined i.s.o. checking compiler version etc.)
 * works, but a study of the current Windows headers suggests this is valid for now at least. */
#if !defined(InterlockedIncrementNoFence)
# define InterlockedIncrementNoFence InterlockedIncrement
#endif

#if !defined(InterlockedOr)
static LONG InterlockedOr ( _Inout_ _Interlocked_operand_ LONG volatile *dest, _In_ LONG mask) {
    LONG i, j = *dest;
    do {
        i = j;
        j = InterlockedCompareExchange(dest, i | mask, i);
    } while (i != j);
    return j;
}
#endif

#if !defined(InterlockedAnd)
static LONG InterlockedAnd ( _Inout_ _Interlocked_operand_ LONG volatile *dest, _In_ LONG mask) {
    LONG i, j = *dest;
    do {
        i = j;
        j = InterlockedCompareExchange(dest, i & mask, i);
    } while (i != j);
    return j;
}
#endif

#if !defined(InterlockedCompareExchangeAcquire)
# define InterlockedCompareExchangeAcquire InterlockedCompareExchange
#endif

#if !defined(InterlockedIncrementAcquire)
# define InterlockedIncrementAcquire InterlockedIncrement
#endif

#if !defined(InterlockedDecrementRelease)
# define InterlockedDecrementRelease InterlockedDecrement
#endif


////////////////////////////////////////////////////////////////////////////////
// EVENT-BASED LOCK HELPERS
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns TRUE if lc was 1 (exclusively obtained mutex).
 * @param lc Pointer to atomically modified counter variable
 * @param spins Number of spins allowed before considering failed
 * @return TRUE if lc was 1, FALSE otherwise
 */
_Check_return_
static BOOLEAN
os__syncEventLockSpinAcquireExclusive (
    _Inout_ _Interlocked_operand_ LONG *lc,
    _In_ UINT32 spins)
{
    assert(lc);

    if(spins) {
        UINT32 spinCount = spins;

        do{ /* First try spinCount times to acquire lc */
            if((volatile LONG *)lc == 0) {
                if(InterlockedCompareExchangeAcquire(lc, 1, 0) == 0) {
                    return TRUE;
                }
            }
        } while(--spinCount > 0);
    }

    /* If lc is still >0, then we should just increase it by one and use
     * expensive callback. If lc == 1, we are done since we acquired the lock.*/
    if(InterlockedIncrementAcquire(lc) > 1){
        return FALSE;
    } else {
        return TRUE;
    }
}

static os_result
os__syncEventLockAcquireExclusive (
    _In_ HANDLE eventHandle)
{
    DWORD wr;

    assert(eventHandle);

    wr = WaitForSingleObject(eventHandle, INFINITE);
    switch(wr){
        case WAIT_OBJECT_0:
            /* Acquired lock. Optional error-checking (recursive locking)
             * could be performed here. */
            return os_resultSuccess;
        case WAIT_FAILED:
        case WAIT_TIMEOUT: /* We specified INFINITE, so WAIT_TIMEOUT is FATAL too */
        default:
            /* TODO: Report failure through Application Event Log (OSPL-7672)? */
            return os_resultFail;
    }

    return os_resultSuccess;
}


////////////////////////////////////////////////////////////////////////////////
// DYNAMIC LOCK-TYPE
////////////////////////////////////////////////////////////////////////////////
#if _WIN32_WINNT < 0x0601 /* _WIN32_WINNT_WIN7 */
/* When _WIN32_WINNT is set such that older OS's not supporting the SRW-locks
 * are excluded, the dynamic selection is omitted. Otherwise the best available
 * lock-type will be selected at runtime.
 *
 * Only from _WIN32_WINNT_WIN7 onwards is it possible to implement the try-lock
 * on SRWLocks, so Windows Vista will use the CRITICAL_SECTION implementation. */
#define OS__DYNAMICALLY_SELECT_LOCKTYPE
#endif /* _WIN32_WINNT < _WIN32_WINNT_WIN7 */

#ifdef OS__DYNAMICALLY_SELECT_LOCKTYPE
static BOOL os__nativeHaveSRWLockAndCondition;

/* Mutexes */
typedef void (WINAPI *os__nativeLockInitializeSRWLock_func) (_Out_ void*);
typedef void (WINAPI *os__nativeLockDelete_func) (_Inout_ void*);
typedef void (WINAPI *os__nativeLockAcquireExclusive_func) (_Inout_ void*);
typedef BOOLEAN (WINAPI *os__nativeLockTryAcquireExclusive_func) (_Inout_ void*);
typedef void (WINAPI *os__nativeLockReleaseExclusive_func) (_Inout_ void*);
typedef void (WINAPI *os__nativeLockAcquireShared_func) (_Inout_ void*);
typedef BOOLEAN (WINAPI *os__nativeLockTryAcquireShared_func) (_Inout_ void*);
typedef void (WINAPI *os__nativeLockReleaseShared_func) (_Inout_ void*);

static os__nativeLockInitializeSRWLock_func os__nativeLockInitializeSRWLock;
static os__nativeLockDelete_func os__nativeLockDelete;
static os__nativeLockAcquireExclusive_func os__nativeLockAcquireExclusive;
static os__nativeLockTryAcquireExclusive_func os__nativeLockTryAcquireExclusive;
static os__nativeLockReleaseExclusive_func os__nativeLockReleaseExclusive;
static os__nativeLockAcquireShared_func os__nativeLockAcquireShared;
static os__nativeLockTryAcquireShared_func os__nativeLockTryAcquireShared;
static os__nativeLockReleaseShared_func os__nativeLockReleaseShared;

/* Conditions */
typedef void (WINAPI *os__nativeCondInitializeConditionVariable_func) (_Out_ void*);
typedef void (WINAPI *os__nativeCondDeleteConditionVariable_func) (_Inout_ void*);
/* SleepConditionVariableSRW has one parameter more, so we need to wrap the invocation */
typedef BOOL (WINAPI *os__nativeCondSleepConditionVariableSRW_func) (_Inout_ void*, _Inout_ void*, _In_ DWORD, _In_ ULONG);
typedef BOOL (WINAPI *os__nativeCondSleepConditionVariable_func) (_Inout_ void*, _Inout_ void*, _In_ DWORD);
typedef void (WINAPI *os__nativeCondWakeConditionVariable_func) (_Inout_ void*);
typedef void (WINAPI *os__nativeCondWakeAllConditionVariable_func) (_Inout_ void*);

/* Wrapper functions for fallback implementation for Windows XP/Vista. */
static void WINAPI os_syncSemaphoreCondInitialize (_Inout_ void*);
static void WINAPI os_syncSemaphoreCondDelete (_Inout_ void*);
static BOOL WINAPI os_syncSemaphoreCondSleep (_Inout_ void*, _Inout_ void*, _In_ DWORD);
static void WINAPI os_syncSemaphoreCondWake (_Inout_ void*);
static void WINAPI os_syncSemaphoreCondWakeAll (_Inout_ void*);

static os__nativeCondInitializeConditionVariable_func os__nativeCondInitializeConditionVariable;
static os__nativeCondDeleteConditionVariable_func os__nativeCondDeleteConditionVariable;
static os__nativeCondSleepConditionVariableSRW_func os__nativeCondSleepConditionVariableSRW;
static os__nativeCondSleepConditionVariable_func os__nativeCondSleepConditionVariable;
static os__nativeCondWakeConditionVariable_func os__nativeCondWakeConditionVariable;
static os__nativeCondWakeAllConditionVariable_func os__nativeCondWakeAllConditionVariable;


////////////////////////////////////////////////////////////////////////////////
// PRIVATE LOCK IMPLEMENTATION
//
// The private lock implements the fastest process-private lock that is
// available on this platform. This private lock is used to implement the event-
// table logic.
////////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
#define OS__SYNC_PRIV_SIG (0x0501DAAD)
#endif

typedef struct os_privateLock_s {
#ifndef NDEBUG
    UINT32 signature;
#endif
    union {
        CRITICAL_SECTION cs;
#ifdef SRWLOCK_INIT
        SRWLOCK srw;
#else
        os__syncLockPrivateSRWLOCK srw;
#endif /* SRWLOCK_INIT */
    } lock;
} os_privateLock;


/**
 * Initializes the fastest process-private lock that is available on this
 * platform.
 */
_Check_return_
static BOOL
os__privateLockInitialize (
    _Out_ os_privateLock *lock)
{
    BOOL r;

    assert(lock);
    assert(os_syncModuleInitialized);

    if(os__nativeHaveSRWLockAndCondition){
        assert(os__nativeLockInitializeSRWLock);

        os__nativeLockInitializeSRWLock(&lock->lock);
        r = TRUE;
    } else {
        /* TODO: make spincount configurable, or define reasonable default.
         * Testing showed that 4000 is nice for short critical sections, longer
         * critical sections should use lower values... Start of with 400. */
        r = InitializeCriticalSectionAndSpinCount(&lock->lock.cs, 0x400);
    }

#ifndef NDEBUG
    lock->signature = r ? OS__SYNC_PRIV_SIG : 0;
#endif

    return r;
}

/**
 * Exclusively acquires the lock.
 */
_Acquires_nonreentrant_lock_(&lock->lock)
static void
os__privateLockAcquireExclusive (
    _Inout_ os_privateLock *lock)
{
    assert(lock);
    assert(os_syncModuleInitialized);

#ifndef NDEBUG
    assert(lock->signature == OS__SYNC_PRIV_SIG);
#endif

    if(os__nativeHaveSRWLockAndCondition){
        assert(os__nativeLockAcquireExclusive);
        os__nativeLockAcquireExclusive(&lock->lock);
    } else {
        EnterCriticalSection(&lock->lock.cs);
    }
}

/**
 * Releases an exclusively acquired lock.
 */
_Releases_nonreentrant_lock_(&lock->lock)
static void
os__privateLockReleaseExclusive (
    _Inout_ os_privateLock *lock)
{
    assert(lock);
    assert(os_syncModuleInitialized);

#ifndef NDEBUG
    assert(lock->signature == OS__SYNC_PRIV_SIG);
#endif

    if(os__nativeHaveSRWLockAndCondition){
        assert(os__nativeLockReleaseExclusive);
        os__nativeLockReleaseExclusive(&lock->lock);
    } else {
        LeaveCriticalSection(&lock->lock.cs);
    }
}

/**
 * Acquires a lock allowing sharing. It is not guaranteed that the acquired lock
 * is actually shared (may be exclusively owned instead).
 */
_Acquires_shared_lock_(&lock->lock)
static void
os__privateLockAcquireShared (
    _Inout_ os_privateLock *lock)
{
    assert(lock);
    assert(os_syncModuleInitialized);

#ifndef NDEBUG
    assert(lock->signature == OS__SYNC_PRIV_SIG);
#endif

    if(os__nativeHaveSRWLockAndCondition){
        assert(os__nativeLockAcquireShared);
        os__nativeLockAcquireShared(&lock->lock);
    } else {
        EnterCriticalSection(&lock->lock.cs);
    }
}

/**
 * Releases an lock that was acquired shared.
 */
_Releases_shared_lock_(&lock->lock)
static void
os__privateLockReleaseShared (
    _Inout_ os_privateLock *lock)
{
    assert(lock);
    assert(os_syncModuleInitialized);

#ifndef NDEBUG
    assert(lock->signature == OS__SYNC_PRIV_SIG);
#endif

    if(os__nativeHaveSRWLockAndCondition){
        assert(os__nativeLockReleaseShared);
        os__nativeLockReleaseShared(&lock->lock);
    } else {
        LeaveCriticalSection(&lock->lock.cs);
    }
}

/**
 * Deletes a lock.
 */
static void
os__privateLockDelete (
    _Inout_ os_privateLock *lock)
{
    assert(lock);
    assert(os_syncModuleInitialized);

#ifndef NDEBUG
    assert(lock->signature == OS__SYNC_PRIV_SIG);
#endif

    if( !os__nativeHaveSRWLockAndCondition ){
        DeleteCriticalSection (&lock->lock.cs);
    }

#ifndef NDEBUG
    lock->signature = 0;
#endif
}

static BOOL
WINAPI
os__nativeCondSleepConditionVariableSRWWrapper (
        _Inout_ void* cond,
        _Inout_ void* mutex,
        _In_ DWORD dwMilliseconds)
{
    assert(os__nativeCondSleepConditionVariableSRW);

    return os__nativeCondSleepConditionVariableSRW(cond, mutex, dwMilliseconds, 0);
}

////////////////////////////////////////////////////////////////////////////////
// NATIVE EVENT-BASED LOCK INTERFACE
////////////////////////////////////////////////////////////////////////////////
static os_result
WINAPI
os_syncEventLockInitialize(
    _Out_ void *mutex);

static void
WINAPI
os_syncEventLockAcquireExclusive (
    _Inout_ void *mutex);

static BOOLEAN
WINAPI
os_syncEventLockTryAcquireExclusive (
    _Inout_ void *mutex);

static void
WINAPI
os_syncEventLockReleaseExclusive (
    _Inout_ void *mutex);

static void
WINAPI
os_syncEventLockDelete(
    _Inout_ void *mutex);

////////////////////////////////////////////////////////////////////////////////
// EVENT-BASED LOCK IMPLEMENTATION
//
// The condition implementation provides an event-based, process-private
// lock that can be used in conjunction with the semaphore based condition
// variable implementation (e.g., for Windows XP).
////////////////////////////////////////////////////////////////////////////////

typedef struct os_syncEventLock_s {
    _Interlocked_ LONG count;
    HANDLE event;
} os_syncEventLock;

static os_result
WINAPI
os_syncEventLockInitialize(
    _Out_ void *m)
{
    os_syncEventLock *mutex = (os_syncEventLock *)m;

    assert(mutex);

    mutex->count = 0;
    mutex->event = CreateEvent (NULL,  // no security
                                FALSE, // auto-reset
                                FALSE, // non-signaled initially
                                NULL); // unnamed
    if(mutex->event == NULL) {
        return os_resultFail;
    }

    return os_resultSuccess;
}

static void
WINAPI
os_syncEventLockAcquireExclusive (
    _Inout_ void *m)
{
    os_syncEventLock *mutex = (os_syncEventLock *)m;

    if (!os__syncEventLockSpinAcquireExclusive(&mutex->count, OS__SYNC_PRIVATE_SPINLOCKCOUNT)) {
        (void) os__syncEventLockAcquireExclusive(mutex->event);
    }
}

static BOOLEAN
WINAPI
os_syncEventLockTryAcquireExclusive (
    _Inout_ void *m)
{
    os_syncEventLock *mutex = (os_syncEventLock *)m;

    if (InterlockedCompareExchangeAcquire(&mutex->count, 1, 0) > 0) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static void
WINAPI
os_syncEventLockReleaseExclusive (
    _Inout_ void *m)
{
    os_syncEventLock *mutex = (os_syncEventLock *)m;

    if (InterlockedDecrementRelease(&mutex->count) > 0) {
        if(SetEvent(mutex->event) == 0){
            /* TODO: Report failure through Application Event Log (OSPL-7672)? */
            abort();
        }
    }
}

static void
WINAPI
os_syncEventLockDelete(
    _Inout_ void *m)
{
    os_syncEventLock *mutex = (os_syncEventLock *)m;

    (void) CloseHandle(mutex->event);
}

////////////////////////////////////////////////////////////////////////////////
// SEMAPHORE CONDITION IMPLEMENTATION
//
// The condition implementation provides a semaphore based, process-private
// condition variable implementation (e.g., for Windows XP).
////////////////////////////////////////////////////////////////////////////////
typedef struct os_syncSemaphoreCond_s {
    HANDLE sem;
    HANDLE done;
    _Interlocked_ LONG waiters;
} os_syncCondSemaphore;

static void
WINAPI
os_syncSemaphoreCondInitialize (
    _Inout_ void *c)
{
    os_syncCondSemaphore *cond = (os_syncCondSemaphore *)c;

    assert(cond);

    cond->waiters = 0;
    cond->sem = CreateSemaphore(NULL,    // no security
                                0,       // initially 0
                                INT_MAX, // max count
                                NULL);   // unnamed
    cond->done = CreateEvent(NULL,  // no security
                             FALSE, // auto-reset
                             FALSE, // initially non-signaled
                             NULL); // unnamed
    /* TODO: Error reporting? */
}

static BOOL
WINAPI
os_syncSemaphoreCondSleep (
    _Inout_ void *c,
    _Inout_ void *m,
    _In_ DWORD dwMilliseconds)
{
    os_syncCondSemaphore *cond = (os_syncCondSemaphore *)c;
    os_syncEventLock *mutex = (os_syncEventLock *)m;
    os_uint32 last;
    DWORD wsr;

    assert(cond);
    assert(mutex);

    /* Increase the waiters-count and release the mutex */
    (void) InterlockedIncrementNoFence(&cond->waiters);
    if(InterlockedDecrementRelease(&mutex->count) > 0) {
        /* The mutex was contended, so signal the mutex and wait for the semaphore */
        wsr = SignalObjectAndWait(mutex->event, cond->sem, dwMilliseconds, FALSE);
    } else {
        /* The mutex wasn't contended, so wait for the semaphore */
        wsr = WaitForSingleObject(cond->sem, dwMilliseconds);
    }
    last = InterlockedDecrementRelease(&cond->waiters);

    /* In case of a broadcast, the last waiter has to signal the broadcaster to
     * ensure fairness. */
    if((last & OS__SYNC_BROADCAST) && !(last & ~OS__SYNC_BROADCAST)) {
        if (!os__syncEventLockSpinAcquireExclusive(&mutex->count, 0)) {
            SignalObjectAndWait (cond->done, mutex->event, INFINITE, FALSE);
        } else {
            SetEvent(cond->done);
        }
    } else {
        os_syncEventLockAcquireExclusive(mutex);
    }

    if (wsr != WAIT_OBJECT_0) {
        if (wsr == WAIT_TIMEOUT) {
            SetLastError(ERROR_TIMEOUT);
        }
        return FALSE;
    } else {
        return TRUE;
    }
}

static void
WINAPI
os_syncSemaphoreCondWake (
    _Inout_ void *c)
{
    os_syncCondSemaphore *cond = (os_syncCondSemaphore *)c;
    assert(cond);

    if((volatile LONG *)&cond->waiters) {
        ReleaseSemaphore(cond->sem, 1, 0);
    }
}

static void
WINAPI
os_syncSemaphoreCondWakeAll (
    _Inout_ void *c)
{
    os_syncCondSemaphore *cond = (os_syncCondSemaphore *)c;
    LONG waiters;

    assert(cond);

    waiters = InterlockedOr(&cond->waiters, OS__SYNC_BROADCAST);
    if (waiters == 0) { /* no waiters */
        /* wakeAll may only be called with mutex held, so we can safely reset
         * the broadcast mask. */
        InterlockedAnd(&cond->waiters, ~OS__SYNC_BROADCAST);
        return;
    }

    ReleaseSemaphore(cond->sem, waiters, 0);

    /* Wait for all awakened threads to acquire the counting semaphore. */
    WaitForSingleObject (cond->done, INFINITE);
    /* Reset the broadcast-bit */
    InterlockedAnd(&cond->waiters, ~OS__SYNC_BROADCAST);
}

static void
WINAPI
os_syncSemaphoreCondDelete(
    _Inout_ void *c)
{
    os_syncCondSemaphore *cond = (os_syncCondSemaphore *)c;

    assert(cond);

    (void) CloseHandle(cond->done);
    (void) CloseHandle(cond->sem);
}

struct os__sync_dyn_locks_compile_constraints {
    char require_sizeof_CRITICAL_SECTION_eq_sizeof_os__syncLockPrivateCRITSEC[(sizeof(CRITICAL_SECTION) == sizeof(os__syncLockPrivateCRITSEC)) ? 1 : -1];
    char require_sizeof_os_syncEventLock_eq_sizeof_os__syncLockPrivateEVENT[(sizeof(os_syncEventLock) == sizeof(os__syncLockPrivateEVENT)) ? 1 : -1];
#ifdef SRWLOCK_INIT
    char require_sizeof_SRWLOCK_eq_sizeof_os__syncLockPrivateSRWLOCK[(sizeof(SRWLOCK) == sizeof(os__syncLockPrivateSRWLOCK)) ? 1 : -1];
#endif /* SRWLOCK_INIT */
    char require_sizeof_os_condPrivateSEMCOND_eq_sizeof_os_syncCondSemaphore[(sizeof(os__syncCondPrivateSEMCOND) == sizeof(os_syncCondSemaphore)) ? 1 : -1];
#ifdef CONDITION_VARIABLE_INIT
    char require_sizeof_CONDITION_VARIABLE_eq_sizeof_os__syncCondPrivateCONDVAR[(sizeof(CONDITION_VARIABLE) == sizeof(os__syncCondPrivateCONDVAR)) ? 1 : -1];
#endif /* CONDITION_VARIABLE_INIT */
    char non_empty_dummy_last_member[1];
};

#else
/** SRWLOCK's and CONDITION_VARIABLE's were available at compile-time, so the os__native* calls are
 * directly mapped to the native implementation. */
#define os__nativeHaveSRWLockAndCondition (TRUE)

/** See InitializeSRWLock at MSDN. Always evaluates to TRUE for interface compatibility. */
#define os__nativeLockInitialize(l)       (InitializeSRWLock(l),TRUE)
/** See InitializeSRWLock at MSDN. */
#define os__nativeLockInitializeSRWLock   InitializeSRWLock
/** See AcquireSRWLockExclusive at MSDN. */
#define os__nativeLockAcquireExclusive    AcquireSRWLockExclusive
/** See TryAcquireSRWLockExclusive at MSDN. */
#define os__nativeLockTryAcquireExclusive TryAcquireSRWLockExclusive
/** See ReleaseSRWLockExclusive at MSDN. */
#define os__nativeLockReleaseExclusive    ReleaseSRWLockExclusive
/** See AcquireSRWLockShared at MSDN. */
#define os__nativeLockAcquireShared       AcquireSRWLockShared
/** See TryAcquireSRWLockShared at MSDN. */
#define os__nativeLockTryAcquireShared    TryAcquireSRWLockShared
/** See ReleaseSRWLockShared at MSDN. */
#define os__nativeLockReleaseShared       ReleaseSRWLockShared
/** For interface compatibility. No-op. */
#define os__nativeLockDelete(l)

typedef SRWLOCK os_privateLock;

#define os__privateLockInitialize          os__nativeLockInitialize
#define os__privateLockAcquireExclusive    os__nativeLockAcquireExclusive
#define os__privateLockTryAcquireExclusive os__nativeLockTryAcquireExclusive
#define os__privateLockReleaseExclusive    os__nativeLockReleaseExclusive
#define os__privateLockAcquireShared       os__nativeLockAcquireShared
#define os__privateLockTryAcquireShared    os__nativeLockTryAcquireShared
#define os__privateLockReleaseShared       os__nativeLockReleaseShared
#define os__privateLockDelete(l)           /* No delete for SRWLock */

/* os__nativeCondInitialize must evaluate to TRUE iff success */
#define os__nativeCondInitializeConditionVariable(c)  (InitializeConditionVariable(c),TRUE)
#define os__nativeCondSleepConditionVariable(c, m, s) (os_resultFail)
#define os__nativeCondSleepConditionVariableSRW       SleepConditionVariableSRW
#define os__nativeCondWakeConditionVariable           WakeConditionVariable
#define os__nativeCondWakeAllConditionVariable        WakeAllConditionVariable
#define os__nativeCondDeleteConditionVariable(c)

#define os_syncEventLockInitialize(lock)  (os_resultFail)
#endif /* OS__DYNAMICALLY_SELECT_LOCKTYPE */

#if !OS__SYNC_NOSHARED
////////////////////////////////////////////////////////////////////////////////
// EVENT-HANDLE-CACHE INTERFACE
////////////////////////////////////////////////////////////////////////////////
#define OS__EVENTHANDLETABLE_NBITSKEY (12) /* #bitsIn(3 * OS__EVENTHANDLE_LIMIT / 2) */
#define OS__EVENTHANDLETABLE_INIT_SIZE (4096) /* 2^OS__EVENTHANDLETABLE_NBITSKEY */

struct os__eventHandleHashEntry {
    struct os__eventHandleHashEntry *next; /* Is used to detect the existence of a chain (i.e. collisions) */
    void *m; /* Is used in chain-heads to mark in-use or free */
    HANDLE h;
};

static struct os__eventHandleHashTable {
#ifndef NDEBUG
    UINT32 signature;
#endif
    SECURITY_ATTRIBUTES sa; /* Used for Creation of events/semaphores */
    BOOL                saInitialized;
    os_privateLock      lock;
    unsigned int        generation;
    struct os__eventHandleHashEntry heads[OS__EVENTHANDLETABLE_INIT_SIZE];
} ht;

typedef union os__syncCreateCallbackArg_u {
    struct {
        BOOL bManualReset;
        BOOL bInitialState;
    } event;
    struct {
        LONG lInitialCount;
        LONG lMaximumCount;
    } semaphore;
} os__syncCreateCallbackArg;

typedef _Ret_maybenull_ HANDLE (*os__syncCreateCallback_func) (_In_ void*, _In_ os__syncCreateCallbackArg*);

/**
 * Initializes the static event-handle cache. This must be invoked before calling
 * any of the
 */
_Pre_satisfies_( !ht.saInitialized )
_Post_satisfies_( ht.saInitialized )
static void
os__eventHandleTableInit(void);

_Pre_satisfies_( ht.saInitialized )
_Post_satisfies_( !ht.saInitialized )
static void
os__eventHandleTableDeinit(void);

_Pre_satisfies_( ht.saInitialized )
static _Ret_maybenull_ HANDLE
os__eventHandleRetrieveWithCallback(
    _In_ void *e,
    _In_ os__syncCreateCallback_func createCallback,
    _In_ os__syncCreateCallbackArg *arg);

_Pre_satisfies_( ht.saInitialized )
static _Ret_maybenull_ HANDLE
os__eventHandleStore(
      int idx,
      _In_ void *e,
      unsigned int generation,
      _In_ os__syncCreateCallback_func createCallback,
      _In_ os__syncCreateCallbackArg *arg);

_Pre_satisfies_( ht.saInitialized )
static _Ret_maybenull_ HANDLE
os__eventHandleRetrieve(
      _In_ void *e);

_Pre_satisfies_( ht.saInitialized )
static _Ret_maybenull_ HANDLE
os__semaphoreHandleRetrieve(
    _In_ void* sem);

_Pre_satisfies_( ht.saInitialized )
static void
os__eventHandleDelete(
    _In_ void *e);
/* EVENT-HANDLE-CACHE */
#endif /* OS__SYNC_NOSHARED */

_Pre_satisfies_( !os_syncModuleInitialized )
_Post_satisfies_( os_syncModuleInitialized )
void
os_syncModuleInit(void)
{
    /* TODO: init-once */
#ifdef OS__DYNAMICALLY_SELECT_LOCKTYPE
    FARPROC proc;
    HMODULE km;

    /* kernel32.dll is always available */
    (void) GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN , "kernel32.dll", &km);
    assert(km);

    /* First check "TryAcquireSRWLockExclusive" or "TryAcquireSRWLockShared",
     * because those are only available from Windows 7 onwards. Even though
     * Windows Vista does support SRW-locks, the implementation lacks support
     * for TryAcquireSRWLock*, so on Windows Vista the fallback implementation
     * is used. */
    if((proc = GetProcAddress(km, "TryAcquireSRWLockExclusive")) != NULL){
        os__nativeHaveSRWLockAndCondition = TRUE;
    } else {
        os__nativeHaveSRWLockAndCondition = FALSE;
    }

    /* Try to resolve "InitializeConditionVariable". This is available from
     * Windows Vista and onwards. If it is not available, the condition variable
     * implementation will have to based on kernel-events and so the mutex
     * implementation must also use kernel events. On Windows XP the
     * CRITICAL_SECTION implementation is thus never used even though the mutex
     * part would be fully supported. */
    if((proc = GetProcAddress(km, "InitializeConditionVariable")) != NULL){
        os__nativeCondInitializeConditionVariable = (os__nativeCondInitializeConditionVariable_func) proc;
        os__nativeCondDeleteConditionVariable = NULL;
        if(os__nativeHaveSRWLockAndCondition){
            os__nativeCondSleepConditionVariableSRW = (os__nativeCondSleepConditionVariableSRW_func) GetProcAddress(km, "SleepConditionVariableSRW");
            os__nativeCondSleepConditionVariable = &os__nativeCondSleepConditionVariableSRWWrapper;
        } else {
            os__nativeCondSleepConditionVariable = (os__nativeCondSleepConditionVariable_func) GetProcAddress(km, "SleepConditionVariableCS");
        }
        os__nativeCondWakeConditionVariable = (os__nativeCondWakeConditionVariable_func) GetProcAddress(km, "WakeConditionVariable");
        os__nativeCondWakeAllConditionVariable = (os__nativeCondWakeAllConditionVariable_func) GetProcAddress(km, "WakeAllConditionVariable");
    } else {
        /* Windows XP fallback */
        assert(!os__nativeHaveSRWLockAndCondition);
        os__nativeCondInitializeConditionVariable = &os_syncSemaphoreCondInitialize;
        os__nativeCondDeleteConditionVariable = &os_syncSemaphoreCondDelete;
        os__nativeCondSleepConditionVariable = &os_syncSemaphoreCondSleep;
        os__nativeCondWakeConditionVariable = &os_syncSemaphoreCondWake;
        os__nativeCondWakeAllConditionVariable = &os_syncSemaphoreCondWakeAll;
    }
    assert(os__nativeCondInitializeConditionVariable);
    assert(os__nativeHaveSRWLockAndCondition || os__nativeCondDeleteConditionVariable);
    assert(os__nativeCondSleepConditionVariable);
    assert(os__nativeCondWakeConditionVariable);
    assert(os__nativeCondWakeAllConditionVariable);

    /* First check "TryAcquireSRWLockExclusive" or "TryAcquireSRWLockShared",
     * because those are only available from Windows 7 onwards. Even though
     * Windows Vista does support SRW-locks, the implementation lacks support
     * for TryAcquireSRWLock*, so on Windows Vista the fallback implementation
     * is used. */
    if(os__nativeHaveSRWLockAndCondition) {
        os__nativeLockInitializeSRWLock = (os__nativeLockInitializeSRWLock_func) GetProcAddress(km, "InitializeSRWLock");
        os__nativeLockDelete = NULL;
        os__nativeLockAcquireExclusive = (os__nativeLockAcquireExclusive_func) GetProcAddress(km, "AcquireSRWLockExclusive");
        os__nativeLockTryAcquireExclusive = (os__nativeLockTryAcquireExclusive_func) GetProcAddress(km, "TryAcquireSRWLockExclusive");
        os__nativeLockReleaseExclusive = (os__nativeLockReleaseExclusive_func) GetProcAddress(km, "ReleaseSRWLockExclusive");
        os__nativeLockAcquireShared = (os__nativeLockAcquireShared_func) GetProcAddress(km, "AcquireSRWLockShared");
        os__nativeLockTryAcquireShared = (os__nativeLockTryAcquireShared_func) GetProcAddress(km, "TryAcquireSRWLockShared");
        os__nativeLockReleaseShared = (os__nativeLockReleaseShared_func) GetProcAddress(km, "ReleaseSRWLockShared");
    } else {
        /* In order to be able to implement condition variables on platforms that
         * don't have native conditions variable support, the mutexes must be
         * implemented on kernel-events too. It is not possible to use
         * CRITICAL_SECTIONS in this case. */
        os__nativeLockInitializeSRWLock = NULL;
        os__nativeLockDelete = &os_syncEventLockDelete;
        os__nativeLockAcquireExclusive = &os_syncEventLockAcquireExclusive;
        os__nativeLockTryAcquireExclusive = &os_syncEventLockTryAcquireExclusive;
        os__nativeLockReleaseExclusive = &os_syncEventLockReleaseExclusive;
        os__nativeLockAcquireShared = &os_syncEventLockAcquireExclusive; /* No shared native locks in fallback for now */
        os__nativeLockTryAcquireShared = &os_syncEventLockTryAcquireExclusive; /* No shared native locks in fallback for now */
        os__nativeLockReleaseShared = &os_syncEventLockReleaseExclusive; /* No shared native locks in fallback for now */
    }

    assert(!os__nativeHaveSRWLockAndCondition || os__nativeLockInitializeSRWLock);
    assert(os__nativeHaveSRWLockAndCondition || os__nativeLockDelete);
    assert(os__nativeLockAcquireExclusive);
    assert(os__nativeLockTryAcquireExclusive);
    assert(os__nativeLockReleaseExclusive);
    assert(os__nativeLockAcquireShared);
    assert(os__nativeLockTryAcquireShared);
    assert(os__nativeLockReleaseShared);
#else
    /* In case the code was compiled with _WIN32_WINNT set to such that older
     * Windows' are excluded, the code will use the new API directly. */
    assert(os__nativeHaveSRWLockAndCondition);
#endif /* OS__DYNAMICALLY_SELECT_LOCKTYPE */

    os_syncModuleInitialized = TRUE;
#if !OS__SYNC_NOSHARED
    os__eventHandleTableInit();
#endif /* OS__SYNC_NOSHARED */
}

_Pre_satisfies_( os_syncModuleInitialized )
_Post_satisfies_( !os_syncModuleInitialized )
void
os_syncModuleDeinit(void)
{
#if !OS__SYNC_NOSHARED
    os__eventHandleTableDeinit();
#endif /* OS__SYNC_NOSHARED */
    os_syncModuleInitialized = FALSE;
}

_Pre_satisfies_( os_syncModuleInitialized )
_Check_return_
os_result
os_syncLockInitialize (
    _Out_ _When_(return != os_resultSuccess, _Post_invalid_) void* lock)
{
    assert(lock);
    if( os__nativeHaveSRWLockAndCondition ) {
        os__nativeLockInitializeSRWLock(lock);
        return os_resultSuccess;
    } else {
        return os_syncEventLockInitialize(lock);
    }
}

_Pre_satisfies_( os_syncModuleInitialized )
_Acquires_nonreentrant_lock_(lock)
void
os_syncLockAcquireExclusive (
    _Inout_ void* lock)
{
    assert(lock);
    os__nativeLockAcquireExclusive(lock);
}

_Pre_satisfies_( os_syncModuleInitialized )
_Check_return_
_When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(lock))
os_result
os_syncLockTryAcquireExclusive (
    _Inout_ void* lock)
{
    assert(lock);
    return os__nativeLockTryAcquireExclusive(lock) ? os_resultSuccess : os_resultBusy;
}

_Pre_satisfies_( os_syncModuleInitialized )
_Releases_nonreentrant_lock_(lock)
void
os_syncLockReleaseExclusive (
    _Inout_ void* lock)
{
    assert(lock);
    os__nativeLockReleaseExclusive(lock);
}

_Pre_satisfies_( os_syncModuleInitialized )
void
os_syncLockDelete(
    _Inout_ _Post_invalid_ void* lock)
{
    assert(lock);
    if( !os__nativeHaveSRWLockAndCondition ) {
        os__nativeLockDelete(lock);
    }
}

_Pre_satisfies_( os_syncModuleInitialized )
void
os_syncCondInitialize(
    _Out_ void *cond)
{
    os__nativeCondInitializeConditionVariable(cond);
}

_Pre_satisfies_( os_syncModuleInitialized )
_Requires_exclusive_lock_held_(lock)
_When_(dwMilliseconds == 0xFFFFFFFF, _Ret_range_(os_resultSuccess, os_resultSuccess))
os_result
os_syncCondSleep(
    _Inout_ void *cond,
    _Inout_ void *lock,
    _In_ DWORD dwMilliseconds)
{
    BOOL result;

    assert(cond);
    assert(lock);

    result = os__nativeCondSleepConditionVariable(cond, lock, dwMilliseconds);

    if( result ) {
        return os_resultSuccess;
    } else if(GetLastError() != ERROR_TIMEOUT) {
        abort();
    } else if (dwMilliseconds != INFINITE) {
        return os_resultTimeout;
    }
    return os_resultSuccess;
}

_Pre_satisfies_( os_syncModuleInitialized )
void
os_syncCondWake(
    _Inout_ void *cond)
{
    assert(cond);

    os__nativeCondWakeConditionVariable(cond);
}

_Pre_satisfies_( os_syncModuleInitialized )
void
os_syncCondWakeAll(
    _Inout_ void *cond)
{
    assert(cond);

    os__nativeCondWakeAllConditionVariable(cond);
}

_Pre_satisfies_( os_syncModuleInitialized )
void
os_syncCondDelete(
    _Inout_ _Post_invalid_ void *cond)
{
    assert(cond);

    os__nativeCondDeleteConditionVariable(cond);
}

#if !OS__SYNC_NOSHARED
/**
 * The only information actually shared is the lock-count. This counter is atomically modified by
 * all processes (as an optimisation, to prevent kernel-access in the uncontended case). The
 * implementation further relies on a kernel-event (name derived from address of the lock) for the
 * contended case. */
typedef struct os_syncSharedLock_s {
    _Interlocked_ LONG count;
} os_syncSharedLock;

_Pre_satisfies_( os_syncModuleInitialized )
_Check_return_
os_result
os_syncSharedLockInitialize(
    _Out_ _When_(return != os_resultSuccess, _Post_invalid_) os__syncLockShared *m)
{
    os_syncSharedLock *mutex = (os_syncSharedLock *)m;

    assert(mutex);

    mutex->count = 0;

    /* No need to initialize the event on which the shared lock is built; that
     * is done on first access as well. This is however the only point in time
     * where we can report a potential error that occurs when creating the event
     * in the Kernel, so we retrieve the event-handle now to ensure it can be
     * created. */
    if(os__eventHandleRetrieve(mutex) == NULL) {
        /* TODO: report error? */
        return os_resultFail;
    }

    return os_resultSuccess;
}

_Pre_satisfies_( os_syncModuleInitialized )
_When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(m))
os_result
os_syncSharedLockAcquireExclusive (
    _Inout_ os__syncLockShared *m)
{
    os_syncSharedLock *mutex = (os_syncSharedLock *)m;

    if (!os__syncEventLockSpinAcquireExclusive(&mutex->count, OS__SYNC_SHARED_SPINLOCKCOUNT)) {
        HANDLE eventHandle;

        eventHandle = os__eventHandleRetrieve(mutex);
        assert(eventHandle);
        return os__syncEventLockAcquireExclusive(eventHandle);
    }

    return os_resultSuccess;
}

_Pre_satisfies_( os_syncModuleInitialized )
_Check_return_
_When_(return == os_resultSuccess, _Acquires_nonreentrant_lock_(m))
os_result
os_syncSharedLockTryAcquireExclusive (
    _Inout_ os__syncLockShared *m)
{
    os_syncSharedLock *mutex = (os_syncSharedLock *)m;

    if (InterlockedCompareExchangeAcquire(&mutex->count, 1, 0) > 0) {
        return os_resultBusy;
    } else {
        return os_resultSuccess;
    }
}

_Pre_satisfies_( os_syncModuleInitialized )
_Releases_nonreentrant_lock_(m)
void
os_syncSharedLockReleaseExclusive (
    _Inout_ os__syncLockShared *m)
{
    os_syncSharedLock *mutex = (os_syncSharedLock *)m;

    if (InterlockedDecrementRelease(&mutex->count) > 0) {
        HANDLE eventHandle = os__eventHandleRetrieve(mutex);
        assert(eventHandle);
        if(SetEvent(eventHandle) == 0){
            /* TODO: Report failure through Application Event Log (OSPL-7672)? */
            abort();
        }
    }
}

_Pre_satisfies_( os_syncModuleInitialized )
void
os_syncSharedLockDelete(
    _In_ _Post_invalid_ os__syncLockShared *m)
{
    os_syncSharedLock *mutex = (os_syncSharedLock *)m;

    os__eventHandleDelete(mutex);
}

#define OS_SYNC_SHARED_SEMADDR(cond) (((char *)(cond)) + 1)
/* Obtain a faux address for hashing that is related to the condition, but is
 * guaranteed to not collide with another condition. */
#define OS_SYNC_SHARED_EVTADDR(cond) (cond)

/**
 * The only information actually shared is the waitcount. This counter is atomically modified by
 * all processes. The implementation further relies on a kernel-event (name derived from address
 * of the shared-condition) and a kernel-semaphore (name derived from address of the shared-
 * condition). */
typedef struct os_syncSharedCond_s {
    _Interlocked_ LONG waiters;
} os_syncSharedCond;

_Pre_satisfies_( os_syncModuleInitialized )
_Check_return_
os_result
os_syncSharedCondInitialize(
    _Out_ _When_(return != os_resultSuccess, _Post_invalid_) os__syncCondShared *c)
{
    os_syncSharedCond *cond = (os_syncSharedCond *)c;
    assert(cond);

    cond->waiters = 0;

    if(os__eventHandleRetrieve(OS_SYNC_SHARED_EVTADDR(cond)) == NULL) {
        /* TODO: report error? */
        goto err_doneEventCreate;
    }

    if(os__semaphoreHandleRetrieve(OS_SYNC_SHARED_SEMADDR(cond)) == NULL) {
        /* TODO: report error? */
        goto err_doneSemaphoreCreate;
    }
    return os_resultSuccess;

err_doneSemaphoreCreate:
    os__eventHandleDelete(OS_SYNC_SHARED_EVTADDR(cond));
err_doneEventCreate:
    return os_resultFail;
}

_Pre_satisfies_( os_syncModuleInitialized )
_Requires_exclusive_lock_held_(m)
_When_(dwMilliseconds == 0xFFFFFFFF, _Ret_range_(os_resultSuccess, os_resultSuccess))
os_result
os_syncSharedCondSleep(
    _Inout_ os__syncCondShared *c,
    _Inout_ os__syncLockShared *m,
    _In_ DWORD dwMilliseconds)
{
    os_syncSharedCond *cond = (os_syncSharedCond *)c;
    os_syncSharedLock *mutex = (os_syncSharedLock *)m;
    os_uint32 last;
    DWORD wsr;
    HANDLE condSem;
    HANDLE mutexEvent = NULL;

    assert(cond);
    assert(mutex);

    condSem = os__semaphoreHandleRetrieve(OS_SYNC_SHARED_SEMADDR(cond));
    assert(condSem);
    /* Increase the waiters-count and release the mutex */
    (void) InterlockedIncrementNoFence(&cond->waiters);
    if(InterlockedDecrementRelease(&mutex->count) > 0) {
        mutexEvent = os__eventHandleRetrieve(mutex);
        assert(mutexEvent);
        /* The mutex was contended, so signal the mutex and wait for the semaphore */
        wsr = SignalObjectAndWait(mutexEvent, condSem, dwMilliseconds, FALSE);
    } else {
        /* The mutex wasn't contended, so wait for the semaphore */
        wsr = WaitForSingleObject(condSem, dwMilliseconds);
    }
    last = InterlockedDecrementRelease(&cond->waiters);

    /* In case of a broadcast, the last waiter has to signal the broadcaster to
     * ensure fairness. */
    if((last & OS__SYNC_BROADCAST) && !(last & ~OS__SYNC_BROADCAST)) {
        HANDLE condDone = os__eventHandleRetrieve(OS_SYNC_SHARED_EVTADDR(cond));
        assert(condDone);
        if (!os__syncEventLockSpinAcquireExclusive(&mutex->count, 0)) {
            mutexEvent = mutexEvent ? mutexEvent : os__eventHandleRetrieve(mutex);
            assert(mutexEvent);
            SignalObjectAndWait (condDone, mutexEvent, INFINITE, FALSE);
        } else {
            /* Acquired the mutex with an InterlockedIncrement, so no need to
             * signal the mutex event. */
            SetEvent(condDone);
        }
    } else {
        os_syncSharedLockAcquireExclusive(m);
    }

    /* TODO: process other return values */
    return wsr == WAIT_OBJECT_0 || dwMilliseconds == INFINITE ? os_resultSuccess : os_resultTimeout;
}

_Pre_satisfies_( os_syncModuleInitialized )
void
os_syncSharedCondWake(
    _Inout_ os__syncCondShared *c)
{
    os_syncSharedCond *cond = (os_syncSharedCond *)c;

    assert(cond);

    if(*((volatile LONG *)&cond->waiters)) {
        HANDLE condSem;

        condSem = os__semaphoreHandleRetrieve(OS_SYNC_SHARED_SEMADDR(cond));
        if(!condSem) {
            /* TODO: report? */
            abort();
        }
        ReleaseSemaphore(condSem, 1, NULL);
    }
}

_Pre_satisfies_( os_syncModuleInitialized )
void
os_syncSharedCondWakeAll(
    _Inout_ os__syncCondShared *c)
{
    os_syncSharedCond *cond = (os_syncSharedCond *)c;
    HANDLE condSem;
    HANDLE condDone;
    LONG waiters;

    assert(cond);

    waiters = InterlockedOr(&cond->waiters, OS__SYNC_BROADCAST);
    if (waiters <= 1) { /* zero or one waiters */
        /* wakeAll may only be called with mutex held, so we can safely reset
         * the broadcast mask. */
        InterlockedAnd(&cond->waiters, ~OS__SYNC_BROADCAST);
        if (waiters == 1) {
            os_syncSharedCondWake(c);
        }
        return;
    }

    condSem = os__semaphoreHandleRetrieve(OS_SYNC_SHARED_SEMADDR(cond));
    assert(condSem);
    condDone = os__eventHandleRetrieve(OS_SYNC_SHARED_EVTADDR(cond));
    assert(condDone);
    ReleaseSemaphore(condSem, waiters, NULL);

    /* Wait for all awakened threads to acquire the counting semaphore.
     * Use a timeout to prevent waiting for a crashed participant
     * signal the condition*/
    WaitForSingleObject (condDone, 1000);
    /* Reset the broadcast-bit */
    InterlockedAnd(&cond->waiters, ~OS__SYNC_BROADCAST);
}

_Pre_satisfies_( os_syncModuleInitialized )
void
os_syncSharedCondDelete(
    _In_ _Post_invalid_ os__syncCondShared *c)
{
    os_syncSharedCond *cond = (os_syncSharedCond *)c;

    os__eventHandleDelete(OS_SYNC_SHARED_SEMADDR(cond));
    os__eventHandleDelete(OS_SYNC_SHARED_EVTADDR(cond));
}

struct os__sync_shared_compile_constraints {
    char require_sizeof_os__syncCondShared_eq_sizeof_os_syncSharedCond[(sizeof(os__syncCondShared) == sizeof(os_syncSharedCond)) ? 1 : -1];
    char require_sizeof_os__syncLockShared_eq_sizeof_os_syncSharedLock[(sizeof(os__syncLockShared) == sizeof(os_syncSharedLock)) ? 1 : -1];
    char non_empty_dummy_last_member[1];
};

////////////////////////////////////////////////////////////////////////////////
// EVENT-HANDLE-CACHE IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////
#define UINT64_CONST(x, y, z) (((os_uint64) (x) * 1000000 + (y)) * 1000000 + (z))

/* Preferably prime (at least uneven), in the range 2^62-2^63 with a random bit pattern */
static const os_uint64 voidUniHashConst = UINT64_CONST (14585777, 916479, 446343);

/* Zeroed initializer for os__eventHandleHashEntries */
static const struct os__eventHandleHashEntry OS_EVENTHANDLEHASHENTRY_INIT;

/* #define os__eventHash(m) (unsigned int) (((os_uint32)(m) * voidUniHashConst) >> (64 - ht.nbitskey)) */
/**
 * Calculates the hash of an address. Since the high-order bits of the address
 * are ignored on 64-bit platforms, the chances of hash-collissions will increase
 * if the hashed address-space is >32bit.
 */
static unsigned int
os__eventHash(
    _In_ void* m)
{
    return (unsigned int) (((os_uint32) (m) * voidUniHashConst) >> (64 - OS__EVENTHANDLETABLE_NBITSKEY));
}

_Pre_satisfies_( !ht.saInitialized )
_Post_satisfies_( ht.saInitialized )
void
os__eventHandleTableInit(void)
{
#ifndef NDEBUG
    assert(ht.signature != OS__SYNC_HT_SIG);
    assert(ht.saInitialized == FALSE);
#endif
    /* Vista and on have tightened security with regard to shared memory. We therefore need
     * need to grant rights to all users via a discretionary access control list. NULL
     * attributes do not allow users other than process starter access. */
    ht.sa.nLength = sizeof(ht.sa);
    ht.saInitialized = ConvertStringSecurityDescriptorToSecurityDescriptor
                            ("D:P(A;OICI;GA;;;WD)", /* grant all acess to 'world' (everyone) */
                            SDDL_REVISION_1, &ht.sa.lpSecurityDescriptor, NULL);
    if(!ht.saInitialized){
        /* TODO: Report failure through Application Event Log (OSPL-7672)? */
        goto fail_sa;
    }

    if (!os__privateLockInitialize(&ht.lock)){
        /* TODO: Report failure through Application Event Log (OSPL-7672)? */
        goto fail_lock;
    }

#ifndef NDEBUG
    ht.signature = OS__SYNC_HT_SIG;
#endif
    return;

fail_lock:
    os_free(ht.heads);
#if 0
fail_heads:
#endif
fail_sa:
#if 0
    LocalFree(ht.sa);
    ht.sa = NULL;
fail_sa_alloc:
#endif
    assert(ht.saInitialized == TRUE);

    return;
}

_Pre_satisfies_( ht.saInitialized )
_Post_satisfies_( !ht.saInitialized )
void
os__eventHandleTableDeinit(void)
{
    int i;
    struct os__eventHandleHashEntry *h, *p;
#ifndef NDEBUG
    assert(ht.signature == OS__SYNC_HT_SIG);
    ht.signature = 0;
#endif
    os__privateLockAcquireExclusive(&ht.lock);

    for(i = 0; i < OS__EVENTHANDLETABLE_INIT_SIZE; i++){
        if((h = ht.heads[i].next) != NULL){
            do{
                if(!CloseHandle(h->h)){
                    /* TODO: Report failure through Application Event Log (OSPL-7672)? */
                }
                p = h;
                h = h->next;
                os_free(p);
            } while (h);
        } if (ht.heads[i].h){
            if(!CloseHandle(ht.heads[i].h)){
                /* TODO: Report failure through Application Event Log (OSPL-7672)? */
            }
            memset(&ht.heads[i], 0, sizeof(ht.heads[i]));
        }
    }

    ht.generation++;
    os__privateLockReleaseExclusive(&ht.lock);

    os__privateLockDelete(&ht.lock);

    assert(ht.saInitialized == TRUE);
    ht.saInitialized = LocalFree(ht.sa.lpSecurityDescriptor) == NULL ? FALSE : TRUE;
    assert(ht.saInitialized == FALSE);

    return;
}

#ifndef _WIN64
  #define OS__EVENT_NAME_TEMPLATE "Global\\OSPL00000000"
#else
  #define OS__EVENT_NAME_TEMPLATE "Global\\OSPL0000000000000000"
#endif

static void
os__eventAddressToName(
    _In_ void *addr,
    _Inout_updates_z_(sizeof OS__EVENT_NAME_TEMPLATE) char *name) /* SAL could perhaps be more precise */
{
    size_t i = sizeof(OS__EVENT_NAME_TEMPLATE) - 1; /* strlen(name) - 1 */
    uintptr_t addr_tmp = (uintptr_t)addr;
    while(addr_tmp){
        name[--i] = "0123456789ABCDEF"[0xF & addr_tmp];
        addr_tmp >>= 4;
    }
}

_Pre_satisfies_( ht.saInitialized )
_Ret_maybenull_ HANDLE
os__eventHandleStore(
    int idx,
    _In_ void* m,
    unsigned int generation,
    _In_ os__syncCreateCallback_func createCallback,
    _In_ os__syncCreateCallbackArg *arg)
{
    os_result r = os_resultSuccess;
    struct os__eventHandleHashEntry *hh;
    HANDLE h = NULL;

#ifndef NDEBUG
    assert(ht.signature == OS__SYNC_HT_SIG);
#endif
    assert(0 <= idx && idx < OS__EVENTHANDLETABLE_INIT_SIZE);

    os__privateLockAcquireExclusive(&ht.lock);

    if (ht.generation != generation) {
        /* Something changed, so verify that the handle has not yet been inserted */
        struct os__eventHandleHashEntry *next = &ht.heads[idx];
        while (next && next->m && next->m != m) {
            next = next->next;
        }
        if (next && next->m) {
            h = next->h;
        }
    }

    if (h == NULL) {
        /* Nothing changed since the detection of the missing handle, or we verified it does not
         * exist yet, so we can fill head of chain or append the handle to the head of the chain. */
        if ((h = createCallback(m, arg)) == NULL) {
            os__privateLockReleaseExclusive(&ht.lock);
            goto fail_handle_create;
        }
        hh = &ht.heads[idx];
        if (hh->m == NULL) {
            /* The head of the chain is still empty, so just fill it */
            assert(hh->h == NULL);
            assert(hh->next == NULL);
            hh->h = h;
            hh->m = m;
            hh->next = NULL;
        } else {
            /* The head of the chain is filled, so append to head and prepend eventual tail. */
            hh = os_malloc(sizeof *hh);
            hh->h = h;
            hh->m = m;
            hh->next = ht.heads[idx].next;
            ht.heads[idx].next = hh;
        }

        /* The table was modified, so increase generation. Don't care about over-
         * flow, since it is checked for equality only and modified by increments of 1,
         * so only a problem if a thread gets scheduled out while others increase the
         * generation MAX_UINT times. */
        ht.generation++;
    }

    os__privateLockReleaseExclusive(&ht.lock);

    return h;

fail_handle_create:
    return NULL;
}

_Pre_satisfies_( ht.saInitialized )
_Ret_maybenull_ HANDLE
os__eventHandleRetrieveWithCallback(
    _In_ void *e,
    _In_ os__syncCreateCallback_func createCallback,
    _In_ os__syncCreateCallbackArg *arg)
{
    os_result r = os_resultSuccess;
    HANDLE result = NULL;
    struct os__eventHandleHashEntry *next;
    unsigned int idx = os__eventHash(e);
    unsigned int generation;

#ifndef NDEBUG
    assert(ht.signature == OS__SYNC_HT_SIG);
#endif
    assert(0 <= idx && idx < OS__EVENTHANDLETABLE_INIT_SIZE);

    os__privateLockAcquireShared(&ht.lock);

    /* Lookup */
    next = &ht.heads[idx];
    while (next && next->m && next->m != e) {
        next = next->next;
    }

    if (next && next->m) {
        assert(next->m == e);
        assert(next->h);

        result = next->h;
    } else {
        /* The handle is not found. Since only a shared-lock is acquired,
         * the current generation needs to be marked, to ensure the new
         * handle is inserted only once (and optimally so) when the
         * exclusive lock is acquired. */
        generation = ht.generation;
    }

    os__privateLockReleaseShared(&ht.lock);

    if (!result) { /* The handle was not found, so create and insert */
        result = os__eventHandleStore(idx, e, generation, createCallback, arg);
    }

    return result;
}

static
_Ret_maybenull_
HANDLE
os__syncEventCreate(
    _In_ void *m,
    _In_ os__syncCreateCallbackArg *arg)
{
    char name[] = OS__EVENT_NAME_TEMPLATE;
    HANDLE event;

    assert(ht.saInitialized == TRUE);

    /* Use the address of the mutex as name. Since a shared mutex must
     * reside in shared memory in this abstraction anyway, this is unique. */
    os__eventAddressToName(m, name);

    /* If CreateEvent succeeds, the return value is a handle to the event object.
     * If the named event object existed before the function call, the function
     * succeeds as well, returns a handle to the existing object and GetLastError
     * returns ERROR_ALREADY_EXISTS. ERROR_ALREADY_EXISTS is ignored, since this
     * is exactly the intended behaviour (i.e., initialize or open on access). */
    event = CreateEvent(&ht.sa, arg->event.bManualReset, arg->event.bInitialState, name);
    /* TODO: Report error ? */

    return event;
}

static
_Ret_maybenull_
HANDLE
os__syncSemaphoreCreate(
    _In_ void *s,
    _In_ os__syncCreateCallbackArg * arg)
{
    char name[] = OS__EVENT_NAME_TEMPLATE;
    HANDLE semaphore;

    assert(ht.saInitialized == TRUE);

    /* Use the address of the semaphore as name. Since a shared mutex must
     * reside in shared memory in this abstraction anyway, this is unique. */
    os__eventAddressToName(s, name);

    /* If CreateSemaphore succeeds, the return value is a handle to the semaphore
     * object. If the named semaphore object existed before the function call,
     * the function succeeds as well, returns a handle to the existing object
     * and GetLastError returns ERROR_ALREADY_EXISTS. ERROR_ALREADY_EXISTS is
     * ignored, since this is exactly the intended behaviour (i.e., initialize
     * or open on access). */
    semaphore = CreateSemaphore(&ht.sa, arg->semaphore.lInitialCount, arg->semaphore.lMaximumCount, name);
    /* TODO: Report error ? */

    return semaphore;
}

_Pre_satisfies_( ht.saInitialized )
_Ret_maybenull_ HANDLE
os__eventHandleRetrieve(
    _In_ void *e)
{
    os__syncCreateCallbackArg arg;

    arg.event.bManualReset = FALSE;
    arg.event.bInitialState = FALSE;

    return os__eventHandleRetrieveWithCallback(e, &os__syncEventCreate, &arg);
}

_Pre_satisfies_( ht.saInitialized )
_Ret_maybenull_ HANDLE
os__semaphoreHandleRetrieve(
    _In_ void* sem)
{
    os__syncCreateCallbackArg arg;

    arg.semaphore.lInitialCount = 0;
    arg.semaphore.lMaximumCount = LONG_MAX;

    return os__eventHandleRetrieveWithCallback(sem, &os__syncSemaphoreCreate, &arg);
}

_Pre_satisfies_( ht.saInitialized )
static void
os__eventHandleDelete(
    _In_ void *e)
{
    os_result r = os_resultSuccess;
    struct os__eventHandleHashEntry *next, *prev;
    unsigned int idx;

#ifndef NDEBUG
    assert(ht.signature == OS__SYNC_HT_SIG);
#endif

    idx = os__eventHash(e);
    assert(0 <= idx && idx < OS__EVENTHANDLETABLE_INIT_SIZE);

    os__privateLockAcquireExclusive(&ht.lock);

    /* Lookup */
    next = &ht.heads[idx];
    prev = NULL;
    while (next && next->m && next->m != e) {
        prev = next;
        next = next->next;
    }

    if (next && next->m) {
        assert(next->m == e);
        if (!CloseHandle(next->h)) {
            //OS_DEBUG_1("os__eventHandleDelete", "Failed to close handle %p", next->h);
        }
        if (prev) {
            prev->next = next->next;
        } else {
            prev = next;
            next = prev->next;
            assert(prev == &ht.heads[idx]);
            *prev = next ? *next : OS_EVENTHANDLEHASHENTRY_INIT;
        }
        os_free(next);
        ht.generation++;
    }

    os__privateLockReleaseExclusive(&ht.lock);

    return;
}
#endif /* OS__SYNC_NOSHARED */
