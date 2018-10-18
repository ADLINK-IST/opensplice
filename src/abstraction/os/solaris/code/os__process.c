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
