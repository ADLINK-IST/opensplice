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
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_errno.h"

#include <stdlib.h>
#include <unistd.h>
#ifndef PIKEOS_POSIX
#include <strings.h>
#endif
#include <string.h>
#include <ctype.h>


#include "os_stdlib_locate.c"
#include "os_stdlib_strsep.c"

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
#ifdef VXWORKS_RTP
    /* The access function is broken for vxworks RTP for some filesystems
       so best ignore the result, and assume the user has correct permissions */
    (void) file_path;
    (void) permission;
    result = os_resultSuccess;
#else
    if (access (file_path, permission) == 0) {
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }
#endif
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
    os_size_t len;
    char *dup;

    len = strlen (s1) + 1;
    dup = os_malloc (len);
    memcpy (dup, s1, len);

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
   int result;

   sigset_t sset_before, sset_omask, sset_pipe, sset_after;

   sigemptyset(&sset_pipe);
   sigaddset(&sset_pipe, SIGPIPE);
   sigpending(&sset_before);
   pthread_sigmask(SIG_BLOCK, &sset_pipe, &sset_omask);
   result = vfprintf(file, format, args);
   sigpending(&sset_after);
   if (!sigismember(&sset_before, SIGPIPE) && sigismember(&sset_after, SIGPIPE)) {
       /* sigtimedwait appears to be fairly well supported, just not by Mac OS. If other platforms prove to be a problem, we can do a proper indication of platform support in the os defs. The advantage of sigtimedwait is that it protects against a deadlock when SIGPIPE is sent from outside the program and all threads have it blocked. In any case, when SIGPIPE is sent in this manner and we consume the signal here, the signal is lost. Nobody should be abusing system-generated signals in this manner. */
#ifndef __APPLE__
       struct timespec timeout = { 0, 0 };
       sigtimedwait(&sset_pipe, NULL, &timeout);
#else
       int sig;
       sigwait(&sset_pipe, &sig);
#endif
#ifndef NDEBUG
       sigpending(&sset_after);
       assert(!sigismember(&sset_after, SIGPIPE));
#endif
       os_setErrno(EPIPE);
       result = -1;
   }
   pthread_sigmask(SIG_SETMASK, &sset_omask, NULL);
   return result;
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
    size_t n)
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
    struct os_stat *buf)
{
    os_result result;
    struct stat _buf;
    int r;

    r = stat(path, &_buf);
    if (r == 0) {
        buf->stat_mode = _buf.st_mode;
        buf->stat_size = (os_size_t) _buf.st_size;
        buf->stat_mtime = OS_TIMEW_INIT(_buf.st_mtime, 0);
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }

    return result;
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
    const char *fpPtr;
    char *normPtr;

    norm = NULL;
    if ((filepath != NULL) && (*filepath != '\0')) {
        norm = os_malloc(strlen(filepath) + 1);
        /* replace any / or \ by OS_FILESEPCHAR */
        fpPtr = (char *) filepath;
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

    if (fsync(fileno(fHandle)) == 0) {
        r = os_resultSuccess;
    } else {
        r = os_resultFail;
    }

    return r;
}

const char *
os_getTempDir()
{
    char * dir_name = NULL;

    dir_name = os_getenv("OSPL_TEMP");

    /* if OSPL_TEMP is not defined the default is /tmp */
    if (dir_name == NULL || (strcmp (dir_name, "") == 0)) {
       dir_name = "/tmp";
    }

    return dir_name;
}

os_ssize_t os_write(int fd, const void *buf, size_t count)
{
    return write(fd, buf, count);
}
