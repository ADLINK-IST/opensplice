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

#include "c_sync.h"
#include "c_typebase.h"
#include "os_report.h"
#include "os_time.h"

#if 1
/* TODO: Remove temporary workaround to prevent spinning
 * applications and come up with an actual fix.
 */
static void
wait_on_error(
    os_result result)
{
    os_time sleepTime;
    if((result != os_resultSuccess) && (result != os_resultTimeout)) {
        OS_REPORT(OS_ERROR, "c_sync", 0, "c_mutex or c_cond operation failed.");
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 1000000; /* 1 ms */
        os_nanoSleep(sleepTime);
    }
}
#endif

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
    mtx->owner = OS_THREAD_ID_NONE;
    result = os_mutexInit(&mtx->mtx, &mutexAttr);
#endif
    if(result != os_resultSuccess) {
        OS_REPORT(OS_ERROR, "c_mutexInit", 0, "os_mutexInit operation failed.");
        assert(result == os_resultSuccess);
    }
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
#if 1
    /* TODO: Remove temporary workaround to prevent spinning
     * applications and come up with an actual fix.
     */
    wait_on_error(result);
#endif
    if(result != os_resultSuccess) {
        OS_REPORT_1(OS_ERROR, "c_mutexLock", 0, "os_mutexLock failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }

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
    if ((result != os_resultSuccess) && (result != os_resultBusy)) {
        OS_REPORT_1(OS_ERROR, "c_mutexTryLock", 0, "os_mutexTryLock failed; os_result = %d.", result);
        assert((result == os_resultSuccess) || (result == os_resultBusy));
    }
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
    assert( os_threadIdToInteger(mtx->owner) ==
            os_threadIdToInteger(os_threadIdSelf()) );
    mtx->owner = OS_THREAD_ID_NONE;
    result = os_mutexUnlock(&mtx->mtx);
#endif
    if(result != os_resultSuccess){
        OS_REPORT_1(OS_ERROR, "c_mutexUnlock", 0, "os_mutexUnlock failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
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
    lck->owner = OS_THREAD_ID_NONE;
    result = os_rwlockInit(&lck->lck, &rwlockAttr);
#endif
    if(result != os_resultSuccess){
        OS_REPORT_1(OS_ERROR, "c_lockInit", 0, "os_rwlockInit failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
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
#if 1
    /* TODO: Remove temporary workaround to prevent spinning
     * applications and come up with an actual fix.
     */
    wait_on_error(result);
#endif
    if(result != os_resultSuccess){
        OS_REPORT_1(OS_ERROR, "c_lockRead", 0, "os_rwlockRead failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
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
#if 1
    /* TODO: Remove temporary workaround to prevent spinning
     * applications and come up with an actual fix.
     */
    wait_on_error(result);
#endif
    if(result != os_resultSuccess){
        OS_REPORT_1(OS_ERROR, "c_lockWrite", 0, "os_rwlockWrite failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
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
    if ((result != os_resultSuccess) && (result != os_resultBusy)) {
        OS_REPORT_1(OS_ERROR, "c_lockTryRead", 0, "os_rwlockTryRead failed; os_result = %d.", result);
        assert((result == os_resultSuccess) || (result == os_resultBusy));
    }
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
    if ((result != os_resultSuccess) && (result != os_resultBusy)) {
        OS_REPORT_1(OS_ERROR, "c_lockTryWrite", 0, "os_rwlockTryWrite failed; os_result = %d.", result);
        assert((result == os_resultSuccess) || (result == os_resultBusy));
    }
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
    lck->owner = OS_THREAD_ID_NONE;
    result = os_rwlockUnlock(&lck->lck);
#endif
    if(result != os_resultSuccess){
        OS_REPORT_1(OS_ERROR, "c_lockUnlock", 0, "os_rwlockUnlock failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
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
    lck->owner = OS_THREAD_ID_NONE;
    result = os_rwlockDestroy(&lck->lck);
#endif
    if(result != os_resultSuccess){
        OS_REPORT_1(OS_ERROR, "c_lockDestroy", 0, "os_rwlockDestroy failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
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
    mtx->owner = OS_THREAD_ID_NONE;
     result = os_condInit(cnd, &mtx->mtx, &condAttr);
#endif
    if(result != os_resultSuccess){
        OS_REPORT_1(OS_ERROR, "c_condInit", 0, "os_condInit failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
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
    mtx->owner = OS_THREAD_ID_NONE;

    result = os_condWait(cnd,&mtx->mtx);
    mtx->owner = os_threadIdSelf();
#endif
#if 1
    /* TODO: Remove temporary workaround to prevent spinning
     * applications and come up with an actual fix.
     */
    wait_on_error(result);
#endif
    if(result != os_resultSuccess){
        OS_REPORT_1(OS_ERROR, "c_condWait", 0, "os_condWait failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
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
    mtx->owner = OS_THREAD_ID_NONE;
    result = os_condTimedWait(cnd,&mtx->mtx,&t);
    mtx->owner = os_threadIdSelf();
#endif
#if 1
    /* TODO: Remove temporary workaround to prevent spinning
     * applications and come up with an actual fix.
     */
    wait_on_error(result);
#endif
    if((result != os_resultSuccess) && (result != os_resultTimeout)){
        OS_REPORT_1(OS_ERROR, "c_condTimedWait", 0, "os_condTimedWait failed; os_result = %d.", result);
        assert((result == os_resultSuccess) || (result == os_resultTimeout));
    }
    return result;
}

c_syncResult
c_condSignal (
    c_cond *cnd)
{
    os_result result;

    result = os_condSignal(cnd);
    if(result != os_resultSuccess){
        OS_REPORT_1(OS_ERROR, "c_condSignal", 0, "os_condSignal failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
    return result;
}

c_syncResult
c_condBroadcast (
    c_cond *cnd)
{
    os_result result;

    result = os_condBroadcast(cnd);
    if(result != os_resultSuccess){
        OS_REPORT_1(OS_ERROR, "c_condBroadcast", 0, "os_condBroadcast failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
    return result;
}

c_syncResult
c_condDestroy (
    c_cond *cnd)
{
    os_result result;

    result = os_condDestroy(cnd);
    if(result != os_resultSuccess){
        OS_REPORT_1(OS_ERROR, "c_condDestroy", 0, "os_condDestroy failed; os_result = %d.", result);
        assert(result == os_resultSuccess);
    }
    return result;
}

#if defined (__cplusplus)
}
#endif
