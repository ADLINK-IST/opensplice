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
