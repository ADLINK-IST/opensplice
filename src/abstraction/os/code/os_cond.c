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
 * Implementation for condition variables conforming to         *
 * OpenSplice requirements                                      *
 ****************************************************************/

/** \file os/code/os_cond.c
 *  \brief Event management - condition variable
 *
 * A condition variable is a synchronisation service that allows threads
 * to suspend execution and relinquish the processors until some predicate
 * on shared data is satisfied.
 * The basic operations are:
 * - signal the condition
 * - wait for the condition
 *
 * Wait suspends the thread execution until another thread signals the condition.
 *
 * A condition variable must always be associated with a mutex, to avoid
 * a race condition where a thread prepares to wait on a condition variable and
 * another thread signals the condition just before the first thread actually
 * waits on it.
 *
 * A condition variable can have a system wide scope in which case it must be
 * defined in system shared memory. In that case it can be used to synchronize
 * processes and threads. A condition variable can also have a process wide
 * scope. The condition variable can then be defined in process shared memory
 * and can be used to synchronize threads.
 *
 * The implementation does not guarantee a process wide scope when requested.
 * This is only a performance optimization which can be ignored by the
 * implementation. The scope will then be system wide.
 */

#include "os_cond.h"

/* include OS specific condition variable implementation 	*/
#include "code/os_cond.c"
