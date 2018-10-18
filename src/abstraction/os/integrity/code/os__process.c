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
/** \file os/int509/code/os_process.c
 *  \brief Posix process management
 *
 */

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

os_int32
os_procGetProcessName(
    char *procName,
    os_uint procNameSize)
{
    os_int32 size = snprintf(procName, procNameSize, "");
    return size;
}

os_result os_procCheckStatus(os_procId procId,
    os_int32 *status)
{
    return os_resultSuccess;
}

os_result os_procDestroy(os_procId procId,
    os_int32 signal)
{
    return os_resultSuccess;
}

os_result
os_procServiceDestroy(
    os_int32 pid,
    os_boolean isblocking,
    os_int32 checkcount)
{
    return os_resultSuccess;
}
