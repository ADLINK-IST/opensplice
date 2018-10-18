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

/** \file os/pikeos3/code/os_process.c
 *  \brief PikeOS/Posix process management
 *
 * Implements process management for PikeOS/Posix
 * by including the POSIX implementation
 */

#include "os_iterator.h"

#include "../posix/code/os_process.c"
#include "../posix/code/os_process_attr.c"

/** \brief Get the process effective scheduling class
 *
 * Possible Results:
 * - process scheduling class is OS_SCHED_REALTIME
 * - process scheduling class is OS_SCHED_TIMESHARE
 * - process scheduling class is OS_SCHED_DEFAULT if
 *   the class effective could not be determined
 */
os_schedClass
os_procAttrGetClass(void)
{
    os_schedClass class;
    int ret;
    int policy;
    struct sched_param dummyparam;

    ret = pthread_getschedparam(pthread_self(), &policy, &dummyparam);
    if ( !ret )
    {
       switch (policy)
       {
          case SCHED_FIFO:
          case SCHED_RR:
             class = OS_SCHED_REALTIME;
             break;
          case SCHED_OTHER:
             class = OS_SCHED_TIMESHARE;
             break;
          default:
             OS_REPORT(OS_WARNING, "os_procAttrGetClass", 1,
                         "sched_getscheduler unexpected return value %d", policy);
             class = OS_SCHED_DEFAULT;
             break;
       }
    }
    else
    {
       OS_REPORT (OS_WARNING, "os_procAttrGetClass", 1,
                    "sched_getscheduler failed with error %d", ret);
       class = OS_SCHED_DEFAULT;
    }
    return class;
}
/** \brief Get the process effective scheduling priority
 *
 * Possible Results:
 * - any platform and scheduling class dependent valid priority
 */
os_int32
os_procAttrGetPriority(void)
{
    struct sched_param param;
    int policy;
    int err;

    param.sched_priority = 0;
    if ( (err = pthread_getschedparam (pthread_self(), &policy, &param)) != 0)
    {
       OS_REPORT (OS_WARNING, "os_procAttrGetPriority", 1,
                    "sched_getparam failed with error %d", err);
    }
    return param.sched_priority;
}

/** \brief Register an process Call back routine
 *
 * \b os_procRegisterCallback registers an process exit
 * handler by nserting \b  function in a list
 * to be called when the process exists.
 * The standard implementation guarantees the
 * required order of execution of the exit handlers.
 */

os_iter  procCallbackList = NULL;

static void
os_procRegisterCallback(
    void (*function)(void))
{
    os_iterInsert(procCallbackList, (void *)function);
    return;
}

/** \brief Register an process exit handler
 *
 * \b os_procAtExit registers an process exit
 * handler by calling \b atexit passing the \b function
 * to be called when the process exists.
 * The standard POSIX implementation guarantees the
 * required order of execution of the exit handlers.
 */
os_result
os_procAtExit(
    void (*function)(void))
{
    assert (function != NULL);

    os_procRegisterCallback(function);

    return os_resultSuccess;
}

/** \brief Returns if threads are terminated by atexit
 *
 * Return values:
 * TRUE - if threads are ungracefully terminated by atexit
 * FALSE - all other situations
 */
os_boolean
os_procAreThreadsTerminatedByAtExit(
    void)
{
    return FALSE;
}

/** \brief Figure out the name of the current process
 *
 * Possible Results:
 * - returns the actual length of procName
 *
 * Postcondition:
 * - \b procName is ""
 *     the process name could not be determined
 * - \b procName is "<process name>"
 *     the process name could be determined
 *
 * \b procName will not be filled beyond the specified \b procNameSize
 */

static char *processName;

void os_procInit()
{
    char *process_env_name;
    pid_t pid;
    static char pidprocname[32];
    process_env_name = os_getenv("SPLICE_PROCNAME");
    if (process_env_name != NULL)
    {
       processName = process_env_name;
    }
    else
    {
       pid=getpid();
       snprintf(pidprocname, 32, "%d", (int)pid );
       /* Pid should never be for the 31 chars, but just in case! */
       pidprocname[31]='\0';
       processName = pidprocname;
    }
    procCallbackList = os_iterNew (NULL);
}

os_int32
os_procGetProcessName(
    char *procName,
    os_uint32 procNameSize)
{
   return ((os_int32)snprintf(procName, procNameSize, "%s", processName));
}

static void
os_procCallbacks(
    void)
{
    void (*function)(void);
    function = (void(*)(void)) os_iterTakeFirst(procCallbackList);
    while (function != (void *)NULL) {
        function();
        function = (void(*)(void)) os_iterTakeFirst(procCallbackList);
    }
}

/** \brief Terminate the process and return the status
 *         the the parent process
 *
 * \b os_procExit terminates the process by calling \b exit.
 */
void
os_procExit(
    os_exitStatus status)
{
    os_procCallbacks();
    exit(status);
}

os_result
os_procCheckStatus(
    os_procId procId,
    os_int32 *status)
{
    return os_resultUnavailable;
}
