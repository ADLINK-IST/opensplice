
#ifndef IN_REPORT_H
#define IN_REPORT_H

#include "os_report.h"
#include "in_configuration.h"
#include "in__configTracing.h"
#include "in__config.h"

/* Errors, warnings and info for everybody who is interested */

#define IN_SERVICENAME "ddsi"
/* conversion of __LINE__ to number-string requires two macro invocations */
#define IN_STRINGIFY(Y) #Y
#define __IN_STRINGIFY2(X) IN_STRINGIFY(X)
#define IN_SPOT __FILE__":"__IN_STRINGIFY2(__LINE__)

#define IN_STRINGIFY_BOOLEAN(Y) ((Y) ? "true" : "false")

#define IN_REPORT_ERROR(funcName, description)                                \
    OS_REPORT(OS_ERROR, IN_SERVICENAME ": " funcName, 0, description)
#define IN_REPORT_ERROR_1(funcName, description, a1)                          \
    OS_REPORT_1(OS_ERROR, IN_SERVICENAME ": " funcName, 0, description, a1)
#define IN_REPORT_ERROR_2(funcName, description, a1, a2)                      \
    OS_REPORT_2(OS_ERROR, IN_SERVICENAME ": " funcName, 0, description, a1, a2)
#define IN_REPORT_ERROR_3(funcName, description, a1, a2, a3)                   \
    OS_REPORT_3(OS_ERROR, IN_SERVICENAME ": " funcName, 0, description, a1, a2, a3)

#define IN_REPORT_WARNING(funcName, description)                              \
    OS_REPORT(OS_WARNING, IN_SERVICENAME ": " funcName, 0, description)
#define IN_REPORT_WARNING_1(funcName, description, a1)                        \
    OS_REPORT_1(OS_WARNING, IN_SERVICENAME ": " funcName, 0, description, a1)
#define IN_REPORT_WARNING_2(funcName, description, a1, a2)                    \
    OS_REPORT_2(OS_WARNING, IN_SERVICENAME ": " funcName, 0, description, a1, a2)
#define IN_REPORT_WARNING_3(funcName, description, a1, a2, a3)                 \
    OS_REPORT_3(OS_WARNING, IN_SERVICENAME ": " funcName, 0, description, a1, a2, a3)
#define IN_REPORT_WARNING_4(funcName, description, a1, a2, a3, a4)                 \
    OS_REPORT_4(OS_WARNING, IN_SERVICENAME ": " funcName, 0, description, a1, a2, a3, a4)
#define IN_REPORT_WARNING_7(funcName, description, a1, a2, a3, a4, a5, a6, a7)                 \
    OS_REPORT_7(OS_WARNING, IN_SERVICENAME ": " funcName, 0, description, a1, a2, a3, a4, a5, a6, a7)

#define IN_REPORT_PERIOD (10U)
#define IN_REPORT_ERROR_PERIODICLY(funcName, description) \
	do{ \
		static int _cnt=0; /* static counter */ \
		if ((_cnt++ % IN_REPORT_PERIOD)==0) { \
			if (_cnt>0) { \
				IN_REPORT_ERROR_1(funcName,"the following error occured %d times", _cnt); \
			} \
			IN_REPORT_ERROR(funcName,description); \
			_cnt=0; /* reset */ \
		} \
	} while(0)


/* Currently the IN_REPORT_INFO macros do not check whether the log level is of
 * interest or not.  These used to call onto the deprecated in_configurationLevelIsInteresting
 * functions (the configuration is now handled by the in_config code).
 * Also the info logging should be disabled when tracing is switched off to prevent
 * the performance penalty for writing all this info to the ospl-info.log, hence the
 * in_configIsTracingEnabled checks below
 */

#define IN_REPORT_INFO(level, description)                                                 \
    if (in_configIsTracingEnabled ()) {                                                    \
        OS_REPORT(OS_INFO, IN_SERVICENAME " level " #level " information", 0, description);\
    }\

#define IN_REPORT_INFO_1(level, description, a1)                                                 \
    if (in_configIsTracingEnabled ()) {                                                          \
        OS_REPORT_1(OS_INFO, IN_SERVICENAME " level " #level " information", 0, description, a1);\
    }\

#define IN_REPORT_INFO_2(level, description, a1, a2)                                                 \
    if (in_configIsTracingEnabled ()) {                                                              \
        OS_REPORT_2(OS_INFO, IN_SERVICENAME " level " #level " information", 0, description, a1, a2);\
    }\

#define IN_REPORT_INFO_3(level, description, a1, a2, a3)                                                 \
    if (in_configIsTracingEnabled ()) {                                                                  \
        OS_REPORT_3(OS_INFO, IN_SERVICENAME " level " #level " information", 0, description, a1, a2, a3);\
    }\

#define IN_REPORT_INFO_4(level, description, a1, a2, a3, a4)                                                 \
    if (in_configIsTracingEnabled ()) {                                                                      \
        OS_REPORT_4(OS_INFO, IN_SERVICENAME " level " #level " information", 0, description, a1, a2, a3, a4);\
    }\


/* Tracing for non-release versions only */

#ifdef IN_TRACING

#define IN_TRACE(traceClass, level, description)                           \
    in_configTracingReport(TC(traceClass), level, #traceClass, description "\n")
#define IN_TRACE_1(traceClass, level, description, a1)                 \
    in_configTracingReport(TC(traceClass), level, #traceClass, description "\n", a1)
#define IN_TRACE_2(traceClass, level, description, a1, a2)             \
    in_configTracingReport(TC(traceClass), level, #traceClass, description "\n", a1, a2)
#define IN_TRACE_3(traceClass, level, description, a1, a2, a3)             \
    in_configTracingReport(TC(traceClass), level, #traceClass, description "\n", a1, a2, a3)

#else /* IN_TRACING */

#define IN_TRACE(traceClass, level, description)
#define IN_TRACE_1(traceClass, level, description, a1)
#define IN_TRACE_2(traceClass, level, description, a1, a2)
#define IN_TRACE_3(traceClass, level, description, a1, a2, a3)

#endif /* IN_TRACING */

#endif /* IN_REPORT_H */

