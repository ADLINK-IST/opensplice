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

#ifndef DDS_REPORT_H
#define DDS_REPORT_H

#include <stdarg.h>
#include "os_report.h"

#define DDS_REPORT_STACK()          \
    os_report_stack_open (NULL,0,NULL,NULL)

#define DDS_PANIC(...)              \
    do {                            \
        dds_panic (                 \
            __FILE__,               \
            __LINE__,               \
            OS_FUNCTION,            \
            __VA_ARGS__);           \
        assert (FALSE);             \
    } while (0);

#define DDS_REPORT(code,...)        \
    dds_report (                    \
        OS_ERROR,                   \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        (code),                     \
        __VA_ARGS__)

#define DDS_REPORT_DEPRECATED(...)  \
    dds_report (                    \
        OS_API_INFO,                \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        DDS_RETCODE_OK,             \
        __VA_ARGS__)

#define DDS_REPORT_WARNING(...)     \
    dds_report (                    \
        OS_API_INFO,                \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        DDS_RETCODE_OK,             \
        __VA_ARGS__)

#define DDS_REPORT_FLUSH(obj, condition) \
    dds_report_flush (              \
        ((void*)obj),               \
        (condition),                \
        __FILE__,                   \
        __LINE__,                   \
       OS_FUNCTION)

void
dds_panic(
    const char *file,
    int32_t line,
    const char *function,
    const char *format,
    ...);

void
dds_report(
    os_reportType reportType,
    const char *file,
    int32_t line,
    const char *function,
    int32_t code,
    const char *format,
    ...);

void
dds_report_flush(
    void *object,
    bool valid,
    const char *file,
    int32_t line,
    const char *function);

int
dds_report_get_error_code();

/* Module identifiers for error codes */

#define DDS_MOD_QOS         0x0100
#define DDS_MOD_KERNEL      0x0200
#define DDS_MOD_DDSI        0x0300
#define DDS_MOD_STREAM      0x0400
#define DDS_MOD_ALLOC       0x0500
#define DDS_MOD_WAITSET     0x0600
#define DDS_MOD_READER      0x0700
#define DDS_MOD_WRITER      0x0800
#define DDS_MOD_COND        0x0900
#define DDS_MOD_RHC         0x0a00
#define DDS_MOD_STATUS      0x0b00
#define DDS_MOD_THREAD      0x0c00
#define DDS_MOD_INST        0x0d00
#define DDS_MOD_PPANT       0x0e00

/* Minor numbers for error codes */

#define DDS_ERR_Mx      0x000000
#define DDS_ERR_M1      0x010000
#define DDS_ERR_M2      0x020000
#define DDS_ERR_M3      0x030000
#define DDS_ERR_M4      0x040000
#define DDS_ERR_M5      0x050000
#define DDS_ERR_M6      0x060000
#define DDS_ERR_M7      0x070000
#define DDS_ERR_M8      0x080000
#define DDS_ERR_M9      0x090000
#define DDS_ERR_M10     0x0A0000
#define DDS_ERR_M11     0x0B0000
#define DDS_ERR_M12     0x0C0000
#define DDS_ERR_M13     0x0D0000
#define DDS_ERR_M14     0x0E0000
#define DDS_ERR_M15     0x0F0000
#define DDS_ERR_M16     0x100000
#define DDS_ERR_M17     0x110000
#define DDS_ERR_M18     0x120000
#define DDS_ERR_M19     0x130000
#define DDS_ERR_M20     0x140000

/* To construct return status */

/* When OK: just return OK
 *   When e < 0: expect the status to have been already constructed.
 *     Otherwise: construct
 */
#define DDS_ERRNO(e,m,n) (int)((e == DDS_RETCODE_OK) ?        \
                                     DDS_RETCODE_OK  :        \
                                     ((e < 0)           ?     \
                                           e            :     \
                                           -((n) | (m) | (e))))


#endif /* DDS_REPORT_H */
