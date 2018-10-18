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
#if ! defined (OSPL_VXWORKS653)
#include <netdb.h>
#endif
#include <ioLib.h>

#include "os_stdlib.h"
#include "os_heap.h"

#include <envLib.h>
#include <stdlib.h>
#include <unistd.h>
#if ! defined (OSPL_VXWORKS653) && defined ( VXWORKS_GTE_6 )
#include <strings.h>
#endif
#include <string.h>
#include "os_errno.h"
#include <ctype.h>
#if !defined (OSPL_VXWORKS653)
#include <hostLib.h>
#endif

#include "../common/code/os_stdlib_locate.c"
#include "../common/code/os_stdlib_bsearch.c"
#include "../common/code/os_stdlib_strsep.c"
#include "../common/code/os_stdlib_strtod.c"
#include "../common/code/os_stdlib_strtol.c"
#include "../common/code/os_stdlib_strtok_r.c"

os_result
os_gethostname(
    char *hostname,
    os_uint32 buffersize)
{
    os_result result;
#if ! defined (OSPL_VXWORKS653)
    char hostnamebuf[MAXHOSTNAMELEN];

    if (gethostname (hostnamebuf, MAXHOSTNAMELEN) == 0) {
        if ((strlen(hostnamebuf)+1) > (size_t)buffersize) {
            result = os_resultFail;
        } else {
            os_strncpy (hostname, hostnamebuf, (size_t)buffersize);
            result = os_resultSuccess;
        }
    } else {
        result = os_resultFail;
    }
#else
    strncpy (hostname, "HostUnkown", (size_t)buffersize);
    result = os_resultSuccess;
#endif

    return result;
}

char *
os_getenv(
    const char *variable)
{
    return getenv(variable);
}

os_result
os_putenv(
    char *variable_definition)
{
    os_result result;

    if (putenv (variable_definition) == 0) {
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }
    return result;
}

const char *
os_fileSep(void)
{
    return "/";
}

const char *
os_pathSep(void)
{
    return ":";
}

os_result
os_access(
    const char *file_path,
    os_int32 permission)
{
    os_result result;

    if (access (file_path, permission) == 0) {
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }
    return result;
}

char *
os_rindex(
    const char *s,
    int c)
{
    char *last = NULL;

    while (*s) {
        if (*s == c) {
            last = (char *)s;
        }
        s++;
    }
    return last;
}

char *
os_strdup(
    const char *s1)
{
    int len;
    char *dup;

    len = strlen (s1) + 1;
    dup = os_malloc (len);
    if (dup) {
        os_strcpy (dup, s1);
    }

    return dup;
}

char *
os_strcat(
    char *s1,
    const char *s2)
{
   return strcat(s1, s2);
}

char *
os_strncat(
    char *s1,
    const char *s2,
    size_t n)
{
   return strncat(s1, s2, n);
}

char *
os_strcpy(
    char *s1,
    const char *s2)
{
   return strcpy(s1, s2);
}

char *
os_strncpy(
    char *s1,
    const char *s2,
    size_t num)
{
   return strncpy(s1, s2, num);
}

int
os_sprintf(
    char *s,
    const char *format,
    ...)
{
   int result;
   va_list args;

   va_start(args, format);

   result = vsprintf(s, format, args);

   va_end(args);

   return result;
}

int
os_vsnprintf(
   char *str,
   size_t size,
   const char *format,
   va_list args)
{
   return vsnprintf(str, size, format, args);
}

int
os_vfprintfnosigpipe(
        FILE *file,
        const char *format,
        va_list args)
{
   return vfprintf(file, format, args);
}

int
os_strcasecmp(
    const char *s1,
    const char *s2)
{
    int cr;

    while (*s1 && *s2) {
        cr = tolower(*s1) - tolower(*s2);
        if (cr) {
            return cr;
        }
        s1++;
        s2++;
    }
    cr = tolower(*s1) - tolower(*s2);
    return cr;
}

int
os_strncasecmp(
    const char *s1,
    const char *s2,
    os_uint32 n)
{
    int cr = 0;

    while (*s1 && *s2 && n) {
        cr = tolower(*s1) - tolower(*s2);
        if (cr) {
            return cr;
        }
        s1++;
        s2++;
        n--;
    }
    if (n) {
        cr = tolower(*s1) - tolower(*s2);
    }
    return cr;
}

os_result
os_stat(
    const char *path,
    struct os_stat_s *buf)
{
    os_result result;
    struct stat _buf;
    int r;

    r = stat(path, &_buf);
    if (r == 0) {
        buf->stat_mode = _buf.st_mode;
        buf->stat_size = _buf.st_size;
        buf->stat_mtime = OS_TIMEW_INIT(_buf.st_mtime, 0);
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }

    return result;
}

extern char *os_environ[];
void os_stdlibInitialize()
{
   char **varset;

   for ( varset = &os_environ[0]; *varset != NULL; varset++ )
   {
      char *savePtr=NULL;
      char *varName;
      char *tmp = os_strdup( *varset );
      varName = strtok_r( tmp, "=", &savePtr );
      if ( os_getenv( varName ) == NULL )
      {
         os_putenv( *varset );
      }
      os_free(tmp);
   }
}

os_result
os_opendir(
    const char *name,
    os_dirHandle *dir)
{
    os_result result;
    DIR *d;

    result = os_resultFail;
    if (dir) {
        d = opendir(name);
        if (d) {
            *dir = (os_dirHandle)d;
            result = os_resultSuccess;
        }
    }

    return result;
}

os_result
os_closedir(
    os_dirHandle d)
{
    os_result result;
    DIR *dp = (DIR *)d;

    result = os_resultFail;
    if (dp) {
        if (closedir(dp) == 0) {
            result = os_resultSuccess;
        }
    }

    return result;
}

os_result
os_readdir(
    os_dirHandle d,
    struct os_dirent *direntp)
{
    os_result result;
    DIR *dp = (DIR *)d;
    struct dirent *d_entry;

    result = os_resultFail;
    if (dp && direntp) {
        d_entry = readdir(dp);
        if (d_entry) {
            os_strcpy(direntp->d_name, d_entry->d_name);
            result = os_resultSuccess;
        }
    }

    return result;
}

os_result os_remove (const char *pathname)
{
    return (remove (pathname) == 0) ? os_resultSuccess : os_resultFail;
}

os_result os_rename (const char *oldpath, const char *newpath)
{
    return (rename (oldpath, newpath) == 0) ? os_resultSuccess : os_resultFail;
}

/* The result of os_fileNormalize should be freed with os_free */
char *
os_fileNormalize(
    const char *filepath)
{
    char *norm;
    char *fpPtr;
    char *normPtr;

    norm = NULL;
    if ((filepath != NULL) && (*filepath != '\0')) {
        norm = os_malloc(strlen(filepath) + 1);
        /* replace any / or \ by OS_FILESEPCHAR */
        fpPtr = filepath;
        normPtr = norm;
        while (*fpPtr != '\0') {
            *normPtr = *fpPtr;
            if ((*fpPtr == '/') || (*fpPtr == '\\')) {
                *normPtr = OS_FILESEPCHAR;
                normPtr++;
            } else {
                if (*fpPtr != '\"') {
                    normPtr++;
                }
            }
            fpPtr++;
        }
        *normPtr = '\0';
    }

    return norm;
}

os_result
os_fsync(
    FILE *fHandle)
{
    os_result r;

    if (ioctl(fileno(fHandle), FIOSYNC, 0) == 0) {
        r = os_resultSuccess;
    } else {
        r = os_resultFail;
    }

    return r;
}

os_ssize_t os_write(int fd, const void *buf, size_t count)
{
  return write(fd, buf, count);
}
