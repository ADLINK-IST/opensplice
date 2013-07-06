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

/** \file os/solaris/code/os_process.c
 *  \brief Solaris process management
 *
 * Implements process management for Solaris
 * by including the POSIX implementation
 */

#include "../posix/code/os_process.c"
#include "os_process_attr.c"

os_int32
os_procGetProcessName(
    char *procName,
    os_uint procNameSize)
{
    char *process_env_name;
    os_int32 size =0;
    const char *exec = NULL;
    char *tmpName = NULL;

    if (processName == NULL) {
        process_env_name = os_getenv("SPLICE_PROCNAME");
        if (process_env_name != NULL) {
            processName = os_strdup(process_env_name);
        }
        else {
            exec = getexecname();
            tmpName = strrchr(exec,'/');
            if (tmpName) {
                /* skip all before the last '/' */
                exec = tmpName+1;
            }
            processName = os_strdup(exec);
        }
    }
    size = snprintf(procName, procNameSize, "%s", processName);
    return size;

}
