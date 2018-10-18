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
#ifndef OS_POSIX_STDLIB_H
#define OS_POSIX_STDLIB_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef OSPL_VERSION
 #ifdef PIKEOS_POSIX
  #include <lwip/netdb.h>
 #else
  #if ! defined (OSPL_VXWORKS653)
   #include <netdb.h>
  #endif
  #if ! defined (VXWORKS_RTP) && ! defined (VXWORKS_55) && ! defined (OSPL_VXWORKS653)
   #ifndef _WRS_KERNEL
    #include <pwd.h>
   #endif
  #endif
 #endif
#endif

#if defined (__cplusplus)
extern "C" {
#endif

#define OS_OS_FILESEPCHAR '/'
#define OS_OS_PATHSEPCHAR ':'
#define OS_OS_EXESUFFIX   ""
#define OS_OS_BATSUFFIX   ""
#define OS_OS_LIB_LOAD_PATH_VAR "LD_LIBRARY_PATH"

#define OS_ROK	R_OK
#define OS_WOK	W_OK
#define OS_XOK	X_OK
#define OS_FOK	F_OK

#define OS_ISDIR S_ISDIR
#define OS_ISREG S_ISREG
#define OS_ISLNK S_ISLNK

#define OS_PATH_MAX _POSIX_PATH_MAX

typedef DIR *os_os_dirHandle;

#if defined (__cplusplus)
}
#endif

#endif /* OS_POSIX_STDLIB_H */
