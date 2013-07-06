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

/* Include interface definitions */
#include "rs_report.h"
#include "rs_reportHandler.h"
#include "rs_reportMsg.h"

#include "os_time.h"
#include "os_thread.h"
#include "os_process.h"
#include "os_heap.h"
#include "os_stdlib.h"

#include <assert.h>

C_STRUCT(rs_report) {
    c_char *domainName;
    rs_reportHandler reportHandler;
};

static c_char *
c_stringDupHeap (
    c_char *str)
{
    c_char *new_str;

    assert (str);

    new_str = os_malloc (strlen(str)+1);
    os_strcpy (new_str, str);
    return new_str;
}

/* Implement own interfaces */
rs_report
rs_reportNew (
    c_char *domain_name,
    c_ushort port)
{
    rs_report report;

    assert (domain_name);

    report = os_malloc (C_SIZEOF (rs_report));
    report->domainName = c_stringDupHeap (domain_name);
    report->reportHandler = rs_reportHandlerNew(domain_name, port);
    return report;
}

void
rs_reportFree (
    rs_report domainReport)
{
    rs_reportHandlerFree (domainReport->reportHandler);
    os_free (domainReport->domainName);
    os_free (domainReport);
}

void rs_reportReport (
    os_IReportService_s domainReport,
    rs_reportType       reportType,
    const c_char       *reportContext,
    const c_char       *fileName,
    c_long              lineNo,
    c_long              reportCode,
    const c_char       *description,
    va_list             args)
{
    rs_reportMsg reportMsg;
    os_time ostime;
    c_time ctime;
    c_char extended_description[512];
    c_char procIdentity[256];
    c_char threadIdentity[64];
    c_char node[64];
    rs_report Report = (rs_report)domainReport;

    ostime = os_timeGet();
    ctime.seconds = ostime.tv_sec;
    ctime.nanoseconds = ostime.tv_nsec;

    os_procFigureIdentity (procIdentity, sizeof (procIdentity)-1);
    procIdentity [sizeof (procIdentity)-1] = '\0';

    os_threadFigureIdentity (threadIdentity, sizeof (threadIdentity)-1);
    threadIdentity [sizeof (threadIdentity)-1] = '\0';

    if (os_gethostname (node, sizeof(node)) != os_resultSuccess) {
	snprintf (node, sizeof(node), "<Unidentified>");
    }

    os_vsnprintf (extended_description, sizeof(extended_description)-1, description, args);
    extended_description [sizeof(extended_description)-1] = '\0';

    reportMsg = rs_reportMsgNew (
        reportType,
        reportContext,
        fileName,
        lineNo,
        reportCode,
        extended_description,
        Report->domainName,
	node,
        procIdentity,
        threadIdentity,
        ctime);
    rs_reportHandlerReport (Report->reportHandler, reportMsg);
    rs_reportMsgFree (reportMsg);
}
