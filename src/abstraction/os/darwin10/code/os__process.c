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
os_uint32 size = _OS_PROCESS_DEFAULT_NAME_LEN_;

    if (processName == NULL) {
        processName = (char*) os_malloc(_OS_PROCESS_DEFAULT_NAME_LEN_);
        *processName = 0;
        process_env_name = os_getenv("SPLICE_PROCNAME");
        if (process_env_name != NULL) {
            size = snprintf(processName, size, "%s",process_env_name);
        }
        else {
            if (_NSGetExecutablePath(processName, &size) != 0) {
                /* processName is longer than allocated */
                char *tmp = (char*) os_realloc(processName, size + 1);
                if (tmp != NULL) {
                    /* reallocation successful */
                    processName = tmp;
                    if (_NSGetExecutablePath(processName, &size) == 0) {
                        /* path set successful */
                    }
                }
            }
            if (processName) {
                exec = strrchr(processName,'/');
                if (exec) {
                    /* move everything following the last slash forward */
                    memmove (processName, exec+1, strlen (exec+1) + 1);
                }
            }

        }
    }
    size = snprintf(procName, procNameSize, "%s", processName);
    return size;
}
#undef _OS_PROCESS_DEFAULT_NAME_LEN_

