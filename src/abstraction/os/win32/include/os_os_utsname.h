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

#ifndef OS_WIN32_UTSNAME_H
#define OS_WIN32_UTSNAME_H


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

#endif
