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

os_int32
os_procGetProcessName(
    char *procName,
    os_uint32 procNameSize)
{
    char *process_env_name;
    char *tmpName;
    char *procPath;
    char *exec;
    int ret;
    int size;
    int finish = 0;

    if (processName == NULL) {
        processName = (char*) os_malloc(_OS_PROCESS_DEFAULT_NAME_LEN_);
        *processName = '\0';
        process_env_name = os_getenv("SPLICE_PROCNAME");
        if (process_env_name != NULL) {
            size = snprintf(processName, _OS_PROCESS_DEFAULT_CMDLINE_LEN_, "%s",process_env_name);
        }
        else {
            procPath = (char*) os_malloc(_OS_PROCESS_DEFAULT_CMDLINE_LEN_);
            if (procPath) {
                size = snprintf(procPath, _OS_PROCESS_DEFAULT_CMDLINE_LEN_,
                        _OS_PROCESS_PROCFS_PATH_FMT_, os_procIdToInteger(os_procIdSelf()));
                if (size >= _OS_PROCESS_DEFAULT_CMDLINE_LEN_) { /* pid is apparently longer */
                    char *tmp = (char*) os_realloc(procPath, size + 1);
                    if (tmp) {
                        procPath = tmp;
                        size = snprintf(procPath, size + 1, _OS_PROCESS_PROCFS_PATH_FMT_,
                                os_procIdToInteger(os_procIdSelf()));
                    } else {
                        /* Memory-claim failed */
                        size = 0;
                    }
                }
                if (size > 0) {
                    size = _OS_PROCESS_DEFAULT_NAME_LEN_;
                    tmpName = (char*) os_malloc(size);
                    if (tmpName) {
                        while (!finish) {
                            ret = readlink(procPath, tmpName, size);
                            if (ret >= size) {
                                char *tmp;
                                size *= 2;
                                tmp = (char*) os_realloc(tmpName, size +1);
                                if (tmp) {
                                    tmpName = tmp;
                                } else {
                                    /* Memory-claim failed */
                                    finish = 1;
                                }
                            } else {
                                finish = 1;
                            }
                        }
                        if (ret > 0) {
                            tmpName[ret] = 0;
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
                os_free(procPath);
            }
        }
    }
    size = snprintf(procName, procNameSize, "%s", processName);
    return (os_int32)size;
}

#undef _OS_PROCESS_DEFAULT_CMDLINE_LEN_
#undef _OS_PROCESS_DEFAULT_NAME_LEN_
#undef _OS_PROCESS_PROCFS_PATH_FMT_
