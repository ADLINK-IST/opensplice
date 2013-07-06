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
