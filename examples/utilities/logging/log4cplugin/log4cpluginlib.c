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

#include "log4cpluginlib.h"

#include /* ../log4c-1.2.1/src/. */ "log4c.h"

int log4c_plugin_init(const char *argument, void **context)
{
    return(log4c_init());
}

int log4c_plugin_typedreport(void *context, os_reportEvent report)
{
    int result = -1;
    int log4c_priority;
    log4c_location_info_t log4c_event_location;
    const log4c_category_t * log_category;

    log4c_event_location.loc_file = report->fileName;
    log4c_event_location.loc_line = report->lineNo;
    log4c_event_location.loc_function = report->reportContext;
    log4c_event_location.loc_data = (void*) &report->code;

    switch (report->reportType) {
    case OS_DEBUG:
        log4c_priority = LOG4C_PRIORITY_DEBUG;
    break;
    case OS_INFO:
        log4c_priority = LOG4C_PRIORITY_INFO;
    break;
    case OS_WARNING:
        log4c_priority = LOG4C_PRIORITY_WARN;
    break;
    case OS_ERROR:
    case OS_REPAIRED:
        log4c_priority = LOG4C_PRIORITY_ERROR;
    break;
    case OS_CRITICAL:
        log4c_priority = LOG4C_PRIORITY_CRIT;
    break;
    case OS_FATAL:
        log4c_priority = LOG4C_PRIORITY_FATAL;
    break;
    default:
        log4c_priority = LOG4C_PRIORITY_NOTSET;
    }

    log_category = log4c_category_get("opensplice");

    log4c_category_log_locinfo(log_category,
                               &log4c_event_location,
                               log4c_priority,
                               "Process=%s Thread=%s Desc=%s",
                               report->processDesc,
                               report->threadDesc,
                               report->description
                              );

    return result;
}

int log4c_plugin_shutdown(void *context)
{
    return(log4c_fini());
}
