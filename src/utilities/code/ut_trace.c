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
#include "ut_trace.h"

#if UT_TRACE_ENABLED
#include <string.h>
#include "vortex_os.h"

#define STDERR_STR "<stderr>"
#define STDOUT_STR "<stdout>"

static FILE *stream = NULL;

int
ut_traceInitialize(
    char *outputPathName)
{
    int result;

    if (strcmp(outputPathName, STDERR_STR) == 0) {
        stream = stderr;
        result = 0;
    } else {
        if (strcmp(outputPathName, STDOUT_STR) == 0) {
            stream = stdout;
            result = 0;
        } else {
            char * filename = os_fileNormalize(outputPathName);
            stream = fopen(filename, "w");
            os_free(filename);
            if (stream) {
                result = 0;
            } else {
                result = 1;
            }
        }
    }

    return result;
}

int
ut_traceFinalize()
{
    int result;

    if ((stream == stderr) || (stream == stdout)) {
        result = 0;
    } else {
        result = fclose(stream);
    }
    return result;
}

FILE *
ut_traceGetStream()
{
    return stream;
}


#else
/* empty module */

typedef int ut_dummy;

#endif /* UT__TRACE_ENABLED */
