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
#include "rs_reportMsg.h"

#include "os_heap.h"
#include "os_stdlib.h"

static const char *reportTypeText [] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "API_INFO",
    "ERROR",
    "CRITICAL ERROR",
    "FATAL ERROR",
    "REPAIRED"
};

C_STRUCT(rs_reportMsg) {
    rs_reportType reportType;
    c_char       *reportContext;
    c_char       *fileName;
    c_long        lineNo;
    c_long        reportCode;
    c_char       *description;
    c_char       *domain;
    c_char       *node;
    c_char       *process;
    c_char       *thread;
    c_time        dateTime;
};

static c_string
c_stringDupHeap (
    const c_char *str)
{
    c_char *new_str;

    assert (str);

    new_str = os_malloc (strlen(str)+1);
    os_strcpy (new_str, str);
    return new_str;
}

rs_reportMsg
rs_reportMsgNew (
    const rs_reportType reportType,
    const c_char *reportContext,
    const c_char *fileName,
    c_long lineNo,
    c_long reportCode,
    const c_char *description,
    const c_char *domain,
    const c_char *node,
    const c_char *process,
    const c_char *thread,
    c_time dateTime)
{
    rs_reportMsg reportMsg = rs_reportMsg(os_malloc(C_SIZEOF(rs_reportMsg)));

    memset (reportMsg, 0, C_SIZEOF(rs_reportMsg));
    rs_reportMsgSetReportType    (reportMsg, reportType);
    rs_reportMsgSetReportContext (reportMsg, reportContext);
    rs_reportMsgSetFileName      (reportMsg, fileName);
    rs_reportMsgSetLineNo        (reportMsg, lineNo);
    rs_reportMsgSetReportCode    (reportMsg, reportCode);
    rs_reportMsgSetDescription   (reportMsg, description);
    rs_reportMsgSetDomain	 (reportMsg, domain);
    rs_reportMsgSetNode		 (reportMsg, node);
    rs_reportMsgSetProcess       (reportMsg, process);
    rs_reportMsgSetThread        (reportMsg, thread);
    rs_reportMsgSetDateTime      (reportMsg, dateTime);
    return reportMsg;
}

void
rs_reportMsgFree (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    os_free (reportMsg->reportContext);
    os_free (reportMsg->fileName);
    os_free (reportMsg->description);
    os_free (reportMsg->domain);
    os_free (reportMsg->node);
    os_free (reportMsg->process);
    os_free (reportMsg->thread);
    os_free (reportMsg);
}


#define MAX_REPORT_SIZE (2000)

void
rs_reportMsgReport (
    rs_reportMsg reportMsg)
{
    char buf[MAX_REPORT_SIZE];
    int written;
    assert (reportMsg);

    written = snprintf (buf, MAX_REPORT_SIZE,
	"### Report Message ###\n"
	"Type        : %s\n"
	"Context     : %s\n"
	"File        : %s\n"
	"Line        : %d\n"
	"Code        : %d\n"
	"Description : %s\n"
	"Domain      : %s\n"
	"Node        : %s\n"
	"Process     : %s\n"
	"Thread      : %s\n"
	"Timestamp   : %d.%9.9d\n",
	reportTypeText[reportMsg->reportType],
	reportMsg->reportContext,
	reportMsg->fileName,
	reportMsg->lineNo,
	reportMsg->reportCode,
	reportMsg->description,
	reportMsg->domain,
	reportMsg->node,
	reportMsg->process,
	reportMsg->thread,
	reportMsg->dateTime.seconds,
	reportMsg->dateTime.nanoseconds);

    os_write(fileno(stdout), buf, written);
}

rs_reportType
rs_reportMsgGetReportType (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    return reportMsg->reportType;
}

c_char *
rs_reportMsgGetReportContext (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    return reportMsg->reportContext;
}

c_char *
rs_reportMsgGetFileName (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    return reportMsg->fileName;
}

c_long
rs_reportMsgGetLineNo (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    return reportMsg->lineNo;
}

c_long
rs_reportMsgGetReportCode (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    return reportMsg->reportCode;
}

c_char *
rs_reportMsgGetDescription (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    return reportMsg->description;
}

c_char *
rs_reportMsgGetDomain (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    return reportMsg->domain;
}

c_char *
rs_reportMsgGetNode (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    return reportMsg->node;
}

c_char *
rs_reportMsgGetProcess (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    return reportMsg->process;
}

c_char *
rs_reportMsgGetThread (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    return reportMsg->thread;
}

c_time
rs_reportMsgGetDateTime (
    rs_reportMsg reportMsg)
{
    assert (reportMsg);

    return reportMsg->dateTime;
}

void
rs_reportMsgSetReportType (
    rs_reportMsg reportMsg,
    const rs_reportType reportType)
{
    assert (reportMsg);

    reportMsg->reportType = reportType;
}

void
rs_reportMsgSetReportContext (
    rs_reportMsg reportMsg,
    const c_char *reportContext)
{
    assert (reportMsg);
    assert (reportContext);

    if (reportMsg->reportContext) {
	os_free (reportMsg->reportContext);
    }
    reportMsg->reportContext = c_stringDupHeap (reportContext);
}

void
rs_reportMsgSetFileName (
    rs_reportMsg reportMsg,
    const c_char *fileName)
{
    assert (reportMsg);
    assert (fileName);

    if (reportMsg->fileName) {
	os_free (reportMsg->fileName);
    }
    reportMsg->fileName = c_stringDupHeap (fileName);
}

void
rs_reportMsgSetLineNo (
    rs_reportMsg reportMsg,
    c_long lineNo)
{
    assert (reportMsg);

    reportMsg->lineNo = lineNo;
}

void
rs_reportMsgSetReportCode (
    rs_reportMsg reportMsg,
    c_long reportCode)
{
    assert (reportMsg);

    reportMsg->reportCode = reportCode;
}

void
rs_reportMsgSetDescription (
    rs_reportMsg reportMsg,
    const c_char *description)
{
    assert (reportMsg);
    assert (description);

    if (reportMsg->description) {
	os_free (reportMsg->description);
    }
    reportMsg->description = c_stringDupHeap (description);
}

void
rs_reportMsgSetDomain (
    rs_reportMsg reportMsg,
    const c_char *domain)
{
    assert (reportMsg);
    assert (domain);

    if (reportMsg->domain) {
	os_free (reportMsg->domain);
    }
    reportMsg->domain = c_stringDupHeap (domain);
}

void
rs_reportMsgSetNode (
    rs_reportMsg reportMsg,
    const c_char *node)
{
    assert (reportMsg);
    assert (node);

    if (reportMsg->node) {
	os_free (reportMsg->node);
    }
    reportMsg->node = c_stringDupHeap (node);
}

void
rs_reportMsgSetProcess (
    rs_reportMsg reportMsg,
    const c_char *process)
{
    assert (reportMsg);
    assert (process);

    if (reportMsg->process) {
	os_free (reportMsg->process);
    }
    reportMsg->process = c_stringDupHeap (process);
}

void
rs_reportMsgSetThread (
    rs_reportMsg reportMsg,
    const c_char *thread)
{
    assert (reportMsg);
    assert (thread);

    if (reportMsg->thread) {
	os_free (reportMsg->thread);
    }
    reportMsg->thread = c_stringDupHeap (thread);
}

void
rs_reportMsgSetDateTime (
    rs_reportMsg reportMsg,
    c_time dateTime)
{
    assert (reportMsg);

    reportMsg->dateTime = dateTime;
}
