/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef OS_WIN32_STDLIB_H
#define OS_WIN32_STDLIB_H

#if defined (__cplusplus)
extern "C" {
#endif

#define OS_OS_FILESEPCHAR '\\'
#define OS_OS_PATHSEPCHAR ';'
#define OS_OS_EXESUFFIX   ".exe"
#define OS_OS_BATSUFFIX   ".bat"
#define OS_OS_LIB_LOAD_PATH_VAR "PATH"

#define OS_ROK (_S_IREAD)
#define OS_WOK (_S_IWRITE)
#define OS_XOK (_S_IEXEC)
#define OS_FOK (0)

#define OS_ISDIR(mode) (mode & _S_IFDIR)
#define OS_ISREG(mode) (mode & _S_IFREG)
#define OS_ISLNK(mode) (0) /* not supported on this platform */

/* on this platform these permission masks are don't cares! */
#define S_IRWXU 00700
#define S_IRWXG 00070
#define S_IRWXO 00007

/* The value _POSIX_PATH_MAX is defined in limits.h, however you have
 * to define _POSIX_ during compilation.This again will remove the
 * _read, _open and _close prototypes!
 */
#define OS_PATH_MAX 255

typedef HANDLE os_os_dirHandle;

#define MAXHOSTNAMELEN MAX_HOSTNAME_LEN

#if _MSC_VER < 1900
OS_API extern int snprintf(char *s, size_t n, const char *format, ...);
#endif

OS_API extern char *optarg;
OS_API extern int optind, opterr;

OS_API int getopt (int argc, char **argv, const char *options);


#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_STDLIB_H */
