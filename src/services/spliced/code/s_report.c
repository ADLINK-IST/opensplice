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
        fprintf(config->tracingOutputFile, description);
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
    os_time time;

    if (config->tracingOutputFile) {
        if (config->tracingTimestamps == TRUE) {
            time = os_timeGet();

            if (config->tracingRelativeTimestamps == TRUE) {
                time = os_timeSub(time, config->startTime);
            }
            fprintf(config->tracingOutputFile, "%d.%9.9d (%s) -> ",
                    time.tv_sec, time.tv_nsec, threadName);
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
