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

#include "c_sync.h"
#include "c_typebase.h"
#include "os_report.h"
#include "os_time.h"
#include "c__base.h"

c_syncResult
c_mutexInit (
    c_base base,
    c_mutex *mtx)
{
    os_result result;
    os_mutexAttr mutexAttr;

    os_mutexAttrInit (&mutexAttr);
    mutexAttr.scopeAttr = c_baseGetScopeAttr(base);

#ifdef NDEBUG
    result = os_mutexInit(mtx, &mutexAttr);
#else
    mtx->owner = OS_THREAD_ID_NONE;
    result = os_mutexInit(&mtx->mtx, &mutexAttr);
#endif
    if(result != os_resultSuccess) {
        OS_REPORT(OS_ERROR, "c_mutexInit", 0, "os_mutexInit operation failed.");
        assert(result == os_resultSuccess);
    }
    return result;
}

void
c_mutexLock (
    c_mutex *mtx)
{
#ifdef NDEBUG
    os_mutexLock(mtx);
#else
    os_mutexLock(&mtx->mtx);
    mtx->owner = os_threadIdSelf();
#endif
}

c_syncResult
c_mutexTryLock (
    c_mutex *mtx)
{
    os_result result;
#ifdef NDEBUG
    result = os_mutexTryLock(mtx);
#else
    result = os_mutexTryLock(&mtx->mtx);
    if ( result == os_resultSuccess ) {
        mtx->owner = os_threadIdSelf();
    }
#endif
    return result;
}

void
c_mutexUnlock (
    c_mutex *mtx)
{
#ifdef NDEBUG
    os_mutexUnlock(mtx);
#else
    assert( os_threadIdToInteger(mtx->owner) ==
            os_threadIdToInteger(os_threadIdSelf()) );
    mtx->owner = OS_THREAD_ID_NONE;
    os_mutexUnlock(&mtx->mtx);
#endif
}

void
c_mutexDestroy (
    c_mutex *mtx)
{
#ifdef NDEBUG
    os_mutexDestroy(mtx);
#else
    os_mutexDestroy(&mtx->mtx);
#endif
}


c_syncResult
c_lockInit (
    c_base base,
    c_lock *lck)
{
    os_result result;
    os_rwlockAttr rwlockAttr;

    os_rwlockAttrInit (&rwlockAttr);
    rwlockAttr.scopeAttr = c_baseGetScopeAttr(base);

#ifdef NDEBUG
    result = os_rwlockInit(lck, &rwlockAttr);
#else
    lck->owner = OS_THREAD_ID_NONE;
    result = os_rwlockInit(&lck->lck, &rwlockAttr);
#endif
    if(result != os_resultSuccess){
        OS_REPORT(OS_ERROR, "c_lockInit", 0, "os_rwlockInit failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
    return result;
}

void
c_lockRead (
    c_lock *lck)
{
#ifdef NDEBUG
    os_rwlockRead(lck);
#else
    os_rwlockRead(&lck->lck);
#endif
}

void
c_lockWrite (
    c_lock *lck)
{
#ifdef NDEBUG
    os_rwlockWrite(lck);
#else
    os_rwlockWrite(&lck->lck);
    lck->owner = os_threadIdSelf();
#endif
}

c_syncResult
c_lockTryRead (
    c_lock *lck)
{
    os_result result;
#ifdef NDEBUG
    result = os_rwlockTryRead(lck);
#else
    result = os_rwlockTryRead(&lck->lck);
#endif
    return result;
}

c_syncResult
c_lockTryWrite (
    c_lock *lck)
{
    os_result result;
#ifdef NDEBUG
    result = os_rwlockTryWrite(lck);
#else
    result = os_rwlockTryWrite(&lck->lck);
    lck->owner = os_threadIdSelf();
#endif
    return result;
}

void
c_lockUnlock (
    c_lock *lck)
{
#ifdef NDEBUG
    os_rwlockUnlock(lck);
#else
    lck->owner = OS_THREAD_ID_NONE;
    os_rwlockUnlock(&lck->lck);
#endif
}

void
c_lockDestroy (
    c_lock *lck)
{
#ifdef NDEBUG
    os_rwlockDestroy(lck);
#else
    lck->owner = OS_THREAD_ID_NONE;
    os_rwlockDestroy(&lck->lck);
#endif
}

c_syncResult
c_condInit (
    c_base base,
    c_cond *cnd,
    c_mutex *mtx)
{
    os_result result;
    os_condAttr condAttr;

    os_condAttrInit (&condAttr);
    condAttr.scopeAttr = c_baseGetScopeAttr(base);

#ifdef NDEBUG
    result = os_condInit(cnd, mtx, &condAttr);
#else
    mtx->owner = OS_THREAD_ID_NONE;
     result = os_condInit(cnd, &mtx->mtx, &condAttr);
#endif
    if(result != os_resultSuccess){
        OS_REPORT(OS_ERROR, "c_condInit", 0, "os_condInit failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
    return result;
}

void
c_condWait (
    c_cond *cnd,
    c_mutex *mtx)
{
#ifdef NDEBUG
    os_condWait(cnd,mtx);
#else
    assert(os_threadIdToInteger(mtx->owner) == os_threadIdToInteger(os_threadIdSelf()));
    mtx->owner = OS_THREAD_ID_NONE;
    os_condWait(cnd, &mtx->mtx);
    mtx->owner = os_threadIdSelf();
#endif
}

c_syncResult
c_condTimedWait (
    c_cond *cnd,
    c_mutex *mtx,
    const os_duration time)
{
    os_result result;

    if (OS_DURATION_ISINFINITE(time)) {
        c_condWait(cnd, mtx);
        result = os_resultSuccess;
    } else {
#ifdef NDEBUG
        result = os_condTimedWait(cnd,mtx,time);
#else
        assert(os_threadIdToInteger(mtx->owner) == os_threadIdToInteger(os_threadIdSelf()));
        mtx->owner = OS_THREAD_ID_NONE;
        result = os_condTimedWait(cnd,&mtx->mtx,time);
        mtx->owner = os_threadIdSelf();
#endif
    }
    return result;
}

void
c_condSignal (
    c_cond *cnd)
{
    os_condSignal(cnd);
}

void
c_condBroadcast (
    c_cond *cnd)
{
    os_condBroadcast(cnd);
}

void
c_condDestroy (
    c_cond *cnd)
{
    os_condDestroy(cnd);
}

#if defined (__cplusplus)
}
#endif
