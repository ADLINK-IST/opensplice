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
 * Implementation for Mutual exclusion semaphores               *
 * conforming to OpenSplice requirements                        *
 ****************************************************************/

/** \file os/code/os_mutex.c
 *  \brief Critical section management - mutual exclusion semaphore
 * 
 * A mutex is a mutual exclusion device, and is useful for protecting
 * shared data structures from concurrent modifications, and implementing
 * critical sections and monitors.
 *
 * A mutex has two possible states: unlocked (not owned by a thread),
 * and locked (owned by one thread). A mutex can never be owned by two
 * different threads simultaneously. A thread attempting to lock a mutex
 * that is already locked by another thread is suspended until the owning
 * thread releases the mutex first. A mutex may not be claimed recursively
 * by the same thread.
 */

#include "os_mutex.h"

/* include OS specific mutual exclusion semaphore implementation */
#include "code/os_mutex.c"
