/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef OS_REPORT_H
#define OS_REPORT_H


#include "os_defs.h"
#include <stdarg.h>
#include "os_if.h"
#include "os_time.h"
#include "os_library.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* Subcomponents might need to alter the report before actually handing it over
 * to os_report. Since os_report truncates messages, those components can get
 * away with fixed size buffers as well, but the maximum size must known at
 * that point.
 */
#define OS_REPORT_BUFLEN 1024

/* following new line macro can be used in report description to insert
 * a new line and indent to align next line.
 */
#define OS_REPORT_NL "\n              "

/*
Note - in the below the check of reportType against os_reportVerbosity is also present
in os_report. By duplicating it we avoid putting the call onto the stack and evaluating
args if not necessary.
*/
#define OS_REPORT(type,context,code,...) \
    (((type) >= os_reportVerbosity) ? os_report((type),(context),__FILE__,__LINE__,(code),-1,OS_TRUE,__VA_ARGS__) : (void)0)

#define OS_REPORT_WID(type,context,code,id,...) \
    (((type) >= os_reportVerbosity) ? os_report((type),(context),__FILE__,__LINE__,(code),(id),OS_TRUE,__VA_ARGS__) : (void)0)

#define OS_REPORT_NOW(type,context,code,id,...) \
    (((type) >= os_reportVerbosity) ? os_report((type),(context),__FILE__,__LINE__,(code),(id),OS_FALSE,__VA_ARGS__) : (void)0)

#define OS_REPORT_STACK() \
    os_report_stack_open(NULL, 0, NULL, NULL)

#define OS_REPORT_FLUSH(condition) \
    os_report_flush((condition), OS_FUNCTION, __FILE__, __LINE__, -1)

#define OS_REPORT_DUMP() \
    os_report_dump(OS_TRUE, OS_FUNCTION, __FILE__, __LINE__, -1)

typedef void * os_reportPlugin;

/**
* These types are an ordered series of incremental 'importance' (to the user)
* levels.
* @see os_reportVerbosity
*/
typedef enum os_reportType {
    OS_DEBUG,
    OS_INFO,
    OS_WARNING,
    OS_API_INFO, /* Deprecated, only here for backwards compatibility */
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
typedef struct os_reportEvent_s
{
    /** The type / level of this report.
     * @see os_reportType */
    os_reportType reportType;
    /** Context information relating to where the even was generated.
     * May contain a function or compnent name or a stacktrace */
    os_char* reportContext;
    /** The source file name where the report even was generated */
    os_char* fileName;
    /** The source file line number where the report was generated */
    os_int32 lineNo;
    /** An integer code associated with the event. */
    os_int32 code;
    /** A description of the reported event */
    os_char *description;
    /** A string identifying the thread the event occured in */
    os_char* threadDesc;
    /** A string identifying the process the event occured in */
    os_char* processDesc;
} *os_reportEvent;

#define OS_REPORT_EVENT_V1 1

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

typedef os_int32 (*os_reportGetDomainCallback)(void *arg);

OS_API void
os_reportRegisterDomainCallback(os_reportGetDomainCallback callback, void *arg);

OS_API os_int32
os_reportGetDomain(void);

OS_API void
os_reportInit(os_boolean forceReInit);

OS_API void os_reportExit(void);

/** \brief Report message directly and do not treat as formatting string
 *
 * Consider this function private. It should be invoked by reporting functions
 * specified in the language bindings only.
 *
 * @param type type of report
 * @param context context in which report was generated, often function name
 *                from which function was invoked
 * @param path path of file from which function was invoked
 * @param line line of file from which function was invoked
 * @param code error code associated with the report
 * @param message message to report
 */

OS_API void
os_report(
        os_reportType type,
        const os_char *context,
        const os_char *path,
        os_int32 line,
        os_int32 code,
        os_int32 domainId,
        os_boolean stack,
        const os_char *format,
        ...) __attribute_format__((printf,8,9));

OS_API void
os_report_va(
        os_reportType type,
        const os_char *context,
        const os_char *path,
        os_int32 line,
        os_int32 code,
        os_int32 domainId,
        os_boolean stack,
        const os_char *format,
        va_list args);

OS_API os_reportInfo *
os_reportGetApiInfo(void);

OS_API void
os_reportClearApiInfo(void);

OS_API os_int32
os_reportRegisterPlugin(
    const char *library_file_name,
    const char *initialize_method_name,
    const char *argument,
    const char *report_method_name,
    const char *typedreport_method_name,
    const char *finalize_method_name,
    os_boolean suppressDefaultLogs,
    os_int32 domainId,
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
    os_int32 domainId,
    os_library libraryHandle,
    os_reportPlugin *plugin);

OS_API os_int32
os_reportUnregisterPlugin(
    os_reportPlugin plugin);

OS_API void
os_reportDisplayLogLocations(void);

OS_API char *
os_reportGetInfoFileName(void);

OS_API char *
os_reportGetErrorFileName(void);

OS_API os_result
os_reportSetVerbosity(
    const char* newVerbosityString);

OS_API void
os_reportRemoveStaleLogs(void);

/*****************************************
 * Report stack related functions
 *****************************************/

/**
 * The os_report_stack_open operation enables a report stack for the current thread.
 * It initializes the report record with the file, line and the signature of the
 * operation that called this operation.
 * The stack will be disabled again by the os_report_flush operation.
 */
OS_API void
os_report_stack_open(
    const os_char *file,
    os_int lineno,
    const os_char *signature,
    void *userInfo);

/**
 * The os_report_stack_free operation frees all memory allocated by the current
 * thread for the report stack.
 */
OS_API void
os_report_stack_free(
    void);

/**
 * The os_report_flush_required checks if the report stack must be flushed.
 * This function checks if the report stack is back at the top of the stack.
 * When the report stack is not at the top the level moved up by one and returns
 * FALSE. When the report stack is at the top level the function returns TRUE
 * when the flush argument is TRUE or when the report stack contains critical
 * reports or warnings which always need to be reported.
 * When this function returns true the caller should call the function
 * os_report_flush to flush the reports from the stack
 */
OS_API os_boolean
os_report_status(
    _In_ os_boolean flush);

/**
 * The os_report_flush operation removes the report message from the stack,
 * and if valid is TRUE also writes them into the report device.
 * This operation additionally disables the stack.
 * Note that this function should only be called when the function
 * os_report_status returns TRUE.
 */
OS_API void
os_report_flush(
    _In_ os_boolean flush,
    _In_ const os_char *context,
    _In_ const os_char *file,
    _In_ os_int line,
    _In_ os_int32 domainId) __nonnull((2,3));

/**
 * The os_report_dump operation removes the report messages from the stack
 * and writes them into the report device, regardless of the state the stack is
 * in. This is useful in panic situations, where you want to dump the content of
 * the current stack prior forcefully terminating the application.
 * This operation keeps the stack in the same state as it was in before invocation,
 * with the one exception that all messages in the stack that have been reported
 * are now removed.
 */
OS_API void
os_report_dump(
    os_boolean valid,
    const os_char *context,
    const os_char *file,
    os_int line,
    os_int32 domainId);

typedef os_char * (os_report_context_callback)(const os_char *context, os_char *buffer, os_uint buflen, void *arg);

/**
 * The os_report_get_context operation returns the context information
 * saved in the report stack
 */
OS_API os_boolean
os_report_get_context(
    const os_char **file,
    os_int *lineno,
    const os_char **signature,
    void **userInfo);

/**
 * The os_report_stack_size operation returns the number of messages in the report stack.
 * This operation will return -1 when no stack is active.
 */
OS_API os_int32
os_report_stack_size(void);

/**
 * The os_report_read operation returns the report message specified by a given index in the stack.
 * This operation will return a null pointer when the index is out of range.
 */
OS_API os_reportEvent
os_report_read(
    os_int32 index);

OS_API void
os_reportIgnoreUserClock();

OS_API os_timeW
os_report_getTimeW();


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_REPORT_H */
