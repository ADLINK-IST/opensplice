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
#ifndef VORTEX_FACE_REPORT_SUPPORT_HPP_
#define VORTEX_FACE_REPORT_SUPPORT_HPP_

#include "Vortex_FACE.hpp"
#include <os_report.h>
#include <stdarg.h>


namespace Vortex {
namespace FACE {

#define FACE_REPORT_STACK_BEGIN()     \
    Vortex::FACE::report_stack_open(  \
        __FILE__,                     \
        __LINE__,                     \
        OS_PRETTY_FUNCTION)

#define FACE_REPORT_STACK_END(flush)   \
     Vortex::FACE::report_stack_close( \
        -1,                            \
        __FILE__,                      \
        __LINE__,                      \
        OS_PRETTY_FUNCTION,            \
        flush)

#define FACE_REPORT_STACK_CID_END(cid, flush) \
     Vortex::FACE::report_stack_close( \
        (cid),                         \
        __FILE__,                      \
        __LINE__,                      \
        OS_PRETTY_FUNCTION,            \
        flush)

#define FACE_REPORT_ERROR(code, ...) \
    Vortex::FACE::report(            \
        (code),                      \
        OS_ERROR,                    \
        __FILE__,                    \
        __LINE__,                    \
        OS_PRETTY_FUNCTION,          \
        __VA_ARGS__)


VORTEX_FACE_API std::string
returnCodeToString(::FACE::RETURN_CODE_TYPE return_code);

VORTEX_FACE_API ::FACE::RETURN_CODE_TYPE
exceptionToReturnCode(const dds::core::Exception& e);


VORTEX_FACE_API void
report_stack_open(
    const char *file,
    int32_t line,
    const char *signature);

VORTEX_FACE_API void
report_stack_close(
    ::FACE::CONNECTION_ID_TYPE connectionId,
    const char *file,
    int32_t line,
    const char *signature,
    bool flush);

VORTEX_FACE_API void
report(
    ::FACE::RETURN_CODE_TYPE code,
    os_reportType reportType,
    const os_char *file,
    int32_t line,
    const os_char *signature,
    const os_char *format,
    ...);

}; /* namespace FACE */
}; /* namespace Vortex */

#endif /* VORTEX_FACE_REPORT_SUPPORT_HPP_ */
