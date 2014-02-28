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
#ifndef OS_REPORT_H
#define OS_REPORT_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"
#include <stdarg.h>
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* following new line macro can be used in report description to insert
 * a new line and indent to align next line.
 */
#define OS_REPORT_NL "\n              "

/*
Note - in the below the check of reportType against os_reportVerbosity is also present
in os_report. By duplicating it we avoid putting the call onto the stack and evaluating
args if not necessary.
*/
#define OS_REPORT(type,context,code,description) \
    if (type >= os_reportVerbosity) os_report(type,context,__FILE__,__LINE__,code,description)

#define OS_REPORT_1(type,context,code,description,a1) \
    if (type >= os_reportVerbosity) os_report(type,context,__FILE__,__LINE__,code,description,a1)

#define OS_REPORT_2(type,context,code,description,a1,a2) \
    if (type >= os_reportVerbosity)  os_report(type,context,__FILE__,__LINE__,code,description,a1,a2)

#define OS_REPORT_3(type,context,code,description,a1,a2,a3) \
    if (type >= os_reportVerbosity) os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3)

#define OS_REPORT_4(type,context,code,description,a1,a2,a3,a4) \
    if (type >= os_reportVerbosity) os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4)

#define OS_REPORT_5(type,context,code,description,a1,a2,a3,a4,a5) \
    if (type >= os_reportVerbosity) os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4,a5)

#define OS_REPORT_6(type,context,code,description,a1,a2,a3,a4,a5,a6) \
    if (type >= os_reportVerbosity) os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4,a5,a6)

#define OS_REPORT_7(type,context,code,description,a1,a2,a3,a4,a5,a6,a7) \
    if (type >= os_reportVerbosity) os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4,a5,a6,a7)

#define OS_REPORT_8(type,context,code,description,a1,a2,a3,a4,a5,a6,a7,a8) \
    if (type >= os_reportVerbosity) os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4,a5,a6,a7,a8)

#define OS_REPORT_9(type,context,code,description,a1,a2,a3,a4,a5,a6,a7,a8,a9) \
    if (type >= os_reportVerbosity) os_report(type,context,__FILE__,__LINE__,code,description,a1,a2,a3,a4,a5,a6,a7,a8,a9)

typedef void * os_IReportService_s;

typedef void * os_reportPlugin;

/**
 * 'Base' of all ::os_report event data structures.
 * It's anticipated that the contents might be added to so it's desirable
 * to do something to enable various version compatibility
 */
struct os_reportEventBase_s
{
    /** The version of the data struct in use */
    os_uint32 version;
};
/**
 * Generic ::os_report event data pointer
 */
typedef struct os_reportEventBase_s* os_reportEvent;

/**
* These types are an ordered series of incremental 'importance' (to the user)
* levels.
* @see os_reportVerbosity
*/
typedef enum os_reportType {
    OS_DEBUG,
    OS_INFO,
    OS_WARNING,
    OS_API_INFO,
    OS_ERROR,
    OS_CRITICAL,
    OS_FATAL,
    OS_REPAIRED,
    OS_NONE
} os_reportType;

/**
 * The information that is made available to a plugged in logger
 * via its TypedReport symbol.
 */
struct os_reportEventV1_s
{
    /** The version of this struct i.e. 1. */
    os_uint32 version;
    /** The type / level of this report.
     * @see os_reportType */
    os_reportType reportType;
    /** Context information relating to where the even was generated.
     * May contain a function or compnent name or a stacktrace */
    const char* reportContext;
    /** The source file name where the report even was generated */
    const char* fileName;
    /** The source file line number where the report was generated */
    os_int32 lineNo;
    /** An integer code associated with the event. */
    os_int32 code;
    /** A description of the reported event */
    const char *description;
    /** A string identifying the thread the event occured in */
    const char* threadDesc;
    /** A string identifying the process the event occured in */
    const char* processDesc;
};

#define OS_REPORT_EVENT_V1 1

/**
 * Pointer to an os_reportEventV1_s struct.
 * @see os_reportEventV1_s
 */
typedef struct os_reportEventV1_s* os_reportEventV1;

/**
* Labels corresponding to os_reportType values.
* @see os_reportType
*/
OS_API extern const char *os_reportTypeText [];

typedef struct os_reportInfo_s {
    char *reportContext;
    char *sourceLine;
    char *callStack;
    os_int32 reportCode;
    char *description;
} os_reportInfo;

/* Docced in impl file */
OS_API extern os_reportType os_reportVerbosity;

OS_API void
os_reportInit(os_boolean forceReInit);

OS_API void os_reportExit();

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
    const char *typedreport_method_name,
    const char *finalize_method_name,
    os_boolean suppressDefaultLogs,
    os_reportPlugin *plugin);

typedef void *os_reportPlugin_context;

typedef int
(*os_reportPlugin_initialize)(
    const char *argument,
    os_reportPlugin_context *context);

typedef int
(*os_reportPlugin_report)(
    os_reportPlugin_context context,
    const char *report);

/**
 * Function pointer type for a plugged in report method
 * taking a typed report event
 */
typedef int
(*os_reportPlugin_typedreport)(
    os_reportPlugin_context context,
    os_reportEvent report);

typedef int
(*os_reportPlugin_finalize)(
    os_reportPlugin_context context);

OS_API os_int32
os_reportInitPlugin(
    const char *argument,
    os_reportPlugin_initialize initFunction,
    os_reportPlugin_finalize finalizeFunction,
    os_reportPlugin_report reportFunction,
    os_reportPlugin_typedreport typedReportFunction,
    os_boolean suppressDefaultLogs,
    os_reportPlugin *plugin);

OS_API os_int32
os_reportUnregisterPlugin(
    os_reportPlugin plugin);

OS_API void
os_reportDisplayLogLocations();

/* docced in implementation file */
OS_API char*
os_reportErrnoToString(int errNo);

OS_API char *
os_reportGetInfoFileName(void);

OS_API char *
os_reportGetErrorFileName(void);

OS_API os_result
os_reportSetVerbosity(
    const char* newVerbosityString);

OS_API void
os_reportSetDoAppend(
    os_boolean shouldAppend);



#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_REPORT_H */
