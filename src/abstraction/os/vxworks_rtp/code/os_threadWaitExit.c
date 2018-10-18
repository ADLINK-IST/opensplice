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

/** \file os/vxworks6.6/code/os_threadWaitExit.c
 *  \brief VxWorks RTP thread join management
 *
 * Implements pthread_join management for VxWorks RTP
 * varies from posix implementation because of known
 * issue within WindRiver's implementation of phread_join
 * (TSR 815826).  When resolved this should be removed.
 */

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "os_report.h"
#include "os_abstract.h"

/** \brief Wait for the termination of the identified thread
 *
 * \b os_threadWaitExit wait for the termination of the
 * thread \b threadId by calling \b pthread_join. The return
 * value of the thread is passed via \b thread_result.
 */
os_result
os_threadWaitExit (
    os_threadId threadId,
    void **thread_result)
{
    os_result rv;
    int result;
    struct sched_param sched_param;
    int policy = 0;
    int max = 0;

    assert (threadId);

    /* On VxWorks 6.x RTP mode, there is a known issue in the pthread_join system call.
     * From WindRiver : when pthread_join returns, it does not indicate end of a thread 
     * in 100% of the situations.  It could appear that the parent process, which calls 
     * pthread_join has a higher priority than the thread which is currently being terminated. 
     * This means that the function pthread_join would return BEFORE pthread_exit has finished.
     * This is contrary to posix spec which says that pthread_join should only return 
     * when the thread is REALLY terminated, which is not true in vxWorks.
     * This was submitted to WindRiver as TSR 815826.  Workaround suggested by WindRiver
     * Support : should increase priority of the task to be killed before giving the semaphore 
     * back in order to force the exit of the thread before pthread_join exits. 
     */

    /* Note that any possible errors raised here are not terminal since the thread
     * may have exited at this point anyway.  This is a best effort to raise the priority
     * of the thread about to be joined so that exit will only return when the thread 
     * has definitely terminated
     */

    result = pthread_getschedparam(threadId, &policy, &sched_param);
    if (result == 0) {
        max = sched_get_priority_max(policy);
	if (max != -1) {
	    result = pthread_setschedprio(threadId, max);
	}
    }

    result = pthread_join (threadId, thread_result);
    if (result != 0) {
        OS_REPORT (OS_ERROR, "os_threadWaitExit", 2, "pthread_join failed with error %d", result);
        rv = os_resultFail;
    } else {
        rv = os_resultSuccess;
    }
    return rv;
}
