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

/** \file os/common/code/os_rwlock.c
 *  \brief common multiple reader writer lock
 *
 * Implements multiple reader writer lock by
 * means of a mutex
 */

#include "os_rwlock.h"

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

    mutexAttr.scopeAttr = rwlockAttr->scopeAttr;
    return os_mutexInit (rwlock, &mutexAttr);
}

/** \brief Destroy the rwlock
 *
 * \b os_rwlockDestroy destroys the mutex that implements the rwlock
 */
os_result
os_rwlockDestroy (
    os_rwlock *rwlock)
{
    assert (rwlock != NULL);
    return os_mutexDestroy (rwlock);
}

/** \brief Acquire the rwlock while intending to read only
 *
 * \b os_rwlockRead calls \b os_mutexLock to claim the rwlock
 */
os_result
os_rwlockRead (
    os_rwlock *rwlock)
{
    return os_mutexLock (rwlock);
}

/** \brief Acquire the rwlock while intending to write
 *
 * \b os_rwlockWrite calls \b os_mutexLock to claim the rwlock
 */
os_result
os_rwlockWrite (
    os_rwlock *rwlock)
{
    return os_mutexLock (rwlock);
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
os_result
os_rwlockUnlock (
    os_rwlock *rwlock)
{
    return os_mutexUnlock (rwlock);
}
