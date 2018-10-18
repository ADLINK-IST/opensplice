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
#ifndef SAC_REPORT_H
#define SAC_REPORT_H

#include <stdarg.h>
#include "os_report.h"

#define SAC_REPORT_STACK()          \
    os_report_stack_open (NULL,0,NULL,NULL)

#define SAC_PANIC(...)              \
    do {                            \
        sac_panic (                 \
            __FILE__,               \
            __LINE__,               \
            OS_FUNCTION,            \
            __VA_ARGS__);           \
        assert (FALSE);             \
    } while (0);

#define SAC_REPORT(code,...)        \
    sac_report (                    \
        OS_ERROR,                   \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        (code),                     \
        __VA_ARGS__)

#define SAC_REPORT_DEPRECATED(...)  \
    sac_report (                    \
        OS_API_INFO,                \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        DDS_RETCODE_OK,             \
        __VA_ARGS__)

#define SAC_REPORT_WARNING(...)     \
    sac_report (                    \
        OS_API_INFO,                \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        DDS_RETCODE_OK,             \
        __VA_ARGS__)

#define SAC_REPORT_FLUSH(obj, condition) \
    sac_report_flush (              \
        (obj),                      \
        (condition),                \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION)


void
sac_panic(
    const os_char *file,
    os_int32 line,
    const os_char *function,
    const os_char *format,
    ...);

void
sac_report(
    os_reportType reportType,
    const os_char *file,
    os_int32 line,
    const os_char *function,
    os_int32 code,
    const char *format,
    ...);

void
sac_report_flush(
    DDS_Object object,
    DDS_boolean valid,
    const os_char *file,
    os_int32 line,
    const os_char *function);

#endif /* SAC_REPORT_H */
