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
/****************************************************************
 * Interface definition for standard operating system features  *
 ****************************************************************/

/** \file os_stdlib.h
 *  \brief standard operating system features
 */

#ifndef OS_STDLIB_H
#define OS_STDLIB_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"
#include "os_time.h" /* needed for os_stat */

/* include OS specific header file                              */
#include "include/os_stdlib.h"
#include "os_if.h"

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */
                                                                                                                            
/** \brief Get host or processor name
 *
 * Possible Results:
 * - assertion failure: hostname = NULL
 * - returns os_resultSuccess if
 *     hostname correctly identifies the name of the host
 * - returns os_resultFail if
 *     actual hostname is longer than buffersize
 */
OS_API os_result
os_gethostname(
    char *hostname,
    os_uint32 buffersize);

/** \brief Get environment variable definition
 *
 * Postcondition:
 * - the pointer returned is only to be accessed readonly
 *
 * Possible Results:
 * - assertion failure: variable = NULL
 * - returns pointer to value of variable if
 *     variable is found
 * - returns NULL if
 *     variable is not found
 */
OS_API char *
os_getenv(
    const char *variable);

/** \brief Set environment variable definition
 *
 * Precondition:
 * - variable_definition follows the format "<variable>=<value>"
 *
 * Possible Results:
 * - assertion failure: variable_definition = NULL
 * - returns os_resultSuccess if
 *     environment variable is set according the variable_definition
 * - returns os_resultFail if
 *     environment variable could not be set according the
 *     variable_definition
 */
OS_API os_result
os_putenv(
    char *variable_definition);

/** \brief Get file seperator
 *
 * Possible Results:
 * - "<file-seperator-character>"
 */
OS_API const char *
os_fileSep(void);

#define OS_FILESEPCHAR OS_OS_FILESEPCHAR

/** \brief Get path seperator
 *
 * Possible Results:
 * - "<file-seperator-character>"
 */
OS_API const char *
os_pathSep(void);

#define OS_PATHSEPCHAR OS_OS_PATHSEPCHAR
#define OS_EXESUFFIX   OS_OS_EXESUFFIX

/** \brief Check user's permissions for a file 
 *
 * Precondition:
 * - permission is a mask of:
 *     OS_ROK check for read access
 *     OS_WOK check for write access
 *     OS_XOK check for execution access
 *     OS_FOK check for existence of the file
 *
 * Possible results:
 * - return os_resultSuccess if
 *     requested file access is granted
 * - return os_resultFail if
 *     requested file access is not granted
 */
OS_API os_result
os_access(
    const char *file_path,
    os_int32 permission);

/** \brief Locate an executable file in the path
 *
 * Precondition:
 * - permission is a mask of:
 *     OS_ROK check for read access
 *     OS_WOK check for write access
 *     OS_XOK check for execution access
 *     OS_FOK check for existence of the file
 *
 * Possible results:
 * - return an os_malloc-ed string if a file with
 *     the given permission mask was found in
 *     PATH. The return value contains the full
 *     path name of the file found.
 *     If the name of the file contains an
 *     absolute or relative path, then no
 *     search is done in the PATH.
 * - return NULL if no file was found in PATH
 */
OS_API char *
os_locate(
    const char *name,
    os_int32 permission);

/** \brief mkdir wrapper
 *
 * because operating systems has different
 * interfaces to mkdir a wrapper is made
 *
 * Precondition:
 *   None
 *
 * Possible results:
 * - return 0 if 
 *     requested dir is created
 * - return -1 if
 *     requested dir could not be created
 */
OS_API os_int32
os_mkdir(
    const char *path,
    os_mode_t mode);

/** \brief rindex wrapper
 *
 * because not all operating systems have
 * interfaces to rindex a wrapper is made
 *
 * Precondition:
 *   None
 *
 * Possible results:
 * - return NULL if 
 *     char c is not found in string s
 * - return address of last occurance of c in s
 */
OS_API char *
os_rindex(
    const char *s,
    int c);

/** \brief strdup wrapper
 *
 * because not all operating systems have
 * interfaces to strdup a wrapper is made
 *
 * Precondition:
 *   None
 * Postcondition:
 *   The allocated string must be freed using os_free
 *
 * Possible results:
 * - return NULL if 
 *     all resources are depleted
 * - return duplicate of the string s1 allocated via
 *     os_malloc
 */
OS_API char *
os_strdup(
    const char *s1);

/** \brief strcat wrapper
 *
 * Microsoft generates deprected warnings for strcat,
 * wrapper removes warnings for win32
 *
 * Precondition:
 *   None
 * Postcondition:
 *   None
 *
 * Possible results:
 * - return s1
 * - append a copy of s2 onto end of s1
 *   overwriting the null byte at end of s1.
 */
OS_API char *
os_strcat(
    char *s1,
    const char *s2);

/** \brief stnrcat wrapper
 *
 * Microsoft generates deprected warnings for strncat,
 * wrapper removes warnings for win32
 *
 * Precondition:
 *   None
 * Postcondition:
 *   None
 *
 * Possible results:
 * - return s1
 * - append n bytes of s2 onto end of s1,
 *   overwriting the null byte at end of s1.
 */
OS_API char *
os_strncat(
    char *s1,
    const char *s2,
    size_t n);

/** \brief strcpy wrapper
 *
 * Microsoft generates deprected warnings for strcpy,
 * wrapper removes warnings for win32
 *
 * Precondition:
 *   None
 * Postcondition:
 *   None
 *
 * Possible results:
 * - return s1
 * - copy string s2 to s1
 */
OS_API char *
os_strcpy(
    char *s1,
    const char *s2);

/** \brief strncpy wrapper
 *
 * Microsoft generates deprected warnings for strncpy,
 * wrapper removes warnings for win32
 *
 * Precondition:
 *   None
 * Postcondition:
 *   None
 *
 * Possible results:
 * - return s1
 * - copy num chars from string s2 to s1
 */
OS_API char *
os_strncpy(
    char *s1,
    const char *s2,
    size_t num);

/** \brief os_strnlen wrapper
 *
 * Precondition:
 *   None
 * Postcondition:
 *   None
 *
 * Possible results:
 * - return as strnlen
 */
OS_API size_t 
os_strnlen(
   char *ptr,
   size_t maxlen);

/** \brief sprintf wrapper
 *
 * Microsoft generates deprected warnings for sprintf,
 * wrapper removes warnings for win32
 *
 * Precondition:
 *   None
 * Postcondition:
 *   None
 *
 * Possible results:
 * - return
 *   Upon successful completion will return the number of
 *   bytes written to s, excluding the terminating null byte,
 *   or a negative value if an error occured.
 * - Writes formatted output to s.
 */
OS_API int
os_sprintf(
    char *s,
    const char *format,
    ...);

/** \brief os_vsnprintf wrapper
 *
 * Microsoft generates deprected warnings for vsnprintf,
 * wrapper removes warnings for win32
 *
 * Precondition:
 *   None
 * Postcondition:
 *   None
 *
 * Possible results:
 * - os_vsnprintf() does not write  more than size bytes (including the trailing '\0').
 *   If the output was truncated due to this limit then the return value is the
 *   number of  characters (not including the trailing '\0') which would have been
 *   written to the final string if enough space had been  available.
 *   Thus, a return value of size or more means that the output was truncated.
 */

OS_API int
os_vsnprintf(
   char *str,
   size_t size,
   const char *format,
   va_list args);

/** \brief strtoll wrapper
 *
 * Translate string str to long long value considering base,
 * and sign. If base is 0, base is determined from
 * str ([1-9]+ base = 10, 0x[0-9]+ base = 16,
 * 0X[0-9]+ base = 16, 0[0-9] base = 8).
 *
 * Precondition:
 *   errno is set to 0
 *
 * Possible results:
 * - return 0 and errno == EINVAL in case of conversion error
 * - return OS_LLONG_MIN and errno == ERANGE
 * - return OS_LLONG_MAX and errno == ERANGE
 * - return value(str)
 */
OS_API long long
os_strtoll(
    const char *str,
    char **endptr,
    os_int32 base);

/** \brief atoll wrapper
 *
 * Translate string to long long value considering base 10.
 *
 * Precondition:
 *   errno is set to 0
 *
 * Possible results:
 * - return os_strtoll(str, 10)
 */
OS_API long long
os_atoll(
    const char *str);

/** \brief lltostr wrapper
 *
 * Translate long long value into string representation based
 * on radix 10.
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return 0 and errno == EINVAL in case of conversion error
 * - return value(str)
 */
OS_API char *
os_lltostr(
    long long value,
    char *endptr);

/** \brief ulltostr wrapper
 *
 * Translate unsigned long long value into string representation based
 * on radix 10.
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return 0 and errno == EINVAL in case of conversion error
 * - return value(str)
 */
OS_API char *
os_ulltostr(
    unsigned long long value,
    char *endptr);

/** \brief strcasecm wrapper
 *
 * Compare 2 strings conform strcasecmp
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return 0 and s1 equals s2
 * - return <0 and s1 is less than s2
 * - return >0 and s1 is greater than s2
 */
OS_API int
os_strcasecmp(
    const char *s1,
    const char *s2);

/** \brief strncasecm wrapper
 *
 * Compare 2 strings conform strncasecmp
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return 0 and s1 equals s2 (maximum the first n characters)
 * - return <0 and s1 is less than s2 (maximum the first n characters)
 * - return >0 and s1 is greater than s2 (maximum the first n characters)
 */
OS_API int
os_strncasecmp(
    const char *s1,
    const char *s2,
    os_uint32 n);

struct os_stat {
/* The mode_t macro's (like OS_ISDIR) are defined in the platform specific header files! */
/* NEVER name these fields identical to the POSIX standard! Since e.g. the Linux implementation
   has defined it as follows:
   struct stat {
     ...
       struct timespec st_mtim;
   #define st_mtime st_mtim.tvsec
     ...
   };
   This results in the fact that our definition is also changed, causing compilation errors!
*/
  os_mode_t stat_mode; 
  os_size_t stat_size;
  os_time   stat_mtime;
};

OS_API os_result
os_stat(
    const char *path,
    struct os_stat *buf);

typedef os_os_dirHandle os_dirHandle;

struct os_dirent {
    char d_name[OS_PATH_MAX + 1];
};

/** \brief opendir wrapper
 *
 * Open the directory conform opendir
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return os_resultSuccess if directory 'name' is opened
 * - os_resultFail if the filre 'name' could not 
 *     be found or is not a directory.
 */
OS_API os_result
os_opendir(
    const char *name,
    os_dirHandle *dir);

/** \brief closedir wrapper
 *
 * Close the directory conform closdir
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return os_resultSuccess if directory identified by the handle
 *     is succesfully closed
 * - return os_resultFail if the handle is invalid.
 */
OS_API os_result
os_closedir(
    os_dirHandle d);

/** \brief readdir wrapper
 *
 * Read the directory conform readdir.
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return os_resultSuccess if next directory is found
 * - return os_resultFail if no more directories are found.
 */
OS_API os_result
os_readdir(
    os_dirHandle d,
    struct os_dirent *direntp);

/** \brief Removes the file or directory given by name.
  *
  * This function is equivalent to POSIX remove(3)
  *
  */

OS_API os_result os_remove (const char *name);

/** \brief Renames a file or directory
  *
  * This function is equivalent to POSIX rename(3)
  *
  */

OS_API os_result os_rename (const char *oldpath, const char *newpath);

/** \brief Transforms the given filepath into a platform specific filepath.
 *
 * This translation function will replace any platform file seperator into
 * the fileseperator of the current platform.
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - returns normalized filepath conform current platform
 * - return NULL if out of memory.
 */
OS_API char *
os_fileNormalize(
    const char *filepath);

/**
 * \brief Flushes the internal buffers associated with the file handle to disk
 * 
 * Precondition:
 *   The file should be open.
 * 
 * Possible results:
 * - os_resultSuccess if function succeeded
 * - os_resultFail    if function failed
 */
OS_API os_result
os_fsync(
    FILE *fHandle);

/**
 * \brief returns the location of the temporary files used by OpenSplice.
 * This may be the key file describing the shared memory or the shared
 * file itself depending on the operating system
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - char * of the absolute path of the temporary location.  This will return
 * always return a valid value, using a default if necessary
 */
OS_API char *
os_getTempDir();

/**
 * \brief return string describing error number.
 * 
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - char * of the string description of the errnum passed in
 * if no string is associated then a string containing the errnum is returned
 */
OS_API char *
os_strerror(int errnum, char *buf, size_t n);

/**
 * \brief writes up to count bytes from the buffer pointed buf to the file referred to by the file descriptor fd.
 * 
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - On success, the number of bytes written is returned (zero indicates
 * nothing was written). On error, -1 is returned
 *
 */
OS_API ssize_t 
os_write(int fd, const void *buf, size_t count);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_STDLIB_H */
