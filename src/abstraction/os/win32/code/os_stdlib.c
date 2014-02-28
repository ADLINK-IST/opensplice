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
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_report.h"
#include "os__socket.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "os_stdlib_getopt.c"
#include "../common/code/os_stdlib_locate.c"
#include "../common/code/os_stdlib_bsearch.c"
#include "../common/code/os_stdlib_strtok_r.c"

static os_int32
os_ensurePathExists(
    char* dir_name);

/**
 *  \brief create a directory with default
 *  security descriptor.  The mode parameter
 *  is ignored for this Operating System.
 *
 */
os_int32
os_mkdir(
    const char *path,
    os_mode_t mode)
{
    os_int32 result = 0;

    if (CreateDirectory(path, NULL)) {
        result = 0;
    } else {
        result = -1;
        errno = GetLastError();
    }
    return result;
}

os_result
os_gethostname(
    char *hostname,
    os_uint buffersize)
{
    os_result result;
    char hostnamebuf[MAXHOSTNAMELEN];
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(OS_SOCK_VERSION, OS_SOCK_REVISION);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        OS_REPORT (OS_FATAL, "os_gethostname", 0, "WSAStartup failed, no compatible socket implementation available");
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        return os_resultFail;
    }
    if (gethostname(hostnamebuf, MAXHOSTNAMELEN) == 0) {
        if ((strlen(hostnamebuf)+1) > (size_t)buffersize) {
            result = os_resultFail;
        } else {
            os_strcpy(hostname, hostnamebuf);
            result = os_resultSuccess;
        }
    } else {
        result = os_resultFail;
    }
    return result;
}

#pragma warning( disable : 4996 )
char *
os_getenv(
    const char *variable)
{
   char * result;
   result = getenv(variable);

   return result;
}
#pragma warning( default : 4996 )

os_result
os_putenv(
    char *variable_definition)
{
    os_result result;

    if (_putenv(variable_definition) == 0) {
        result = os_resultSuccess;
    } else {
        result = os_resultFail;
    }
    return result;
}

const char *
os_fileSep(void)
{
    return "\\";
}

const char *
os_pathSep(void)
{
    return ";";
}

os_result
os_access(
    const char *file_path,
    os_int32 permission)
{
    struct _stat statbuf;
    os_result result;

    result = os_resultFail;
    if (file_path) {
       if (_stat(file_path, &statbuf) == 0) {
           if ((statbuf.st_mode & permission) == permission) {
               result = os_resultSuccess;
           }
       }
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
    os_size_t len;
    char *dup;

    len = strlen(s1) + 1;
    dup = os_malloc(len);
    if (dup) {
        os_strcpy(dup, s1);
    }

    return dup;
}

#pragma warning( disable : 4996 )
char *
os_strcat(
    char *s1,
    const char *s2)
{
   return strcat(s1, s2);
}
#pragma warning( default : 4996 )

#pragma warning( disable : 4996 )
char *
os_strncat(
    char *s1,
    const char *s2,
    size_t n)
{
   return strncat(s1, s2, n);
}
#pragma warning( default : 4996 )

char *
os_strcpy(
    char *s1,
    const char *s2)
{
   size_t size = strlen (s2) + 1;

   strcpy_s(s1, size, s2);
   return s1;
}

#pragma warning( disable : 4996 )
char *
os_strncpy(
    char *s1,
    const char *s2,
    size_t num)
{
   strncpy (s1, s2, num);

   return s1;
}
#pragma warning( default : 4996 )

char *
os_strsep (char **str, const char *sep)
{
    char *ret;
    if (**str == '\0')
        return 0;
    ret = *str;
    while (**str && strchr (sep, **str) == 0)
        (*str)++;
    if (**str != '\0')
    {
        **str = '\0';
        (*str)++;
    }
    return ret;
}

#pragma warning( disable : 4996 )
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
#pragma warning( default : 4996 )

static long long
digit_value(
    char digit,
    os_int32 base)
{
    signed char val;

    if (digit >= '0' && digit <= '9') {
        val = digit - '0';
        if (val >= base) {
            val = -1;
        }
    } else if (digit >= 'a' && digit <= 'z') {
        val = digit - 'a' + 10;
        if (val >= base) {
            val = -1;
        }
    } else if (digit >= 'A' && digit <= 'Z') {
        val = digit - 'A' + 10;
        if (val >= base) {
            val = -1;
        }
    } else {
        val = -1;
    }

    return (long long)val;
}

long long
os_strtoll(
    const char *str,
    char **endptr,
    os_int32 base)
{
    return _strtoi64(str, endptr, base);
}

unsigned long long
os_strtoull(
    const char *str,
    char **endptr,
    os_int32 base)
{
    return _strtoui64(str, endptr, base);
}

long long
os_atoll(
    const char *str)
{
    return os_strtoll(str, NULL, 10);
}

unsigned long long
os_atoull(
    const char *str)
{
    return os_strtoull(str, NULL, 10);
}

char *
os_lltostr(
    long long value,
    char *endptr)
{
    long long lval;
    long long sign;

    if (value < 0LL) {
        sign = -1LL;
    } else {
        sign = 1LL;
    }
    lval = value / 10LL;
    if (sign < 0) {
        endptr--;
        *endptr = '0' + (char)((lval * 10LL) - value);
    } else {
        endptr--;
        *endptr = '0' + (char)(value - (lval * 10LL));
    }
    value = lval * sign;
    while (value > 0LL) {
        lval = value / 10LL;
        endptr--;
        *endptr = '0' + (char)(value - (lval * 10LL));
        value = lval;
    }
    if (sign < 0) {
        endptr--;
        *endptr = '-';
    }
    return endptr;
}

char *
os_ulltostr(
    unsigned long long value,
    char *endptr)
{
    long long lval;

    lval = value / 10ULL;
    endptr--;
    *endptr = '0' + (char)(value - (lval * 10ULL));
    value = lval;
    while (value > 0ULL) {
        lval = value / 10ULL;
        endptr--;
        *endptr = '0' + (char)(value - (lval * 10ULL));
        value = lval;
    }
    return endptr;
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
    struct os_stat *buf)
{
    os_result result;
    struct _stat _buf;
    int r;

    r = _stat(path, &_buf);
    if (r == 0) {
        buf->stat_mode = _buf.st_mode;
        buf->stat_size = _buf.st_size;
        buf->stat_mtime.tv_sec = _buf.st_mtime;
        buf->stat_mtime.tv_nsec = 0;
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
    TCHAR szDir[MAX_PATH + 1];
    WIN32_FIND_DATA FileData;
    os_result result;
    HANDLE hList;

    result = os_resultFail;
    if (dir) {
        snprintf(szDir, MAX_PATH, "%s\\*", name);
        hList = FindFirstFile(szDir, &FileData);

        if (hList != INVALID_HANDLE_VALUE) {
            *dir = (os_dirHandle)hList;
            result = os_resultSuccess;
        }
    }

    return result;
}

os_result
os_closedir(
    os_dirHandle d)
{
    FindClose((HANDLE)d);

    return os_resultSuccess;
}

os_result
os_readdir(
    os_dirHandle d,
    struct os_dirent *direntp)
{
    os_result result;
    WIN32_FIND_DATA FileData;
    BOOL r;

    if (direntp) {
        r = FindNextFile((HANDLE)d, &FileData);
        if (r) {
            os_strcpy(direntp->d_name, FileData.cFileName);
            result = os_resultSuccess;
        } else {
            result = os_resultFail;
        }
    } else {
        result = os_resultFail;
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

    if (FlushFileBuffers((HANDLE)fHandle)) {
        r = os_resultSuccess;
    } else {
        r = os_resultFail;
    }
    return r;
}

char *
os_getTempDir()
{
    char * dir_name = NULL;

    dir_name = os_getenv("OSPL_TEMP");

    /* if OSPL_TEMP is not defined use the TEMP variable */
    if (dir_name == NULL || (strcmp (dir_name, "") == 0)) {
        dir_name = os_getenv("TEMP");
    }

    /* if TEMP is not defined use the TMP variable */
    if (dir_name == NULL || (strcmp (dir_name, "") == 0)) {
        dir_name = os_getenv("TMP");
    }

    /* Now we need to verify if we found a temp directory path, and if we did
     * we have to make sure all the (sub)directories in the path exist.
     * This is needed  to prevent any other operations from using the directory
     * path while it doesn't exist and therefore running into errors.
     */
    if (dir_name == NULL || (strcmp (dir_name, "") == 0)) {
        OS_REPORT(OS_ERROR, "os_getTempDir", 0,
            "Could not retrieve temporary directory path - "
            "neither of environment variables TEMP, TMP, OSPL_TEMP were set");
    } else if(os_ensurePathExists(dir_name) != 0)
    {
        OS_REPORT_1(OS_ERROR, "os_getTempDir", 0,
            "Could not ensure all (sub)directories of the temporary directory "OS_REPORT_NL
            "path '%s' exist. "OS_REPORT_NL
            "This has consequences for the ability of OpenSpliceDDS to run "OS_REPORT_NL
            "properly, as the directory path must be accessible to create "OS_REPORT_NL
            "database and key files in. Without this ability OpenSpliceDDS can "OS_REPORT_NL
            "not start."OS_REPORT_NL,
            dir_name);
    }

    return dir_name;
}

os_int32
os_ensurePathExists(
    char* dir_name)
{
    char* tmp;
    char* ptr;
    char ptrTmp;
    struct os_stat statBuf;
    os_result status;
    os_int32 result = 0;
    os_int32 cont = 1;

    if(dir_name)
    {
        tmp = os_strdup(dir_name);

        for (ptr = tmp; cont; ptr++)
        {
            if (*ptr == '\\' || *ptr == '/' || *ptr == '\0')
            {
                ptrTmp = ptr[0];
                ptr[0] = '\0';
                status = os_stat(tmp, &statBuf);

                if (status != os_resultSuccess)
                {
                    os_mkdir(tmp, 0);
                    status = os_stat(tmp, &statBuf);
                }

                if (!OS_ISDIR (statBuf.stat_mode))
                {
                    if ((strlen(tmp) == 2) && (tmp[1] == ':')) {
                        /*This is a device like for instance: 'C:'*/
                    }
                    else
                    {
                        OS_REPORT_3(OS_ERROR, "os_ensurePathExists", 0,
                        "Unable to create directory '%s' within path '%s'. Errorcode: %d",
                        tmp,
                        dir_name,
                        (int)errno);
                        result = -1;
                    }
                }
                ptr[0] = ptrTmp;
            }
            if(*ptr == '\0' || result == -1)
            {
                cont = 0;
            }
        }
        if(tmp)
        {
            os_free(tmp);
        }
    }
    return result;
}

#pragma warning( disable : 4996 )
int
os_vsnprintf(
   char *str,
   size_t size,
   const char *format,
   va_list args)
{
    int result;

    /* Return-values of _vsnprintf don't match the output on posix platforms,
     * so this extra code is needed to bring it in accordance. It is made to
     * behave as follows (copied from printf man-pages):
     * Upon successful return, this function returns the number of characters
     * printed (not including the trailing '\0' used to end output to strings).
     * The function does not write more than size bytes (including the trailing
     * '\0').  If the output was truncated due to this limit then the return
     * value is the number of characters (not including the trailing '\0') which
     * would have been written to the final string if enough space had been
     * available. Thus, a return value of size or more means that the output was
     * truncated. If an output error is encountered, a negative value is
     * returned. */

    result = _vsnprintf(str, size, format, args);

    if(result == -1){
       /* vsnprintf will return the length that would be written for the given
        * formatting arguments if invoked with NULL and 0 as the first two arguments.
        */
        result = _vsnprintf(NULL,0,format,args);
    }

    /* Truncation occurred, so we need to guarantee that the string is NULL-
     * terminated. */
    if((size_t) result >= size){
        str[size - 1] = '\0';
    }
    return result;
}
#pragma warning( default : 4996 )

int
snprintf(
         char *s,
         size_t size,
         const char *format,
         ...)
{
   int result;
   va_list args;

   va_start(args, format);

   result = os_vsnprintf(s, size, format, args);

   va_end(args);

   return result;
}

os_ssize_t os_write(int fd, const void *buf, size_t count)
{
  return write(fd, buf, (int)count);
}
