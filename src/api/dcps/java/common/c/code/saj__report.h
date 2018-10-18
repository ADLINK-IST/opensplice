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
#ifndef SAJ__REPORT_H
#define SAJ__REPORT_H

#include "os_defs.h"

#define SAJ_REPORT_STACK()          \
    saj_report_stack ()

#define SAJ_REPORT(code,...)        \
    saj_report (                    \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        code,                       \
        __VA_ARGS__)

#define SAJ_REPORT_FLUSH(condition) \
    saj_report_flush (              \
        __FILE__,                   \
        __LINE__,                   \
        OS_FUNCTION,                \
        (condition))

void
saj_report_stack(
    void);

void
saj_report(
    const os_char *file,
    os_int32 line,
    const os_char *function,
    os_int32 code,
    const char *format,
    ...);

void
saj_report_deprecated(
    const os_char *file,
    os_int32 line,
    const os_char *function,
    const char *format,
    ...);

void
saj_report_flush(
    const os_char *file,
    os_int32 line,
    const os_char *function,
    os_int32 flush);

#endif /* SAJ__REPORT_H */
