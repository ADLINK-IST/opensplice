/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef CMA__LOG_H_
#define CMA__LOG_H_

#include "cma__types.h"
#include "cma__util.h"

#include "os_report.h"
#include "os_errno.h"

/* Log categories */
#define LOG_NONE    0U
#define LOG_FATAL   1U
#define LOG_ERROR   2U
#define LOG_WARNING 4U
#define LOG_INFO    8U
#define LOG_CONFIG  16U
#define LOG_TRACE   (LOG_FATAL | LOG_ERROR | LOG_WARNING | LOG_INFO | LOG_CONFIG)

typedef unsigned cma_logcat;
typedef unsigned cma_loglevel;

typedef struct cma_logbuf_t {
        char buf[2048];
        size_t bufsz;
        size_t pos;
        cma_time tstamp;
} *cma_logbuf;

cma_logbuf
cma_logbufNew(void) __attribute_malloc__ __attribute_returns_nonnull__;

void
cma_logbufInit(
    cma_logbuf _this) __nonnull_all__;

void
cma_logbufFree(
    cma_logbuf _this,
    cma_thread self) __nonnull_all__;

/* Convert verbosity string from configuration to cma_loglevel (and vice-versa) */
cma_loglevel
cma_logVerbosityFromString(
    const os_char *str) __attribute_pure__ __nonnull_all__;

os_char*
cma_logVerbosityToString(
    cma_loglevel verbosity) __attribute_malloc__ __attribute_returns_nonnull__;

/* Convert category string from configuration to cma_logcat (and vice-versa) */
cma_logcat
cma_logCategoryFromString(
    const os_char *str) __nonnull_all__;

os_char*
cma_logCategoryToString(
    cma_logcat category) __attribute_malloc__ __attribute_returns_nonnull__;

int
cma_log(
    cma_loglevel level,
    const char *fmt,
    ...) __attribute_format__((printf, 2, 3));

int
cma_trace(
    const char *fmt,
    ...) __attribute_format__((printf, 1, 2));

/* CMA_TRACE: log to trace log only, if configured level is LL_FINEST */
#define CMA_TRACE(args) (cma_trace args)

/* CMA_INFO: log to trace log as LL_INFO and to OpenSplice log as OS_INFO */
#define CMA_INFO(context, ...) \
    do { \
        cma_log(LOG_INFO, __VA_ARGS__); \
        os_report(OS_INFO, context, __FILE__, __LINE__, 0, __VA_ARGS__); \
    } while (0)

/* CMA_WARNING: log to trace log as LL_WARNING and to OpenSplice log as OS_WARNING */
#define CMA_WARNING(context, ...) \
    do { \
        cma_log(LOG_WARNING, __VA_ARGS__); \
        os_report(OS_WARNING, context, __FILE__, __LINE__, 0, __VA_ARGS__); \
    } while (0)

#define CMA_ERROR(context, ...) \
    do { \
        cma_log(LOG_ERROR, __VA_ARGS__); \
        os_report(OS_ERROR, context, __FILE__, __LINE__, 0, __VA_ARGS__); \
    } while (0)

#define CMA_FATAL(context, ...) \
    do { \
        cma_log(LOG_FATAL, __VA_ARGS__); \
        os_report(OS_FATAL, context, __FILE__, __LINE__, 0, __VA_ARGS__); \
    } while (0)

#endif /* CMA__LOG_H_ */
