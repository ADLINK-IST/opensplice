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

/** \file
 *  \brief Darwin process management
 *
 * Implements process management for Darwin
 * by including the POSIX implementation
 */

#include <unistd.h>
#include <mach-o/dyld.h>
struct sched_param;
int sched_setscheduler (pid_t pid, int sched, const struct sched_param *p);
int sched_getscheduler (pid_t pid);
int sched_getparam (pid_t pid, struct sched_param *p);

#include <../posix/code/os_process.c>
#include <../posix/code/os_process_attr.c>
#include <pthread.h>

int sched_setscheduler (pid_t pid __attribute__ ((unused)), int sched __attribute__ ((unused)), const struct sched_param *p __attribute__ ((unused)))
{
  return 0;
}

int sched_getscheduler (pid_t pid __attribute__ ((unused)))
{
  int policy;
  struct sched_param p;
  if (pthread_getschedparam (pthread_self (), &policy, &p) == 0)
    return policy;
  else
    return SCHED_OTHER;
}

int sched_getparam (pid_t pid __attribute__ ((unused)), struct sched_param *p)
{
  int policy;
  return pthread_getschedparam (pthread_self (), &policy, p);
}
#define _OS_PROCESS_DEFAULT_NAME_LEN_ (512)
os_int32
os_procGetProcessName(
    char *procName,
    os_uint procNameSize)
{
    char* process_env_name;
    char* exec;

    if (processName == NULL) {
        processName = os_malloc(_OS_PROCESS_DEFAULT_NAME_LEN_);
        *processName = 0;
        process_env_name = os_getenv("SPLICE_PROCNAME");
        if (process_env_name != NULL) {
            (void) snprintf(processName, _OS_PROCESS_DEFAULT_NAME_LEN_, "%s", process_env_name);
        } else {
            uint32_t usize = _OS_PROCESS_DEFAULT_NAME_LEN_;
            if (_NSGetExecutablePath(processName, &usize) != 0) {
                /* processName is longer than allocated */
                processName = os_realloc(processName, usize + 1);
                if (_NSGetExecutablePath(processName, &usize) == 0) {
                    /* path set successful */
                }
            }
            exec = strrchr(processName,'/');
            if (exec) {
                /* move everything following the last slash forward */
                memmove (processName, exec+1, strlen (exec+1) + 1);
            }
        }
    }
    return snprintf(procName, procNameSize, "%s", processName);
}
#undef _OS_PROCESS_DEFAULT_NAME_LEN_

