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

/** \file os/AIX5.3/code/os_process.c
 *  \brief AIX process management
 *
 * Implements process management for AIX
 * by including the POSIX implementation but with AIX-specific default values
 */

#include "../posix/code/os_process.c"
#include "os_process_attr.c"
#include <sys/procfs.h>

#define _OS_PROCESS_DEFAULT_NAME_LEN_ (512)
#define _OS_PROCESS_DEFAULT_CMDLINE_LEN_ (64)
#define _OS_PROCESS_PROCFS_PATH_FMT_     "/proc/%d/psinfo"

os_int32
os_procGetProcessName(
    char *procName,
    os_uint procNameSize)
{
    char* process_name = NULL;
    char* procPath = NULL;
    int fileId;
    struct psinfo info;
    os_int32 size = 0;

    if (processName) {
        process_name = os_getenv("SPLICE_PROCNAME");
        processName = (char*) os_malloc(_OS_PROCESS_DEFAULT_NAME_LEN_);
        *processName = "\0";
        if (process_name != NULL) {
           size = snprintf(processName,_OS_PROCESS_DEFAULT_NAME_LEN_,"%s",process_name);
        } else {
           procPath = (char*) os_malloc(_OS_PROCESS_DEFAULT_CMDLINE_LEN_);
           if (procPath) {
               snprintf(procPath,_OS_PROCESS_DEFAULT_NAME_LEN_, _OS_PROCESS_PROCFS_PATH_FMT_, os_procIdSelf());
               if ((fileId = open( procPath, O_RDONLY, 0555 )) >= 0) {
                   if ( read( fileId, & info, sizeof(info) ) >= 0 ) {
                       snprintf(processName,_OS_PROCESS_DEFAULT_NAME_LEN_,"%s",info.pr_fname);
                   }
               }
               os_free(procPath);
               close( fileId );
           }
        }
    }
    size = snprintf(procName, procNameSize, "%s", processName);
    return size;
}

#undef _OS_PROCESS_DEFAULT_NAME_LEN_
#undef _OS_PROCESS_DEFAULT_CMDLINE_LEN_
#undef _OS_PROCESS_PROCFS_PATH_FMT_
