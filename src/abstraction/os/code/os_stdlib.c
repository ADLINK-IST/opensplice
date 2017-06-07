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
#include "os_abstract.h"

#include "code/os_stdlib_defs.c"
#include "code/os_stdlib_mkdir.c"
#include "code/os_stdlib.c"

size_t
os_strnlen(const char *ptr, size_t maxlen)
{
   size_t len;
   for ( len = 0; len < maxlen && ptr[len] != '\0'; len++ )
   {
   }
   return(len);
}

os_result
os_setenv(const char *name, const char *value)
{
    assert(name != NULL);
    assert(value != NULL);

#ifdef WINCE
    return os_putenv_reg(name, value);
#elif _WIN32
    return ((_putenv_s(name, value) == 0) ? os_resultSuccess : os_resultFail);
#elif defined (OS_SOLARIS_DEFS_H) &&  (OS_SOLARIS_VER < 10) || defined ( _WRS_KERNEL )
    {
       /* setenv didn't exist on Solaris until 5.10. Fallback to putenv... */
       char* new_string = os_malloc(strlen(name) + sizeof("=") + strlen(value));
       if (new_string)
       {
           os_sprintf(new_string, "%s%c%s", name, '=', value);
           return os_putenv(new_string);
       }
    }
    return os_resultFail;
#else
    return ((setenv(name, value, 1) == 0) ? os_resultSuccess : os_resultFail);
#endif
}

char *os_dirname_r(char *path)
{
   char *last_slash=NULL;
   char *second_last_slash=NULL;
   char *lpath = path;
   char *dir_name;
   char *ptr;
   os_size_t destsize;

   /* Locate the last and second last slash characters in the path */
   for ( ptr = path; *ptr != '\0'; ptr++ )
   {
      if ( *ptr == '/' )
      {
         second_last_slash = last_slash;
         last_slash = ptr;
      }
   }

   if ( last_slash == NULL )
   {
      /* No slashes in string, -> dirname is . */
      lpath=".";
      destsize=1;
   }
   else
   {
      if ( last_slash == (ptr-1) )
      {
         if ( second_last_slash == NULL)
         {
           if( last_slash == path )
           {
              /* Only slash is at start of path -> dirname is / */
              lpath="/";
              destsize=1;
           }
           else
           {
              /* Only slash is at end of string -> dirname is . */
              lpath=".";
              destsize=1;
           }
         }
         else
         {
            /* Ignore slash at end of path */
            destsize = (os_size_t) (second_last_slash-lpath);
         }
      }
      else
      {
         destsize = (os_size_t) (last_slash-lpath);
      }
   }
   if ( destsize == 0 )
   {
      destsize = 1;
      lpath = "/";
   }
   dir_name = os_malloc( destsize+1 );
   os_strncpy( dir_name, lpath, destsize);
   *(dir_name + destsize) = '\0';
   return(dir_name);
}

os_result
os_mkpath(const os_char *path, os_mode_t mode)
{
    os_char *dir;
    const os_char *strErr;
    os_size_t len, pos;
    struct os_stat sbuf;
    os_result result = os_resultSuccess;

    if (path == NULL) {
        OS_REPORT(OS_ERROR, "os_mkpath", 0,
            "Cannot create NULL path");
        result = os_resultFail;
    } else {
        len = strlen(path);
        /* win32 os_stat doesn't support paths with trailing slash so pretend it's not there */
        if (len && ((path[len - 1] == OS_FILESEPCHAR) || (path[len - 1] == '/'))) {
            len--;
        }
        dir = os_malloc(len + 1);
        if (dir == NULL) {
            OS_REPORT(OS_ERROR, "os_mkpath", 0,
                "Failed to allocate %"PA_PRIuSIZE" bytes for path string",
                len + 1);
            result = os_resultFail;
        } else {
            memset(dir, 0, len + 1);
            for (pos = 0; pos <= len; pos++) {
                if (path[pos] == OS_FILESEPCHAR || path[pos] == '/' || pos == len) {
                    /* Don't attempt to create filesystem root, i.e. '/' or 'C:\'  */
                    if ((pos != 0) && !((pos == 2) && (path[1] == ':'))) {
                        result = os_stat(dir, &sbuf);
                        if (result != os_resultSuccess) {
                            if (os_mkdir(dir, mode) != 0) {
                                strErr = os_strError(os_getErrno());
                                OS_REPORT(OS_ERROR, "os_mkpath", 0,
                                    "Failed to create path '%s': %s", dir, strErr);
                                result = os_resultFail;
                                break;
                            } else {
                                result = os_resultSuccess;
                            }
                        }
                    }
                }
                dir[pos] = path[pos];
            }
            os_free(dir);
        }
    }

    return result;
}
