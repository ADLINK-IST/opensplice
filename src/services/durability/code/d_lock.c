/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "d__lock.h"
#include "d__thread.h"
#include "d__misc.h"
#include "d_object.h"
#include "os_heap.h"
#include "os_report.h"

void
d_lockInit(
    d_lock lock,
    d_kind kind,
    d_objectDeinitFunc deinit)
{
    if (lock) {
        /* Call super-init */
        d_objectInit(d_object(lock), kind,
                (d_objectDeinitFunc)deinit);
        /* Initialize the lock */
        /* At the moment the return value of os_mutexInit is unchecked
         * and hence assumed to succeed always. If this assumption
         * is dropped then d_lockInit must return a proper error code
         * to the caller indicating that the d_lockInit has failed.
         * The caller must then take appropiate action. */
        (void)os_mutexInit(&lock->lock, NULL);
#ifndef NDEBUG
        lock->tid = 0U;
#endif
    }
}


void
d_lockDeinit(
    d_lock lock)
{
    assert(d_lockIsValid(lock));

    /* Deinitialize the lock */
    os_mutexDestroy(&(lock->lock));
    /* Call super-deinit */
    d_objectDeinit(d_object(lock));
}


void
d_lockFree(
    d_lock lock)
{
    assert(d_lockIsValid(lock));

    d_objectFree(d_object(lock));
}


void
d_lockLock(
    d_lock lock)
{
    assert(d_lockIsValid(lock));

    os_mutexLock(&lock->lock);
#ifndef NDEBUG
    lock->tid = os_threadIdToInteger(os_threadIdSelf());
#endif
}


void
d_lockUnlock(
    d_lock lock)
{
    assert(d_lockIsValid(lock));

#ifndef NDEBUG
    lock->tid = 0U;
#endif
    os_mutexUnlock(&lock->lock);
}
