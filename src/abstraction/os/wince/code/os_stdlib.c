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
#include "os_stdlib.h"
#include "os_version.h"
#include "os_heap.h"
#include "os_report.h"
#include "code/os__socket.h"

#include <stdio.h>
#include <string.h>
#include "os_errno.h"
#include <ctype.h>

#include "code/os_stdlib_getopt.c"
#include "../common/code/os_stdlib_locate.c"
#include "code/os_stdlib_bsearch.c"
#include "../common/code/os_stdlib_strtod.c"
#include "../common/code/os_stdlib_strtol.c"
#include "../common/code/os_stdlib_strtok_r.c"
#include "../common/code/os_stdlib_strsep.c"

#define REGISTRY_PATH "SOFTWARE\\PrismTech\\OpenSpliceDDS\\"

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
   wchar_t* wStringPath;

   wStringPath = wce_mbtowc(path);
   if (CreateDirectory(wStringPath, NULL)) {
      result = 0;
   } else {
      result = -1;
   }
   os_free(wStringPath);
   return result;
}

os_result
os_gethostname(
   char *hostname,
   size_t buffersize)
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
         strncpy(hostname, hostnamebuf, (size_t)buffersize);
         result = os_resultSuccess;
      }
   } else {
      result = os_resultFail;
   }
   return result;
}

/* Note the value returned from getRegistryPath must be freed with os_free */
wchar_t *
getRegistryPath(void)
{
   char * registryPath;
   wchar_t* wStringRegistryPath;

   registryPath = os_malloc (strlen(REGISTRY_PATH) + strlen(OSPL_VERSION_STR) + 1);
   strcpy (registryPath, REGISTRY_PATH);
   strcat (registryPath, OSPL_VERSION_STR);

   wStringRegistryPath = wce_mbtowc(registryPath);
   os_free (registryPath);

   return wStringRegistryPath;
}

char *
os_getenv(
   const char *variable)
{
   DWORD ret;
   DWORD dwType = REG_SZ;
   DWORD cbData = 4096;
   wchar_t* PerfData = (wchar_t*)os_malloc (4096);
   HKEY ENVKEY;
   wchar_t* wStringRegistryPath;
   wchar_t* wStringVariable;

   wStringRegistryPath = getRegistryPath();
   RegCreateKeyEx(HKEY_LOCAL_MACHINE, wStringRegistryPath,0, NULL, REG_OPTION_NON_VOLATILE, 0, NULL, &ENVKEY, NULL);
   os_free (wStringRegistryPath);

   wStringVariable = wce_mbtowc(variable);
   ret = RegQueryValueEx(ENVKEY,
                         wStringVariable,
                         NULL,
                         &dwType,
                         (LPBYTE) PerfData,
                         &cbData);
   os_free (wStringVariable);

   if (ret == ERROR_SUCCESS)
   {
      return wce_wctomb(PerfData);
   }

   return NULL;
}

os_result
os_putenv_reg(
   const char *name,
   const char *value)
{
   DWORD ret;
   HKEY ENVKEY;
   wchar_t* wStringRegistryPath;
   wchar_t* wStringName;
   wchar_t* wStringValue;

   wStringRegistryPath = getRegistryPath();
   RegCreateKeyEx(HKEY_LOCAL_MACHINE, wStringRegistryPath,0, NULL, REG_OPTION_NON_VOLATILE, 0, NULL, &ENVKEY, NULL);
   os_free (wStringRegistryPath);

   wStringName = wce_mbtowc(name);
   wStringValue = wce_mbtowc(value);

   ret = RegSetValueEx(ENVKEY,
                       wStringName,
                       0,
                       REG_SZ,
                       (BYTE *)wStringValue,
                       wcslen(wStringValue) * sizeof(wchar_t));

   os_free(wStringName);
   os_free(wStringValue);

   return (ret == 0 ? os_resultSuccess : os_resultFail);
}

os_result
os_putenv(
   char *variable_definition)
{
   os_result result;
   int iSep;
   char * name;
   char * value;

   value = strchr(variable_definition, '=') + 1;
   iSep = value - variable_definition;
   name = (char*)malloc(iSep);
   strncpy(name, variable_definition, iSep);
   *(name + (iSep-1)) = '\0';

   result = os_putenv_reg (name, value);

   os_free(name);

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
   os_result result;
   LPWSTR w_path;
   size_t size;

   WIN32_FIND_DATA FileData;
   HANDLE hFile;

   if(!file_path)
   {
      return os_resultFail;
   }

   size = mbstowcs(NULL, file_path, 0);
   w_path = (wchar_t *)os_malloc( (size + 1) * sizeof( wchar_t ));
   mbstowcs(w_path, file_path, size+1);

   hFile = FindFirstFile(w_path, &FileData);

   if(hFile == INVALID_HANDLE_VALUE)
   {
      result = os_resultFail;
   }
   else
   {
      result = os_resultFail;
      if ((FileData.dwFileAttributes & permission) == permission)
      {
         result = os_resultSuccess;
      }
      FindClose(hFile);
   }

   os_free (w_path);
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
   dup = (char*)os_malloc(len);
   if (dup) {
      strcpy(dup, s1);
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
   struct os_stat_s *buf)
{
   os_result result;

   LPWSTR w_path;
   WIN32_FIND_DATA FileData;
   DWORD64 dw64HighDateTime, dw64LowDateTime;
   DWORD64 dw64LastWriteTime;
   DWORD64 dw64MAXDWORD;
   HANDLE hFile;

   size_t size = mbstowcs(NULL, path, 0);
   w_path = (wchar_t *)os_malloc( (size + 1) * sizeof( wchar_t ));
   mbstowcs(w_path, path, size+1);

   hFile = FindFirstFile(w_path, &FileData);

   if(hFile == INVALID_HANDLE_VALUE)
   {
      result = os_resultFail;
   }
   else
   {
      // Convert to 64 bit for calculations
      dw64HighDateTime = FileData.ftLastWriteTime.dwHighDateTime;
      dw64LowDateTime = FileData.ftLastWriteTime.dwLowDateTime;
      dw64MAXDWORD = MAXDWORD;
      dw64LastWriteTime = (dw64HighDateTime * (dw64MAXDWORD+1)) + dw64LowDateTime;

      buf->stat_mode = FileData.dwFileAttributes;
      buf->stat_size = (FileData.nFileSizeHigh * (dw64MAXDWORD+1)) + FileData.nFileSizeLow;
      buf->stat_mtime = OS_TIMEW_INIT((dw64LastWriteTime - EPOCH_DIFF)/UNITY_DIFF, 0);

      result = os_resultSuccess;

      FindClose(hFile);
   }
   os_free(w_path);
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
      _snprintf((char*)szDir, MAX_PATH + 1, "%s\\*", name);
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
         strcpy(direntp->d_name, (const char*)FileData.cFileName);
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
   os_result result;
   wchar_t* wStringPathName;
   DWORD attrs;

   wStringPathName = wce_mbtowc (pathname);
   attrs = GetFileAttributes (wStringPathName);

   if (attrs == 0xFFFFFFFF)
   {
      result = os_resultFail;
   }
   else
   {
      if (attrs & FILE_ATTRIBUTE_DIRECTORY)
      {
         result = (RemoveDirectory (wStringPathName) != 0) ?
         os_resultSuccess : os_resultFail;
      }
      else
      {
         result = (DeleteFile (wStringPathName) != 0) ?
         os_resultSuccess : os_resultFail;
      }
   }

   os_free(wStringPathName);
   return result;
}

os_result os_rename (const char *oldpath, const char *newpath)
{
   BOOL result;
   wchar_t* wStringOldPath;
   wchar_t* wStringNewPath;

   wStringOldPath = wce_mbtowc (oldpath);
   wStringNewPath = wce_mbtowc (newpath);

   result = MoveFile (wStringOldPath, wStringNewPath);

   os_free(wStringOldPath);
   os_free(wStringNewPath);

   return (result != 0) ? os_resultSuccess : os_resultFail;
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
      norm = (char*)os_malloc(strlen(filepath) + 1);
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


const char *
os_getTempDir()
{
   static char pathname_c[MAX_PATH] = { '\0' };
   if (pathname_c[0] == '\0')
   {
      LPWSTR path_name = (LPWSTR) os_malloc (MAX_PATH);
      GetTempPath(MAX_PATH, path_name);
      WideCharToMultiByte
         (CP_ACP, 0, path_name, -1, pathname_c, MAX_PATH, NULL, NULL);
      os_free (path_name);
   }
   return pathname_c;
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
                OS_REPORT (OS_ERROR, "snprintf", 0,
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

int snprintf(char *s, size_t n, const char *format, ...)
{
   va_list args;
   va_start(args, format);
   return _vsnprintf(s, n, format, args);
}

int vsnprintf(char *s, size_t n, const char *format, va_list args)
{
   return _vsnprintf(s, n, format, args);
}

int os_vfprintfnosigpipe(FILE *file, const char *format, va_list args)
{
   return vfprintf(file, format, args);
}

int isatty(int handle)
{
   return 0;
}

/* Convert (char*) -> (wchar_t*)
 * Note that the result of this function must be freed with os_free
 */
wchar_t* wce_mbtowc(const char* a)
{
   int length;
   wchar_t *wbuf;

   length = MultiByteToWideChar(CP_ACP, 0,
                                a, -1, NULL, 0);
   wbuf = (wchar_t*)os_malloc( (length+1)*sizeof(wchar_t) );
   MultiByteToWideChar(CP_ACP, 0,
                       a, -1, wbuf, length);

   return wbuf;
}

/* Convert (wchar_t*) -> (char*)
 * Note that the result of this function must be freed with os_free
 */
char* wce_wctomb(const wchar_t* w)
{
   DWORD charlength;
   char* pChar;

   charlength = WideCharToMultiByte(CP_ACP, 0, w,
                                    -1, NULL, 0, NULL, NULL);
   pChar = (char*)os_malloc(charlength+1);
   WideCharToMultiByte(CP_ACP, 0, w,
                       -1, pChar, charlength, NULL, NULL);

   return pChar;
}

os_ssize_t os_write(int fd, const void *buf, size_t count)
{
   os_ssize_t ret = 0;
   FILE * stream = _wfdopen(&fd,L"w");
   ret = fwrite(buf, sizeof(buf), count, stream);
   fclose(stream);
   return ret;
}
