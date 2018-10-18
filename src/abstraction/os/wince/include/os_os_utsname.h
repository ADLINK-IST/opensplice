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

#ifndef OS_WINCE_UTSNAME_H
#define OS_WINCE_UTSNAME_H


#define OS_OS_SYS_NMLN       257     /* 4.0 size of utsname elements */
                                     /* Must be at least 257 to      */
                                     /* support Internet hostnames.  */

typedef struct os_os_utsname {
        char    sysname[OS_OS_SYS_NMLN];
        char    nodename[OS_OS_SYS_NMLN];
        char    release[OS_OS_SYS_NMLN];
        char    version[OS_OS_SYS_NMLN];
        char    machine[OS_OS_SYS_NMLN];
} os_os_utsname;

#undef OS_OS_SYS_NMLN

#endif /* OS_WINCE_UTSNAME_H */
