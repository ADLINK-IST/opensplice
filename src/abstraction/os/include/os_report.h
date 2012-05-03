/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef OS_REPORT_H
#define OS_REPORT_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"
#include <stdarg.h>
#include "os_if.h"

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* following new line macro can be used in report description to insert
 * a new line and indent to align next line.
 */
#define OS_REPORT_NL "\n              "

#define OS_REPORT(type,context,code,description) \
    os_report(type,context,__FILE__,__LINE__,code,description)

#define OS_REPORT_1(type,context,code,description,a1) \
    os_report(type,context,__FILE__,__LINE__,code,description,a1)

#define OS_REPORT_2(type,context,code,description,a1,a2) \
    os_report(type,context,__FILE__,__LINE__,code,description,a1,a2)

#define OS_REPORT_3(type,context,code,description,a1,a2,a3) \
    os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3)

#define OS_REPORT_4(type,context,code,description,a1,a2,a3,a4) \
    os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4)

#define OS_REPORT_5(type,context,code,description,a1,a2,a3,a4,a5) \
    os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4,a5)

#define OS_REPORT_6(type,context,code,description,a1,a2,a3,a4,a5,a6) \
    os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4,a5,a6)

#define OS_REPORT_7(type,context,code,description,a1,a2,a3,a4,a5,a6,a7) \
    os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4,a5,a6,a7)

#define OS_REPORT_8(type,context,code,description,a1,a2,a3,a4,a5,a6,a7,a8) \
    os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4,a5,a6,a7,a8)

#define OS_REPORT_9(type,context,code,description,a1,a2,a3,a4,a5,a6,a7,a8,a9) \
    os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4,a5,a6,a7,a8,a9)

typedef void * os_IReportService_s;

typedef void * os_reportPlugin;

typedef enum os_reportType {
    OS_INFO,
    OS_WARNING,
    OS_ERROR,
    OS_CRITICAL,
    OS_FATAL,
    OS_REPAIRED,
    OS_API_INFO
} os_reportType;

typedef struct os_reportInfo_s {
    char *reportContext;
    char *sourceLine;
    char *callStack;
    os_int32 reportCode;
    char *description;
} os_reportInfo;

OS_API void
os_report(
    os_reportType reportType,
    const char   *reportContext,
    const char   *fileName,
    os_int32      lineNo,
    os_int32      reportCode,
    const char   *description,
    ...);

OS_API void
os_reportSetApiInfo(
    const char   *reportContext,
    const char   *sourceLine,
    const char   *callStack,
    os_int32      reportCode,
    const char   *description,
    ...);

OS_API void
os_reportSetApiInfoContext(
    const char   *reportContext);

OS_API void
os_reportSetApiInfoLine(
    const char   *sourceLine);

OS_API void
os_reportSetApiInfoStack(
    const char   *callStack);

OS_API void
os_reportSetApiInfoCode(
    os_int32      reportCode);

OS_API void
os_reportSetApiInfoDescription(
    const char   *description,
    ...);

OS_API os_reportInfo *
os_reportGetApiInfo(void);

OS_API void
os_reportClearApiInfo(void);

OS_API os_int32
os_registerReportService (
    os_IReportService_s reportServiceContext,
    void (*reportService)(
    os_IReportService_s reportServiceContext,
    os_reportType reportType,
    const char *reportContext,
    const char *fileName,
    os_int32 lineNo,
    os_int32 code,
    const char *description,
    va_list args));

OS_API os_int32
os_unregisterReportService(
    os_IReportService_s reportServiceContext);

OS_API os_int32
os_reportRegisterPlugin(
    const char *library_file_name,
    const char *initialize_method_name,
    const char *argument,
    const char *report_method_name,
    const char *finalize_method_name,
    os_boolean suppressDefaultLogs,
    os_reportPlugin *plugin);

OS_API os_int32
os_reportUnregisterPlugin(
    os_reportPlugin plugin);

OS_API void
os_reportDisplayLogLocations();




#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_REPORT_H */
