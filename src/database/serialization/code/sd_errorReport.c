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
#include "sd_errorReport.h"
#include "sd_misc.h"
#include "os_heap.h"
#include "os_report.h"

sd_errorReport
sd_errorReportNew (
    c_ulong       errorNumber,
    const c_char *message,
    const c_char *location)
{
    sd_errorReport report;
    report = os_malloc(C_SIZEOF(sd_errorReport));
    report->errorNumber = errorNumber;
    report->message     = sd_stringDup(message);
    report->location    = sd_stringDup(location);
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

void
sd_errorReportFlush (
    sd_errorReport report)
{
    if ( report ) {
        if ( report->message ) {
            if ( report->location ) {
                OS_REPORT(OS_ERROR, "sd_errorReportFlush", 0,
                            "%s at %s", report->message, report->location);
            } else {
                OS_REPORT(OS_ERROR, "sd_errorReportFlush", 0,
                            "%s", report->message);
            }
        } else {
            OS_REPORT(OS_ERROR, "sd_errorReportFlush", 0,
                      "Invalid parameter: report->message == NULL");
        }
    } else {
        OS_REPORT(OS_ERROR, "sd_errorReportFlush", 0,
                  "Invalid parameter: report == NULL");
    }
}

