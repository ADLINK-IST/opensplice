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

/** \file os/linux/code/os_process.c
 *  \brief Linux process management
 *
 * Implements process management for Linux
 * by including the POSIX implementation
 */

#include "../posix/code/os_process.c"
#include "../posix/code/os_process_attr.c"

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
#define _OS_PROCESS_DEFAULT_CMDLINE_LEN_ (32)
#define _OS_PROCESS_DEFAULT_NAME_LEN_ (512)
#define _OS_PROCESS_PROCFS_PATH_FMT_     "/proc/%i/exe"

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
}

os_int32
os_procGetProcessName(
    char *procName,
    os_uint32 procNameSize)
{
   return ((os_int32)snprintf(procName, procNameSize, "%s", processName));
}

#undef _OS_PROCESS_DEFAULT_CMDLINE_LEN_
#undef _OS_PROCESS_DEFAULT_NAME_LEN_
#undef _OS_PROCESS_PROCFS_PATH_FMT_
