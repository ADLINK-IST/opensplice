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
#ifndef DDS_OPENSPLICE_REPORTUTILS_H
#define DDS_OPENSPLICE_REPORTUTILS_H

#include "ccpp.h"
#include "CppSuperClass.h"

#include "os_defs.h"
#include "os_report.h"
#include "cpp_dcps_if.h"

#include <stdarg.h>

namespace DDS {
namespace OpenSplice {
namespace Utils {

#define CPP_REPORT_STACK()                  \
    DDS::OpenSplice::Utils::report_stack()

/* Panic must flush report stack immediately */
#define CPP_PANIC(...)                      \
    do {                                    \
        DDS::OpenSplice::Utils::panic (     \
            __FILE__,                       \
            __LINE__,                       \
            OS_PRETTY_FUNCTION,             \
            __VA_ARGS__);                   \
        assert (FALSE);                     \
    } while (0);

#define CPP_REPORT(code, ...)               \
    DDS::OpenSplice::Utils::report (        \
        OS_ERROR,                           \
        __FILE__,                           \
        __LINE__,                           \
        OS_PRETTY_FUNCTION,                 \
        (code),                             \
        __VA_ARGS__)

#define CPP_REPORT_DEPRECATED(...)          \
     DDS::OpenSplice::Utils::report (       \
        OS_API_INFO,                        \
        __FILE__,                           \
        __LINE__,                           \
        OS_PRETTY_FUNCTION,                 \
        DDS::RETCODE_OK,                    \
        __VA_ARGS__)

#define CPP_REPORT_WARNING(...)             \
     DDS::OpenSplice::Utils::report (       \
        OS_WARNING,                         \
        __FILE__,                           \
        __LINE__,                           \
        OS_PRETTY_FUNCTION,                 \
        DDS::RETCODE_OK,                    \
        __VA_ARGS__)

#define CPP_REPORT_FLUSH_NO_ID(condition)   \
    DDS::OpenSplice::Utils::report_flush(   \
        __FILE__,                           \
        __LINE__,                           \
        OS_PRETTY_FUNCTION,                 \
        (condition),                        \
        NULL)

#define CPP_REPORT_FLUSH(obj, condition)    \
    DDS::OpenSplice::Utils::report_flush(   \
        __FILE__,                           \
        __LINE__,                           \
        OS_PRETTY_FUNCTION,                 \
        (condition),                        \
        (obj))

OS_API os_char *
pretty_function(
    const os_char *);

OS_API void
report_stack();

OS_API void
panic(
    const os_char *file,
    os_int32 line,
    const os_char *signature,
    const os_char *format,
    ...);

OS_API void
report(
    os_reportType reportType,
    const os_char *file,
    os_int32 line,
    const os_char *signature,
    DDS::ReturnCode_t code,
    const os_char *format,
    ...);

OS_API void
report_flush(
    const os_char *file,
    os_int32 line,
    const os_char *signature,
    DDS::Boolean flush,
    CppSuperClassInterface *object);

} /* end namespace Utils */
} /* end namespace OpenSplice */
} /* end namespace DDS */

#undef OS_API

#endif /* DDS_OPENSPLICE_REPORTUTILS_H */
