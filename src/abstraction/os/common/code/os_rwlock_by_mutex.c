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

/** \file os/common/code/os_rwlock.c
 *  \brief common multiple reader writer lock
 *
 * Implements multiple reader writer lock by
 * means of a mutex
 */

#include "os_rwlock.h"
#include "os_mutex.h"

#include <stdio.h>
#include <assert.h>

/** \brief Initialize the rwlock taking the rwlock attributes into account
 *
 * \b os_rwlockInit intializes the mutex which implements the rwlock
 */
os_result
os_rwlockInit (
    os_rwlock *rwlock,
    const os_rwlockAttr *rwlockAttr)
{
    os_mutexAttr mutexAttr;

    os_mutexAttrInit(&mutexAttr);
    if(rwlockAttr) {
        mutexAttr.scopeAttr = rwlockAttr->scopeAttr;
    }
    return os_mutexInit (rwlock, &mutexAttr);
}

/** \brief Destroy the rwlock
 *
 * \b os_rwlockDestroy destroys the mutex that implements the rwlock
 */
void
os_rwlockDestroy (
    os_rwlock *rwlock)
{
    assert (rwlock != NULL);
    os_mutexDestroy (rwlock);
}

/** \brief Acquire the rwlock while intending to read only
 *
 * \b os_rwlockRead calls \b os_mutexLock to claim the rwlock
 */
void
os_rwlockRead (
    os_rwlock *rwlock)
{
    os_mutexLock (rwlock);
}

/** \brief Acquire the rwlock while intending to write
 *
 * \b os_rwlockWrite calls \b os_mutexLock to claim the rwlock
 */
void
os_rwlockWrite (
    os_rwlock *rwlock)
{
    os_mutexLock (rwlock);
}

/** \brief Try to acquire the rwlock while intending to read only
 *
 * \b os_rwlockTryRead calls \b os_mutexTryLock to claim the rwlock
 */
os_result
os_rwlockTryRead (
    os_rwlock *rwlock)
{
    return os_mutexTryLock (rwlock);
}

/** \brief Try to acquire the rwlock while intending to write
 *
 * \b os_rwlockTryWrite calls \b os_mutexTryLock to claim the rwlock
 */
os_result
os_rwlockTryWrite (
    os_rwlock *rwlock)
{
    return os_mutexTryLock (rwlock);
}

/** \brief Release the acquired rwlock
 *
 * \b os_rwlockUnlock calls \b os_mutexUnlock to release
 * the \b rwlock.
 */
void
os_rwlockUnlock (
    os_rwlock *rwlock)
{
    os_mutexUnlock (rwlock);
}
