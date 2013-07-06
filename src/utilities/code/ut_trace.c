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
#include "ut__trace.h"

#if UT_TRACE_ENABLED
#include <string.h>
#include "os.h"

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
