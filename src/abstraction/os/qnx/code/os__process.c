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

/** \file os/qnx/code/os_process.c
 *  \brief QNX process management
 *
 * Implements process management for QNX
 */

#include <spawn.h>

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

os_int32
os_procGetProcessName(
    char *procName,
    os_uint32 procNameSize)
{
    char *process_env_name;
    char *tmpName;
    char *exec;
    int size;

    if (processName == NULL) {
        processName = (char*) os_malloc(_OS_PROCESS_DEFAULT_NAME_LEN_);
        *processName = '\0';
        process_env_name = os_getenv("SPLICE_PROCNAME");
        if (process_env_name != NULL) {
            size = snprintf(processName, _OS_PROCESS_DEFAULT_CMDLINE_LEN_, "%s",process_env_name);
        }
        else {
            size = _OS_PROCESS_DEFAULT_NAME_LEN_;
            tmpName = (char*) os_malloc(size);
            if (tmpName) {
               if (_cmdname (tmpName)) {
                    exec = strrchr(tmpName,'/');
                    if (exec) {
                        /* skip all before the last '/' */
                        exec++;
                        (void)snprintf(processName, size, "%s",exec);
                    } else {
                        (void)snprintf(processName, size, "%s",tmpName);
                    }
                }
                os_free(tmpName);
            }
        }
    }
    size = snprintf(procName, procNameSize, "%s", processName);
    return (os_int32)size;
}

os_result
os_procCreate(
    const char *executable_file,
    const char *name,
    const char *arguments,
    os_procAttr * procAttr,
    os_procId *procId)
{
    os_result rv = os_resultSuccess;

    char *argv[64];
    int argc = 1;
    int go_on = 1;
    int i = 0;
    int sq_open = 0;
    int sq_close = 0;
    int dq_open = 0;
    int dq_close = 0;
    char *argin;
    struct inheritance inherit;
    pid_t id;
    int priority;
    char *currentProc;

    assert(executable_file != NULL);
    assert(name != NULL);
    assert(arguments != NULL);
    assert(procAttr != NULL);
    assert(procId != NULL);

    inherit.flags = SPAWN_EXPLICIT_SCHED;
    if (procAttr->schedClass == OS_SCHED_REALTIME) {
        inherit.policy = SCHED_FIFO;
    } else if (procAttr->schedClass == OS_SCHED_TIMESHARE) {
        inherit.policy = SCHED_OTHER;
    } else if (procAttr->schedClass == OS_SCHED_DEFAULT) {
        inherit.policy = SCHED_OTHER;
    } else {
        OS_REPORT(OS_WARNING, "os_procCreate", 2,
                    "scheduling class outside valid range for executable: %s",
                    name);
        return os_resultInvalid;
    }
    priority = procAttr->schedPriority;
    if (priority < sched_get_priority_min (inherit.policy) ||
        priority > sched_get_priority_max (inherit.policy)) {

        priority = (sched_get_priority_max (inherit.policy) +
                    sched_get_priority_min(inherit.policy)) / 2;
        OS_REPORT(OS_WARNING, "os_procCreate", 2,
            "scheduling priority outside valid range for the policy reverted to valid value (%s)",
            name);
    }
    inherit.param.sched_priority = priority;

    if (access(executable_file, X_OK) != 0) {
        rv = os_resultInvalid;
        OS_REPORT(OS_WARNING, "os_procCreate", 2,
                            "Insufficient rights to execute executable %s",
                            name);
    } else {
        /* first translate the input string into an argv structured list */
        argin = os_malloc(strlen(arguments) + 1);
        os_strcpy(argin, arguments);
        argv[0] = os_malloc(strlen(name) + 1);
        argv[0] = os_strcpy(argv[0], name);
        while (go_on && (unsigned int)argc <= (sizeof(argv)/(sizeof(char *)))) {
            while (argin[i] == ' ' || argin[i] == '\t' ) {
                i++;
            }
            if (argin[i] == '\0' ) {
                break;
            } else if (argin[i] == '\'') {
                if (sq_open == sq_close) {
                    sq_open++;
                    argv[argc] = &argin[i];
                } else {
                    sq_close++;
                }
                i++;
            } else if (argin[i] == '\"') {
                if (dq_open == dq_close) {
                    dq_open++;
                } else {
                    dq_close++;
                }
                i++;
            } else {
                argv[argc] = &argin[i];
                argc++;
                while ((argin[i] != ' ' && argin[i] != '\t') ||
                       (sq_open != sq_close) ||
                       (dq_open != dq_close)) {
                    if (argin[i] == '\0') {
                        go_on = 0;
                        break;
                    } else if (argin[i] == '\'') {
                        sq_close++;
                        if ((sq_open == sq_close) && (dq_open == dq_close)) {
                            argin[i] = '\0';
                        }
                        i++;
                    } else if (argin[i] == '\"') {
                        dq_close++;
                        if ((dq_open == dq_close) && (sq_open == sq_close)) {
                            argin[i] = '\0';
                        }
                        i++;
                    } else {
                        i++;
                    }
                }
                argin[i] = '\0';
                i++;
            }
        }
        argv [argc] = NULL;

        currentProc = getenv ("SPLICE_PROCNAME");
        if (currentProc)
        {
            currentProc = os_strdup (currentProc);
        }
        setenv ("SPLICE_PROCNAME", name, 1);
        id = spawn (executable_file, 0, NULL, &inherit, argv, NULL);
        if (currentProc)
        {
            setenv ("SPLICE_PROCNAME", currentProc, 1);
            os_free (currentProc);
        }
        else
        {
            unsetenv ("SPLICE_PROCNAME");
        }

        if (id == -1)
        {
            OS_REPORT
            (
                OS_WARNING,
                "os_procCreate",
                1,
                "spawn failed with error %d (%s, %s)",
                errno, executable_file, name
            );
            rv = os_resultFail;
        }
        else
        {
            *procId = id;
        }
        os_free(argv[0]);
        os_free(argin);
    }
    return rv;
}

#undef _OS_PROCESS_DEFAULT_CMDLINE_LEN_
#undef _OS_PROCESS_DEFAULT_NAME_LEN_
