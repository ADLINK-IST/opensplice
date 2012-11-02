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

#ifndef NW_REPORT_H
#define NW_REPORT_H

#include "os_report.h"
#include "nw_configuration.h"

/* Errors, warnins and info for everybody who is interested */

#define NW_SERVICENAME "networking"

#define NW_REPORT_ERROR(funcName, description)                                \
    OS_REPORT(OS_ERROR, NW_SERVICENAME ": " funcName, 0, description)
#define NW_REPORT_ERROR_1(funcName, description, a1)                          \
    OS_REPORT_1(OS_ERROR, NW_SERVICENAME ": " funcName, 0, description, a1)
#define NW_REPORT_ERROR_2(funcName, description, a1, a2)                      \
    OS_REPORT_2(OS_ERROR, NW_SERVICENAME ": " funcName, 0, description, a1, a2)
#define NW_REPORT_ERROR_3(funcName, description, a1, a2, a3)                   \
    OS_REPORT_3(OS_ERROR, NW_SERVICENAME ": " funcName, 0, description, a1, a2, a3)
#define NW_REPORT_ERROR_4(funcName, description, a1, a2, a3, a4)               \
    OS_REPORT_4(OS_ERROR, NW_SERVICENAME ": " funcName, 0, description, a1, a2, a3, a4)

#define NW_REPORT_WARNING(funcName, description)                              \
    OS_REPORT(OS_WARNING, NW_SERVICENAME ": " funcName, 0, description)
#define NW_REPORT_WARNING_1(funcName, description, a1)                        \
    OS_REPORT_1(OS_WARNING, NW_SERVICENAME ": " funcName, 0, description, a1)
#define NW_REPORT_WARNING_2(funcName, description, a1, a2)                    \
    OS_REPORT_2(OS_WARNING, NW_SERVICENAME ": " funcName, 0, description, a1, a2)
#define NW_REPORT_WARNING_3(funcName, description, a1, a2, a3)                 \
    OS_REPORT_3(OS_WARNING, NW_SERVICENAME ": " funcName, 0, description, a1, a2, a3)
#define NW_REPORT_WARNING_4(funcName, description, a1, a2, a3, a4)                 \
    OS_REPORT_4(OS_WARNING, NW_SERVICENAME ": " funcName, 0, description, a1, a2, a3, a4)
#define NW_REPORT_WARNING_5(funcName, description, a1, a2, a3, a4, a5)                 \
    OS_REPORT_5(OS_WARNING, NW_SERVICENAME ": " funcName, 0, description, a1, a2, a3, a4, a5)


#define NW_REPORT_INFO(level, description)                                     \
    if (nw_configurationLevelIsInteresting(level)) {                            \
        OS_REPORT(OS_INFO, NW_SERVICENAME " level " #level " information", 0, description); \
    }\

#define NW_REPORT_INFO_1(level, description, a1)                               \
    if (nw_configurationLevelIsInteresting(level)) {                            \
        OS_REPORT_1(OS_INFO, NW_SERVICENAME " level " #level " information", 0, description, a1); \
    }\

#define NW_REPORT_INFO_2(level, description, a1, a2)                           \
    if (nw_configurationLevelIsInteresting(level)) {                            \
        OS_REPORT_2(OS_INFO, NW_SERVICENAME " level " #level " information", 0, description, a1, a2); \
    }

#define NW_REPORT_INFO_3(level, description, a1, a2, a3)                       \
    if (nw_configurationLevelIsInteresting(level)) {                            \
        OS_REPORT_3(OS_INFO, NW_SERVICENAME " level " #level " information", 0, description, a1, a2, a3); \
    }

#define NW_REPORT_INFO_4(level, description, a1, a2, a3, a4)                       \
    if (nw_configurationLevelIsInteresting(level)) {                            \
        OS_REPORT_4(OS_INFO, NW_SERVICENAME " level " #level " information", 0, description, a1, a2, a3, a4); \
    }



/* Tracing for non-release versions only */
/* N.B. The above does not seem to be true. See nw_configuration.h.
NW_TRACING is always defined */

#ifdef NW_TRACING

/**
* Hold the 'highest' tracing level from any category in te process.
* We can use this to avoid usual case wasteful argument evaluation and stack pushes.
*/
extern c_ulong highestTraceLevel;

#define TC(name) TC_##name

typedef enum nw_traceClass_e {
  TC(Undefined),
  TC(Configuration),
  TC(Construction),
  TC(Destruction),
  TC(Mainloop),
  TC(Groups),
  TC(Send),
  TC(Receive),
  TC(Discovery),
  TC(Test),
  TC(Count)/* Last element, keep this at the end */
} nw_traceClass;

/* You can use this function as is, but using the macro's below is preferred
 * Note: this function is implemented in nw_configuration in order to speed
 * up the testing etc. */
void nw_reportTrace(
         nw_traceClass traceClass,
         c_ulong level,
         const c_char *context,
         const char *description, ...);


#define NW_TRACE(traceClass, level, description)                           \
    if (highestTraceLevel >= level) nw_reportTrace(TC(traceClass), level, #traceClass, description "\n")
#define NW_TRACE_1(traceClass, level, description, a1)                 \
    if (highestTraceLevel >= level) nw_reportTrace(TC(traceClass), level, #traceClass, description "\n", a1)
#define NW_TRACE_2(traceClass, level, description, a1, a2)             \
    if (highestTraceLevel >= level) nw_reportTrace(TC(traceClass), level, #traceClass, description "\n", a1, a2)
#define NW_TRACE_3(traceClass, level, description, a1, a2, a3)             \
    if (highestTraceLevel >= level) nw_reportTrace(TC(traceClass), level, #traceClass, description "\n", a1, a2, a3)
#define NW_TRACE_4(traceClass, level, description, a1, a2, a3, a4)             \
    if (highestTraceLevel >= level) nw_reportTrace(TC(traceClass), level, #traceClass, description "\n", a1, a2, a3, a4)
#define NW_TRACE_5(traceClass, level, description, a1, a2, a3, a4, a5)             \
    if (highestTraceLevel >= level) nw_reportTrace(TC(traceClass), level, #traceClass, description "\n", a1, a2, a3, a4, a5)
#define NW_TRACE_6(traceClass, level, description, a1, a2, a3, a4, a5, a6)             \
    if (highestTraceLevel >= level) nw_reportTrace(TC(traceClass), level, #traceClass, description "\n", a1, a2, a3, a4, a5, a6)
#define NW_TRACE_7(traceClass, level, description, a1, a2, a3, a4, a5, a6, a7)            \
    if (highestTraceLevel >= level) nw_reportTrace(TC(traceClass), level, #traceClass, description "\n", a1, a2, a3, a4, a5, a6, a7)

#else /* NW_TRACING */

#define NW_TRACE(traceClass, level, description)
#define NW_TRACE_1(traceClass, level, description, a1)
#define NW_TRACE_2(traceClass, level, description, a1, a2)
#define NW_TRACE_3(traceClass, level, description, a1, a2, a3)
#define NW_TRACE_4(traceClass, level, description, a1, a2, a3, a4)
#define NW_TRACE_5(traceClass, level, description, a1, a2, a3, a4, a5)
#define NW_TRACE_6(traceClass, level, description, a1, a2, a3, a4, a5, a6)
#define NW_TRACE_7(traceClass, level, description, a1, a2, a3, a4, a5, a6, a7)


#endif /* NW_TRACING */

#endif /* NW_REPORT_H */

