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
#ifndef RS_REPORT_H
#define RS_REPORT_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_report.h"
#include "rs_reportMsg.h"
#include "c_typebase.h"

#include <stdarg.h>
#include "os_if.h"

#ifdef OSPL_BUILD_REPORTSERVICE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define rs_report(o) ((rs_report)(o))

C_CLASS(rs_report);

OS_API rs_report rs_reportNew (c_char *domain_name, c_ushort port);

OS_API void rs_reportFree (rs_report domainReport);

OS_API void
rs_reportReport (
    os_IReportService_s domainReport,
    rs_reportType       reportType,
    const c_char       *reportContext,
    const c_char       *fileName,
    c_long              lineNo,
    c_long              reportCode,
    const c_char       *description,
    va_list             args);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* RS_REPORT_H */
