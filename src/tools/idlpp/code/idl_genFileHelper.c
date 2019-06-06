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


#include "idl_genFileHelper.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_string.h"

#include <ctype.h>


static c_bool include_full_filename = FALSE;

void
idl_genSetIncludeGuardMode(
    c_bool full)
{
    include_full_filename = full;
}

c_char *
idl_genIncludeGuardFromFilename(
    idl_scope scope,
    const char *append)
{
    static c_char macro[1024];
    c_char *filename;
    size_t i, j, l;

    if (include_full_filename) {
        filename = idl_scopeFilename(scope);
    } else {
        filename = idl_scopeBasename(scope);
    }

    l = strlen(filename);

    i = j = 0;
    while (i < l) {
        switch (filename[i]) {
        case '.':
            if (i+1 < l && strchr("./\\", filename[i+1]) == NULL) {
                i = l;
            }
            break;
        case '/':
            macro[j++] = '_';
            macro[j] = '\0';
            while (i < l && filename[i+1] == '/') i++;
            break;
        case '\\':
            macro[j++] = '_';
            macro[j] = '\0';
            while (i < l && filename[i+1] == '\\') i++;
            break;
        case '_':
            macro[j++] = '_';
            if (include_full_filename) {
                macro[j++] = '_';
            }
            macro[j] = '\0';
            break;
        default:
            if (strchr("ABCDEFGHIJKLMOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", filename[i]) == NULL) {
                char g[3];
                sprintf(g,"%02X", filename[i]&0xFF);
                macro[j++] = g[0];
                macro[j++] = g[1];
            } else {
                macro[j++] = (c_char) toupper(filename[i]);
            }
            macro[j] = '\0';
            break;
        }
        i++;
    }

    os_strncat(macro, append, sizeof(macro)-strlen(append));

    os_free(filename);

    return macro;
}
