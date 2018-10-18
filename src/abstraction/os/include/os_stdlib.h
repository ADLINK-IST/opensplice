/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
/****************************************************************
 * Interface definition for standard operating system features  *
 ****************************************************************/

/** \file os_stdlib.h
 *  \brief standard operating system features
 */

#ifndef OS_STDLIB_H
#define OS_STDLIB_H

#include "os_defs.h"
#include "os_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
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
    size_t buffersize);

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
 * @deprecated This function is a thin, bordering on anorexic, wrapper to
 * putenv. putenv behaviour varies from unsafe to leaky across different
 * platforms. Use ::os_setenv instead.
 */
OS_API os_result
os_putenv(
    char *variable_definition);

/** \brief Set an environment variable definition
 * Possible Results:
 * - assertion failure: name or value are null.
 * @param name variable name to be set
 * @param value the value to set it to
 * @return os_resultSuccess if
 *     environment variable is set according the variable_definition or
 * os_resultFail if
 *     environment variable could not be set according the
 *     variable_definition
 */
OS_API os_result
os_setenv(
    const char *name, const char *value);

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
#define OS_BATSUFFIX   OS_OS_BATSUFFIX
#define OS_LIB_LOAD_PATH_VAR OS_OS_LIB_LOAD_PATH_VAR

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

/** \brief Create a path including parent dirs
 *
 * Alternative to os_mkdir that creates all path elements instead
 * of only the top-level directory
 *
 * Preconditions:
 *   None
 *
 * Possible results:
 * - return os_resultSuccess if
 *     requested path is created
 * - return os_resultFail if
 *     requested path could not be created
 *
 * When path creation fails an appropriate error message is reported
 */
OS_API os_result
os_mkpath(
    const os_char *path,
    os_mode_t mode);

/** \brief dirname wrapper
 *
 * because not all operating systems have
 * interfaces to dirname a wrapper is made
 *
 * Precondition:
 *   None
 *
 * Possible results:
 * - return '.' if
 *     path is a null pointer
 * - return char *
 *     to a string that is the parent directory of path
 */
OS_API char * os_dirname_r(char *path);

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

/** \brief index wrapper
 *
 * because not all operating systems have
 * interfaces to index a wrapper is made
 *
 * Precondition:
 *   None
 *
 * Possible results:
 * - return NULL if
 *     char c is not found in string s
 * - return address of first occurance of c in s
 */
OS_API char *
os_index(
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
    const char *s1) __nonnull_all__
                    __attribute_malloc__
                    __attribute_returns_nonnull__
                    __attribute_warn_unused_result__;

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
    const char *ptr,
    size_t maxlen);

/** \brief os_strsep wrapper
 *
 * See strsep()
 */
OS_API char *
os_strsep(
   char **stringp,
   const char *delim);


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

/**
 * \brief Allocates destination buffer in which to store the formatted string
 *
 * Precondition:
 *   None
 * Postcondition:
 *   None
 *
 * Possible results:
 * - os_asprintf allocates space to write the formatted string and returns the
 *   number of bytes written excluding the null byte. If an error occurs, e.g.
 *   memory allocation fails, -1 is returned instead.
 */
OS_API int
os_asprintf(
    char **strp,
    const char *fmt,
    ...);

/** \brief fprintf wrapper with disabled broken pipe signals
 *
 * A fprintf can cause an broken pipe signal that can result in a deadlock
 * if the interrupted thread is holding recourses needed by for example the
 * signal handler thread.
 *
 * Precondition:
 *   None
 * Postcondition:
 *   None
 *
 * Possible results:
 * - return
 *   Upon successful completion will return the number of
 *   bytes written to file
 *   or a negative value if an error occured.
 *   errno will be set in such case
 * - Writes formatted output to file.
 */
int
os_vfprintfnosigpipe(
    FILE *file,
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

/** \brief strtoull wrapper
 *
 * Translate string str to unsigned long long value considering
 * base. If base is 0, base is determined from
 * str ([1-9]+ base = 10, 0x[0-9]+ base = 16,
 * 0X[0-9]+ base = 16, 0[0-9] base = 8).
 *
 * Precondition:
 *   errno is set to 0
 *
 * Possible results:
 * - return 0 and errno == EINVAL in case of conversion error
 * - return OS_ULLONG_MIN and errno == ERANGE
 * - return OS_ULLONG_MAX and errno == ERANGE
 * - return value(str)
 */
OS_API unsigned long long
os_strtoull(
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

/** \brief atoull wrapper
 *
 * Translate string to unsigned long long value considering base 10.
 *
 * Precondition:
 *   errno is set to 0
 *
 * Possible results:
 * - return os_strtoll(str, 10)
 */
OS_API unsigned long long
os_atoull(
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
OS_API os_char *
os_lltostr(
    long long num,
    os_char *str,
    os_size_t len,
    os_char **endptr);

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
OS_API os_char *
os_ulltostr(
    unsigned long long num,
    os_char *str,
    os_size_t len,
    os_char **endptr);

/** \brief strtod wrapper
 *
 * Translate string to double value considering base 10.
 *
 * Normal strtod is locale dependent, meaning that if you
 * provide "2.2" and lc_numeric is ',', then the result
 * would be 2. This function makes sure that both "2.2"
 * (which is mostly used) and "2,2" (which could be provided
 * by applications) are translated to 2.2 when the locale
 * indicates that lc_numeric is ','.
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return value(str)
 */
OS_API double
os_strtod(const char *nptr, char **endptr);

/** \brief strtof wrapper
 *
 * Translate string to float value considering base 10.
 *
 * Normal strtof is locale dependent, meaning that if you
 * provide "2.2" and lc_numeric is ',', then the result
 * would be 2. This function makes sure that both "2.2"
 * (which is mostly used) and "2,2" (which could be provided
 * by applications) are translated to 2.2 when the locale
 * indicates that lc_numeric is ','.
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return value(str)
 */
OS_API float
os_strtof(const char *nptr, char **endptr);

/** \brief os_strtod mirror
 *
 * Translate double to string considering base 10.
 *
 * The function dtostr doesn't exists and can be done by
 *     snprintf((char*)dst, "%f", (double)src).
 * But like strtod, snprint is locale dependent, meaning
 * that if you provide 2.2 and lc_numeric is ',' then the
 * result would be "2,2". This comma can cause problems
 * when serializing data to other nodes with different
 * locales.
 * This function makes sure that 2.2 is translated into
 * "2.2", whatever lc_numeric is set ('.' or ',').
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return value(str)
 */
OS_API int
os_dtostr(double src, char *str, size_t size);

/** \brief os_strtof mirror
 *
 * Translate float to string considering base 10.
 *
 * The function ftostr doesn't exists and can be done by
 *     snprintf((char*)dst, "%f", (float)src).
 * But like strtof, snprint is locale dependent, meaning
 * that if you provide 2.2 and lc_numeric is ',' then the
 * result would be "2,2". This comma can cause problems
 * when serializing data to other nodes with different
 * locales.
 * This function makes sure that 2.2 is translated into
 * "2.2", whatever lc_numeric is set ('.' or ',').
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return value(str)
 */
OS_API int
os_ftostr(float src, char *str, size_t size);

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
    size_t n);

/** \brief strtok_r wrapper
 *
 * Tokenise strings based on delim
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return ptr to next token or NULL if there are no tokens left.
 */
OS_API char *
os_strtok_r(char *str, const char *delim, char **saveptr);

struct os_stat_s {
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
  os_timeW  stat_mtime;
};

OS_API os_result
os_stat(
    const char *path,
    struct os_stat_s *buf);

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
OS_API const char *
os_getTempDir(void);

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
OS_API os_ssize_t
os_write(int fd, const void *buf, size_t count);

/**
 * \brief binary search algorithm on an already sorted list.
 *
 *
 * Precondition:
 *   list must be sorted in ascending order
 *
 * Possible results:
 * - On success, the number the matching item is returned.  When the item
 * does not exist, NULL is returned.
 *
 */
OS_API void *
os_bsearch(const void *key, const void *base, size_t nmemb, size_t size,
    int (*compar) (const void *, const void *));


/**
 * \brief  returns a pseudo-random integer in the range 0 to RAND_MAX inclusive (i.e., the mathematical range [0, RAND_MAX])
 *
 * Precondition:
 *   none
 *
 * Possible results:
 * - return a value between 0 and RAND_MAX
 *
 * Note:
 *   rand should not be used for security related applications, as linear congruential algorithms are too easy to break.
 *   Use a compliant random generator, such as /dev/random or /dev/urandom on Unix-like systemsm and CryptGenRandom on Windows.
 */
OS_API int
os_rand(void);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_STDLIB_H */
