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

#include <stdlib.h>
#include <unistd.h>
#ifndef PIKEOS_POSIX
#include <strings.h>
#endif
#include <string.h>
#include <errno.h>
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
    long long value = 0LL;
    long long sign = 1LL;
    long long radix;
    long long dvalue;

    errno = 0;

    if (endptr) {
        *endptr = (char *)str;
    }
    while (isspace ((int)*str)) {
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

unsigned long long
os_strtoull(
    const char *str,
    char **endptr,
    os_int32 base)
{
    unsigned long long value = 0LL;
    long long sign = 1LL;
    unsigned long long radix;
    long long dvalue;

    errno = 0;

    if (endptr) {
        *endptr = (char *)str;
    }
    while (isspace ((int)*str)) {
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
    radix = (unsigned long long)base;
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
    return os_strtoll (str, NULL, 10);
}

unsigned long long
os_atoull(
    const char *str)
{
    return os_strtoull (str, NULL, 10);
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
    *endptr = '0' + (value - (lval * 10ULL));
    value = lval;
    while (value > 0ULL) {
        lval = value / 10ULL;
        endptr--;
        *endptr = '0' + (value - (lval * 10ULL));
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
    struct stat _buf;
    int r;

    r = stat(path, &_buf);
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

char *
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
