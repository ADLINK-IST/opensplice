#ifndef OS_POSIX_STDLIB_H
#define OS_POSIX_STDLIB_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef VXWORKS_RTP
#include <pwd.h>
#endif

#define OS_OS_FILESEPCHAR '/'
#define OS_OS_PATHSEPCHAR ':'
#define OS_OS_EXESUFFIX   ""

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
