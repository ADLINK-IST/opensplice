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
#ifndef OS_WIN32_STDLIB_H
#define OS_WIN32_STDLIB_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h> /* for gethostname */
#include <iptypes.h>  /* needed for FIXED_INFO struct */
#include <stdio.h>
#include "io.h"
//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define OS_OS_FILESEPCHAR '\\'
#define OS_OS_PATHSEPCHAR ';'
#define OS_OS_EXESUFFIX   ".exe"
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

OS_API extern int snprintf(char *s, size_t n, const char *format, ...);

OS_API extern char *optarg;
OS_API extern int optind, opterr;

OS_API int getopt (int argc, char **argv, const char *options);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_STDLIB_H */
