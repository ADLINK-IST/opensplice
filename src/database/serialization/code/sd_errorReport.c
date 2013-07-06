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
#include "sd_errorReport.h"
#include "sd_misc.h"
#include "os_heap.h"

sd_errorReport
sd_errorReportNew (
    c_ulong       errorNumber,
    const c_char *message,
    const c_char *location)
{
    sd_errorReport report;

    report = (sd_errorReport) os_malloc(C_SIZEOF(sd_errorReport));
    if ( report ) {
        report->errorNumber = errorNumber;
        report->message     = sd_stringDup(message);
        report->location    = sd_stringDup(location);
    }

    return report;
}

    

void
sd_errorReportFree (
    sd_errorReport report)
{
    if ( report ) {
        if ( report->message ) {
            os_free(report->message);
        }

        if ( report->location ) {
            os_free(report->location);
        }

        os_free(report);
    }
}

