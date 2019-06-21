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
#include "ut_md5.h"

#include <ctype.h>


static c_bool add_random_guard = FALSE;

void
idl_genSetIncludeGuardMode(
    c_bool full)
{
    add_random_guard = full;
}

static c_bool
idl_genComputeDigest(
    const char *filename,
    ut_md5_byte_t digest[16])
{
    c_bool result = FALSE;
    struct os_stat_s fStat;
    int f;
    unsigned nRead;
    ut_md5_state_t pms;
    ut_md5_byte_t *buffer;

    if (os_stat(filename, &fStat) != os_resultSuccess) {
        return result;
    }

    buffer = os_malloc(fStat.stat_size+1);
    f = open(filename, O_RDONLY);
    if (f >= 0) {
        nRead = (unsigned)read(f, buffer, fStat.stat_size);
        close(f);
        if (nRead > 0) {
            ut_md5_init(&pms);
            ut_md5_append(&pms, buffer, nRead);
            ut_md5_finish(&pms, digest);
            result = TRUE;
        }
    }

    os_free(buffer);

    return result;
}

c_char *
idl_genIncludeGuardFromScope(
    idl_scope scope,
    const char *append)
{
    static c_char macro[1024];
    c_char *filename;
    size_t i, j, l, a;

    j = 0;
    memset(macro, 0, sizeof(macro));

    if (add_random_guard) {
        ut_md5_byte_t digest[16];
        memset(digest, 0, 16);
        filename = idl_scopeFilename(scope);
        if (idl_genComputeDigest(filename, digest)) {
            macro[j++] = 'H';
            macro[j++] = '_';

            for (i = 0; i< sizeof(digest); i++) {
                sprintf(&macro[j], "%02X", digest[i]);
                j += 2;
            }
            macro[j++] = '_';
        }

        os_free(filename);
    }

    a = strlen(append);
    l = sizeof(macro) - a - j;
    filename = idl_scopeBasename(scope);
    os_strncat(macro, filename, l);
    os_free(filename);

    os_strncat(macro, append, sizeof(macro)-a);

    return macro;
}
