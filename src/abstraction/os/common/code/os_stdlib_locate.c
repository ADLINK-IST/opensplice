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
#include <assert.h>

#define PATH_ENVVAR "PATH"

/* helper class std_splitList
 *   Splits a string into an object that contains
 *   a list of strings. */

typedef struct std_splitList_s {
    char **strings;
    unsigned size;
} *std_splitList;


static std_splitList
std_splitListNew(
    const char *str,
    char separator)
{
    std_splitList result = NULL;
    unsigned count;
    unsigned i;
    char *copy;
    char *ptr;
    int inString;
    char *tail;

    if (str) {
        /* First count the number of items that will
         * have to be allocated */
        copy = os_strdup(str);
        ptr = copy;
        inString = 0;
        count = 0;
        while (*ptr) {
            if (*ptr == separator) {
                if (inString) {
                    inString = 0;
                }
                *ptr = '\0';
            } else {
            	if (!inString) {
            		count++;
            	}
                inString = !0;
            }
            ptr = &ptr[1];
        }
        /* For confidence checking only */
        OS_UNUSED_ARG(tail);
        tail = ptr;
        (void) tail;

        if (count > 0) {
            result = (std_splitList)os_malloc(sizeof(*result));
            result->strings = (char **)os_malloc(count*sizeof(*result->strings));
            result->size = count;
            i = 0;
            ptr = copy;
            while (i<count) {
                while (*ptr == '\0') {
                    ptr = &ptr[1];
                    assert(ptr != tail);
                }
                result->strings[i] = os_strdup(ptr);
                i++;
                while (*ptr != '\0') {
                    ptr = &ptr[1];

                    if(i<count){
                        assert(ptr != tail);
                    }
                }
            }
            assert(*ptr == '\0');
        }
        os_free(copy);
    }
    return result;
}

static void
std_splitListFree(
    std_splitList list)
{
    unsigned i;

    if (list) {
        for (i=0; i<list->size; i++) {
            os_free(list->strings[i]);
        }
        os_free(list->strings);
        os_free(list);
    }
}

static unsigned
std_splitListSize(
    std_splitList list)
{
    unsigned result = 0;

    if (list) {
        result = list->size;
    }

    return result;
}

static char *
std_splitListGet(
    std_splitList list,
    unsigned index)
{
    char *result = NULL;

    if (list) {
        if (index < list->size) {
            result = list->strings[index];
        }
    }
    return result;
}


/* os_locate itself */

char *
os_locate(
    const char *name,
    os_int32 permission)
{
    char *result = NULL;
    const char *fsep;
    char *path;
    char *fullName;
    os_result osr;
    std_splitList dirs;
    unsigned dirsSize;
    char *curDir;
    unsigned i;

    if (name) {
        fsep = os_fileSep();
        /* If the command contains an absolute or relative path,
         * only check the permissions, otherwise search the file
         * in the PATH environment */

        if ((*name == '.') || (strchr(name, *fsep) != NULL)) {
            osr = os_access(name, permission);
            if (osr == os_resultSuccess) {
                result = os_strdup(name);
            }
        } else {
            /* No relative path in name, so walk over the
             * whole path */
            path = os_getenv(PATH_ENVVAR);
            dirs = std_splitListNew(path, OS_PATHSEPCHAR);
            dirsSize = std_splitListSize(dirs);
            for (i=0; (i<dirsSize) && !result; i++) {
                curDir = std_splitListGet(dirs, i);
                fullName = (char *)os_malloc(
                   strlen(curDir) + strlen(fsep) + strlen(name) + 1);
                if (fullName) {
                    os_strcpy(fullName, curDir);
                    os_strcat(fullName, fsep);
                    os_strcat(fullName, name);
                    /* Check file permissions. Do not have to check if file
                     * exists, since permission check fails when the file
                     * does not exist. */
                   osr = os_access(fullName, permission);
                   if (osr == os_resultSuccess) {
                       result = fullName;
                   } else {
                       os_free(fullName);
                   }
                }
            }
            std_splitListFree(dirs);
        }
    }
    return result;
}
