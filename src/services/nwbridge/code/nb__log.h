/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef NB__LOG_H_
#define NB__LOG_H_

#include "os_report.h"
#include "nb__util.h"
#include "nb__types.h"
#include "os_errno.h"

/* Log categories */
#define LOG_NONE    0U
#define LOG_FATAL   1U
#define LOG_ERROR   2U
#define LOG_WARNING 4U
#define LOG_INFO    8U
#define LOG_CONFIG  16U
#define LOG_TRACE   (LOG_FATAL | LOG_ERROR | LOG_WARNING | LOG_INFO | LOG_CONFIG)

typedef unsigned nb_logcat;

/*
 * NB_TRACE macro: goes to trace log only, if configured level is LL_FINEST
 * NB_INFO macro's: go to trace log as LL_INFO and to OpenSplice log as OS_INFO
 * NB_WARNING macro's: go to trace log as LL_WARNING and to OpenSplice log as OS_WARNING
 * NB_ERROR macro's: go to trace log as LL_SEVERE and to OpenSplice log as OS_ERROR
 * NB_FATAL macro's: go to trace log as LL_SEVERE and to OpenSplice log as OS_FATAL and abort is called
 */

typedef unsigned nb_loglevel;

typedef struct nb_logbuf_t {
    char buf[2048];
    size_t bufsz;
    size_t pos;
    os_timeW tstamp;
} *nb_logbuf;

nb_logbuf   nb_logbufNew(void) __attribute_malloc__
                               __attribute_returns_nonnull__;

void        nb_logbufInit(nb_logbuf lb) __nonnull_all__;

void        nb_logbufFree(nb_thread self, nb_logbuf lb) __nonnull_all__;

/* Converts verbosity string from configfile to nb_loglevel (and vice-versa) */
nb_loglevel nb_logVerbosityFromString(const os_char *str) __attribute_pure__
                                                          __nonnull_all__;

os_char*    nb_logVerbosityToString(nb_loglevel verbosity) __attribute_malloc__
                                                           __attribute_returns_nonnull__;

/* Converts category string from config file to nb_logcat (and vice-versa) */
nb_logcat   nb_logCategoryFromString(os_char *str) __nonnull_all__;

os_char*    nb_logCategoryToString(nb_logcat category) __attribute_malloc__
                                                       __attribute_returns_nonnull__;

int         nb_log(nb_loglevel level, const char *fmt, ...) __attribute_format__((printf, 2, 3));

int         nb_trace(const char *fmt, ...) __attribute_format__((printf, 1, 2));

#define NB_TRACE(args) (nb_trace args)
#define NB_KEYFMT "{systemId=%u, localId=%u}"

/* TODO rewrite all os_report to use one of the macros below */

#define NB_INFO(context, fmt) do { \
        nb_log(LOG_INFO, (fmt)); \
        OS_REPORT(OS_INFO, context, 0, (fmt)); \
    } while (0)

#define NB_INFO_1(context, fmt, a) do { \
        nb_log(LOG_INFO, (fmt), a); \
        OS_REPORT(OS_INFO, context, 0, (fmt), a); \
    } while (0)

#define NB_INFO_2(context, fmt, a, b) do { \
        nb_log(LOG_INFO, (fmt), a, b); \
        OS_REPORT(OS_INFO, context, 0, (fmt), a, b); \
    } while (0)

#define NB_INFO_3(context, fmt, a, b, c) do { \
        nb_log(LOG_INFO, (fmt), a, b, c); \
        OS_REPORT(OS_INFO, context, 0, (fmt), a, b, c); \
    } while (0)

#define NB_WARNING(context, fmt) do { \
        nb_log(LOG_WARNING, (fmt)); \
        OS_REPORT(OS_WARNING, context, 0, (fmt)); \
    } while (0)

#define NB_WARNING_1(context, fmt, a) do { \
        nb_log(LOG_WARNING, (fmt), a); \
        OS_REPORT(OS_WARNING, context, 0, (fmt), a); \
    } while (0)

#define NB_WARNING_2(context, fmt, a, b) do { \
        nb_log(LOG_WARNING, (fmt), a, b); \
        OS_REPORT(OS_WARNING, context, 0, (fmt), a, b); \
    } while (0)

#define NB_WARNING_3(context, fmt, a, b, c) do { \
        nb_log(LOG_WARNING, (fmt), a, b, c); \
        OS_REPORT(OS_WARNING, context, 0, (fmt), a, b, c); \
    } while (0)

#define NB_ERROR(context, fmt) do { \
        nb_log(LOG_ERROR, (fmt)); \
        OS_REPORT(OS_ERROR, context, 0, (fmt)); \
    } while (0)

#define NB_ERROR_1(context, fmt, a) do { \
        nb_log(LOG_ERROR, (fmt), a); \
        OS_REPORT(OS_ERROR, context, 0, (fmt), a); \
    } while (0)

#define NB_ERROR_2(context, fmt, a, b) do { \
        nb_log(LOG_ERROR, (fmt), a, b); \
        OS_REPORT(OS_ERROR, context, 0, (fmt), a, b); \
    } while (0)

#define NB_ERROR_3(context, fmt, a, b, c) do { \
        nb_log(LOG_ERROR, (fmt), a, b, c); \
        OS_REPORT(OS_ERROR, context, 0, (fmt), a, b, c); \
    } while (0)

#define NB_FATAL(context, fmt) do { \
        nb_log(LOG_FATAL, (fmt)); \
        OS_REPORT(OS_FATAL, context, 0, (fmt)); \
        abort(); \
    } while (0)

#define NB_FATAL_1(context, fmt, a) do { \
        nb_log(LOG_FATAL, (fmt), a); \
        OS_REPORT(OS_FATAL, context, 0, (fmt), a); \
        abort(); \
    } while (0)

#define NB_FATAL_2(context, fmt, a, b) do { \
        nb_log(LOG_FATAL, (fmt), a, b); \
        OS_REPORT(OS_FATAL, context, 0, (fmt), a, b); \
        abort(); \
    } while (0)

#endif /* NB__LOG_H_ */
