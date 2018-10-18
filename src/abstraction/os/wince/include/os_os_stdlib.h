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

#define _S_IFMT         0170000         /* file type mask */
#define _S_IFDIR        0040000         /* directory */
#define _S_IFCHR        0020000         /* character special */
#define _S_IFIFO        0010000         /* pipe */
#define _S_IFREG        0100000         /* regular */
#define _S_IREAD        0000400         /* read permission, owner */
#define _S_IWRITE       0000200         /* write permission, owner */
#define _S_IEXEC        0000100         /* execute/search permission, owner */

#define OS_ROK 0x00//(_S_IREAD)
#define OS_WOK 0x00//(_S_IWRITE)
#define OS_XOK 0x00//(_S_IEXEC)
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

/* snprintf is not supported on windows, use _snprintf */
/* The above comment has the ring of truth to it - sm */
OS_API extern int snprintf(char *s, size_t n, const char *format, ...);
OS_API extern int vsnprintf(char *s, size_t n, const char *format, va_list args);

OS_API extern int isatty (int handle);

OS_API extern char *optarg;
OS_API extern int optind, opterr;

OS_API int getopt (int argc, char **argv, const char *options);

/* helper functions to convert between w_char* and char* */
OS_API extern wchar_t* wce_mbtowc(const char* a);
OS_API extern char* wce_wctomb(const wchar_t* w);

/* WinCe doesn't do main(int, char**) */

#define OPENSPLICE_MAIN(n) \
int WINAPI WinMain \
    (HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow) \
{ \
    int argc; \
    char *argv[256]; \
    argc = os_mainparams (lpCmdLine, argv); \
    return main (argc, argv); \
} \
int main (int argc, char *argv[])

OS_API extern int os_mainparams (LPTSTR lpCmdLine, char *argv[]);

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_STDLIB_H */
