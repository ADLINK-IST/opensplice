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

#include "log4cpluginlib.h"

#include /* ../log4c-1.2.1/src/. */ "log4c.h"

int log4c_plugin_init(const char *argument, void **context)
{
    return(log4c_init());
}

int log4c_plugin_typedreport(void *context, os_reportEvent report)
{
    int result = -1;

    if (report->version == OS_REPORT_EVENT_V1)
    {
        int log4c_priority;
        os_reportEventV1 reportV1;
        log4c_location_info_t log4c_event_location;
        const log4c_category_t * log_category;

        reportV1 = (os_reportEventV1) report;

        log4c_event_location.loc_file = reportV1->fileName;
        log4c_event_location.loc_line = reportV1->lineNo;
        log4c_event_location.loc_function = reportV1->reportContext;
        log4c_event_location.loc_data = (void*) &reportV1->code;

        switch (reportV1->reportType)
        {
            case OS_DEBUG:
                log4c_priority = LOG4C_PRIORITY_DEBUG;
                break;
            case OS_INFO:
                log4c_priority = LOG4C_PRIORITY_INFO;
                break;
            case OS_WARNING:
                log4c_priority = LOG4C_PRIORITY_WARN;
                break;
            case OS_API_INFO:
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
                                   reportV1->processDesc,
                                   reportV1->threadDesc,
                                   reportV1->description
                                  );
    }

    return result;
}

int log4c_plugin_shutdown(void *context)
{
    return(log4c_fini());
}
