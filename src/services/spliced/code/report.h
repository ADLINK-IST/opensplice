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
#ifndef REPORT_H
#define REPORT_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "spliced.h"
#include "os_report.h"

#define OSRPT_CNTXT_SPLICED "OpenSplice domain service"

typedef enum s_reportlevel {
    S_RPTLEVEL_FINEST, S_RPTLEVEL_FINER, S_RPTLEVEL_FINE,
    S_RPTLEVEL_CONFIG, S_RPTLEVEL_INFO,
    S_RPTLEVEL_WARNING, S_RPTLEVEL_SEVERE, S_RPTLEVEL_NONE
} s_reportlevel;

void
s_printTimedEvent(
    spliced s,
    s_reportlevel level,
    const char *threadName,
    const char *format,
    ...);

void
s_printEvent(
    spliced s,
    s_reportlevel level,
    const char *format,
    ...);

#if defined (__cplusplus)
}
#endif

#endif /* REPORT_H */
