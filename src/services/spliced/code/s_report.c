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
#include "report.h"
#include "s_misc.h"
#include "spliced.h"
#include "s_configuration.h"


static void
s_doPrint(
    s_configuration config,
    const char* format,
    va_list args)
{
    char description[512];

    if (config->tracingOutputFile) {
        os_vsnprintf(description, sizeof(description)-1, format, args);
        description [sizeof(description)-1] = '\0';
        fprintf(config->tracingOutputFile, "%s", description);
        fflush(config->tracingOutputFile);

        if (config->tracingSynchronous) {
            os_fsync(config->tracingOutputFile);
        }
    }
}

static void
s_printState(
    spliced s,
    s_configuration config,
    const char* threadName)
{
    OS_UNUSED_ARG(s);

    if (config->tracingOutputFile) {
        if (config->tracingTimestamps == TRUE) {

            if (config->tracingRelativeTimestamps == TRUE) {
                os_duration diff;
                /* relative timestamps, use the monotonic clock for timestamping log messages */
                diff = os_timeMDiff(os_timeMGet(), config->startTimeMonotonic);
                fprintf(config->tracingOutputFile, "%"PA_PRId64".%09d (%s) -> ",
                        OS_DURATION_GET_SECONDS(diff), OS_DURATION_GET_NANOSECONDS(diff), threadName);
            } else {
                os_timeM time;
                time = os_timeMGet();
                fprintf(config->tracingOutputFile, "%" PA_PRItime " (%s) -> ",
                        OS_TIMEM_PRINT(time), threadName);
            }
        } else {
            fprintf(config->tracingOutputFile, "(%s) -> ", threadName);
        }
    }
}

void
s_printTimedEvent(
    spliced s,
    s_reportlevel level,
    const char *threadName,
    const char *format,
    ...)
{
    va_list args;
    s_configuration config;

    config = splicedGetConfiguration(s);

    if (config) {
        if (((c_ulong)level) >= ((c_ulong)config->tracingVerbosityLevel)) {
            s_printState(s, config, threadName);
            va_start (args, format);
            s_doPrint(config, format, args);
            va_end (args);
        }
    }
}

void
s_printEvent(
    spliced s,
    s_reportlevel level,
    const char *format,
    ...)
{
    va_list args;
    s_configuration config;

    config = splicedGetConfiguration(s);

    if (config) {
        if (((c_ulong)level) >= ((c_ulong)config->tracingVerbosityLevel)) {
            va_start(args, format);
            s_doPrint(config, format, args);
            va_end(args);
        }
    }
}
