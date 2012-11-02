/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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
#include "code/os__socket.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "code/os_stdlib_getopt.c"
#include "../common/code/os_stdlib_locate.c"

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
    int len;
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
        val = digit - 'a';
        if (val >= base) {
            val = -1;
        }
    } else if (digit >= 'A' && digit <= 'Z') {
        val = digit - 'A';
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
    long long value = 0LL;
    long long sign = 1LL;
    long long radix;
    long long dvalue;

    errno = 0;

    if (endptr) {
        *endptr = (char *)str;
    }
    while (isspace(*str)) {
        str++;
    }
    if (*str == '-') {
        sign = -1LL;
        str++;
    } else if (*str == '+') {
        sign = 1LL;
        str++;
    }
    if (base == 0) {
	/* determine radix from string str */
        if (*str == '\0') {
            errno = EINVAL;
            return value;
        }
        if (*str == '0') {
            str++;
            /* base = 8, 10 or 16 */
            if (*str == '\0') {
                base = 10;
            } else if (*str == 'x' || *str == 'X') {
                base = 16;
                str++;
            } else if (*str >= '0' || *str <= '7') {
                base = 8;
            } else {
                errno = EINVAL;
                return value;
            }
        } else if (*str >= '1' || *str <= '9') {
            base = 10;
        }
    } else if (base < 2) {
        /* invalid radix */
        errno = EINVAL;
        return value;
    } else if (base > 36) {
        /* invalid radix */
        errno = EINVAL;
        return value;
    } else if (base == 16) {
        /* Check if prefixed by 0x or 0X */
        if (*str == '0' && (*(str+1) == 'x' || *(str+1) == 'X')) {
            str++;
            str++;
        }
    }
    radix = (long long)base;
    dvalue = digit_value (*str, base);
    while (dvalue >= 0) {
        value = value * radix + dvalue;
        str++;
        dvalue = digit_value (*str, base);
    }
    if (endptr) {
        *endptr = (char *)str;
    }
    return value*sign;
}

long long
os_atoll(
    const char *str)
{
    return os_strtoll(str, NULL, 10);
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

    if (dir_name == NULL || (strcmp (dir_name, "") == 0)) {
        OS_REPORT(OS_ERROR, "os_getTempDir", 0,
            "Could not retrieve temporary directory path - "
            "neither of environment variables TEMP, TMP, OSPL_TEMP were set");
    }

    return dir_name;
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
        /* Output was truncated, so calculate length of resulting string. The
         * length of the resulting string is calculated by:
         * strlen(<format>) - strlen(<format_specifiers>) + strlen(<formatted args>).
         * This is really hard to do, since the formatted length of the arguments
         * needs to be determined. Therefore it is implemented by generating the
         * string on heap until not truncated anymore and returning that length. */
        char *tmp;
        int newSize = strlen(format) + size;

        /* 256 would probably be a good starting point, unless input is already
         * larger. Will be multiplied by two. */
        newSize = (newSize > 128) ? newSize : 128;

        do{
            newSize *= 2;
            tmp = (char*) os_malloc(newSize);

            if(tmp){
                result = _vsnprintf(tmp, newSize, format, args);
                os_free(tmp); /* Do not set tmp to NULL, since it is used in
                                 loop-condition to see whether the memory-claim
                                 succeeded. */
            } else {
                /* Memory-claim denied, result == -1. */
                OS_REPORT_1 (OS_ERROR, "snprintf", 0,
                            "Memory-claim (heap) of size %d denied",
                            newSize);
            }
        } while(result == -1 && tmp); /* NOTE: *tmp may NOT be read at this point */
    }

    /* Truncation occurred, so we need to guarantee that the string is NULL-
     * terminated. */
    if(result >= size){
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

char * os_strerror(int errnum, char *buf, size_t n)
{
    strerror_s(buf, n, errnum);
    return buf;
}

ssize_t os_write(int fd, const void *buf, size_t count)
{
  return write(fd, buf, count);
}
