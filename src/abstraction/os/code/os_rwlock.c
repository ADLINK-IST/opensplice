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

/****************************************************************
 * Implementation for multiple reader writer lock conforming    *
 * to OpenSplice requirements                                   *
 ****************************************************************/

/** \file os/code/os_rwlock.c
 *  \brief Critical section management - multiple reader writer lock
 *
 * An rwlock is a mutual exclusion device, and is useful for protecting
 * shared data structures from concurrent modifications, and implementing
 * critical sections and monitors. In contradiction with a mutex, it
 * allows more than one reader to concurrently gain access to the shared
 * data, critical section and monitor.
 *
 * An rwlock has two possible states: unlocked (not owned by a thraed),
 * and locked (owned by one or more reader threads, or one writer thread).
 * An rwlock can never be owned by two different writer threads or two
 * different reader and writer threads simultaneously. A writing thread
 * attempting to lock an rwlock that is already locked by one or more
 * reader threads or another writer thread is suspended until the owning
 * reader threads or writer thread unlocks the rwlock. An reading thread
 * attempting to lock an rwlock that a writer thread is blocking on,
 * is suspended until the writer thread has acquired the rwlock and has
 * unlocked the rwlock again to prevent writer starvation. An rwlock may
 * not be claimed recursively by the same thread not for writing nor for
 * reading. This allows the rwlock to be implemented by means of a mutex
 * if the rwlock is not supported by the specific platform. In this case
 * no multiple reader can own the rwlock.
 */

#include "os_rwlock.h"

/* include OS specific multiple reader writer implementation	*/
#include "code/os_rwlock.c"

