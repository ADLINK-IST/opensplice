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
