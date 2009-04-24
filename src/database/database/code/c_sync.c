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

#include "c_sync.h"
#include "c_typebase.h"

c_syncResult
c_mutexInit (
    c_mutex *mtx,
    const c_mutexAttr attr)
{
    os_result result;
    os_mutexAttr mutexAttr;

    os_mutexAttrInit (&mutexAttr);
    if (attr == PRIVATE_MUTEX) {
	mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
    }
#ifdef NDEBUG
    result = os_mutexInit(mtx, &mutexAttr);
#else
    mtx->owner = 0;
    result = os_mutexInit(&mtx->mtx, &mutexAttr);
#endif
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_mutexLock (
    c_mutex *mtx)
{
    os_result result;

#ifdef NDEBUG
    result = os_mutexLock(mtx);
#else
    result = os_mutexLock(&mtx->mtx);
    if ( result == os_resultSuccess )
    {
       mtx->owner = os_threadIdSelf();
    }
#endif
    assert(result == os_resultSuccess);
    return result;
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
    if ( result == os_resultSuccess )
    {
       mtx->owner = os_threadIdSelf();
    }
#endif
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_mutexUnlock (
    c_mutex *mtx)
{
    os_result result;
#ifdef NDEBUG
    result = os_mutexUnlock(mtx);
#else
    assert( mtx->owner == os_threadIdSelf() );
    mtx->owner = 0;
    result = os_mutexUnlock(&mtx->mtx);
#endif
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_mutexDestroy (
    c_mutex *mtx)
{
    os_result result;
#ifdef NDEBUG
    result = os_mutexDestroy(mtx);
#else
    do {
        result = os_mutexDestroy(&mtx->mtx);
        if (result != os_resultSuccess) {
            assert(FALSE); /* create core dump */
        }
    } while (result != os_resultSuccess);
#endif
    return result;
}


c_syncResult
c_lockInit (
    c_lock *lck,
    const c_lockAttr attr)
{
    os_result result;

    os_rwlockAttr rwlockAttr;

    os_rwlockAttrInit (&rwlockAttr);
    if (attr == PRIVATE_LOCK) {
	rwlockAttr.scopeAttr = OS_SCOPE_PRIVATE;
    }
#ifdef NDEBUG
    result = os_rwlockInit(lck, &rwlockAttr);
#else
    lck->owner = 0;
    result = os_rwlockInit(&lck->lck, &rwlockAttr);
#endif
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_lockRead (
    c_lock *lck)
{
    os_result result;
#ifdef NDEBUG
    result = os_rwlockRead(lck);
#else
    result = os_rwlockRead(&lck->lck);
#endif
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_lockWrite (
    c_lock *lck)
{
    os_result result;
#ifdef NDEBUG
    result = os_rwlockWrite(lck);
#else
    result = os_rwlockWrite(&lck->lck);
    lck->owner = os_threadIdSelf();
#endif
    assert(result == os_resultSuccess);
    return result;
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
    assert(result == os_resultSuccess);
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
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_lockUnlock (
    c_lock *lck)
{
    os_result result;
#ifdef NDEBUG
    result = os_rwlockUnlock(lck);
#else
    lck->owner = 0;
    result = os_rwlockUnlock(&lck->lck);
#endif
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_lockDestroy (
    c_lock *lck)
{
    os_result result;
#ifdef NDEBUG
    result = os_rwlockDestroy(lck);
#else
    lck->owner = 0;
    result = os_rwlockDestroy(&lck->lck);
#endif
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_condInit (
    c_cond *cnd,
    c_mutex *mtx,
    const c_condAttr attr)
{
    os_result result;

    os_condAttr condAttr;

    os_condAttrInit (&condAttr);
    if (attr == PRIVATE_COND) {
	condAttr.scopeAttr = OS_SCOPE_PRIVATE;
    }
#ifdef NDEBUG
    result = os_condInit(cnd, mtx, &condAttr);
#else
    mtx->owner = 0;
     result = os_condInit(cnd, &mtx->mtx, &condAttr);
#endif
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_condWait (
    c_cond *cnd,
    c_mutex *mtx)
{
    os_result result;

#ifdef NDEBUG
    result = os_condWait(cnd,mtx);
#else
    mtx->owner = 0;
    result = os_condWait(cnd,&mtx->mtx);
    mtx->owner = os_threadIdSelf();
#endif
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_condTimedWait (
    c_cond *cnd,
    c_mutex *mtx,
    const c_time time)
{
    os_result result;
    os_time t;

    t.tv_sec = time.seconds;
    t.tv_nsec = time.nanoseconds;
#ifdef NDEBUG
    result = os_condTimedWait(cnd,mtx,&t);
#else
    mtx->owner = 0;
    result = os_condTimedWait(cnd,&mtx->mtx,&t);
    mtx->owner = os_threadIdSelf();
#endif
    assert((result == os_resultSuccess) || (result == os_resultTimeout));
    return result;
}

c_syncResult
c_condSignal (
    c_cond *cnd)
{
    os_result result;

    result = os_condSignal(cnd);
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_condBroadcast (
    c_cond *cnd)
{
    os_result result;

    result = os_condBroadcast(cnd);
    assert(result == os_resultSuccess);
    return result;
}

c_syncResult
c_condDestroy (
    c_cond *cnd)
{
    os_result result;

    result = os_condDestroy(cnd);
    assert(result == os_resultSuccess);
    return result;
}

#if defined (__cplusplus)
}
#endif
