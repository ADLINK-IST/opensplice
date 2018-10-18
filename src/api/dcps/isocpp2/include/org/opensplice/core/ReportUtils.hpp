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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_CORE_REPORT_UTILS_HPP_
#define ORG_OPENSPLICE_CORE_REPORT_UTILS_HPP_

#include <string>
#include <sstream>
#include <org/opensplice/core/config.hpp>
#include <org/opensplice/core/ObjectDelegate.hpp>
#include <dds/core/Exception.hpp>
#include <ios>
#include <os_report.h>
#include <u_types.h>
#include <stdarg.h>


#define ISOCPP_ERROR                       org::opensplice::core::utils::error_code
#define ISOCPP_UNSUPPORTED_ERROR           org::opensplice::core::utils::unsupported_error_code
#define ISOCPP_INVALID_ARGUMENT_ERROR      org::opensplice::core::utils::invalid_argument_code
#define ISOCPP_PRECONDITION_NOT_MET_ERROR  org::opensplice::core::utils::precondition_not_met_error_code
#define ISOCPP_OUT_OF_RESOURCES_ERROR      org::opensplice::core::utils::out_of_resources_error_code
#define ISOCPP_NOT_ENABLED_ERROR           org::opensplice::core::utils::not_enabled_error_code
#define ISOCPP_IMMUTABLE_POLICY_ERROR      org::opensplice::core::utils::immutable_policy_error_code
#define ISOCPP_INCONSISTENT_POLICY_ERROR   org::opensplice::core::utils::inconsistent_policy_error_code
#define ISOCPP_ALREADY_CLOSED_ERROR        org::opensplice::core::utils::already_closed_error_code
#define ISOCPP_TIMEOUT_ERROR               org::opensplice::core::utils::timeout_error_code
#define ISOCPP_NO_DATA_ERROR               org::opensplice::core::utils::no_data_error_code
#define ISOCPP_ILLEGAL_OPERATION_ERROR     org::opensplice::core::utils::illegal_operation_error_code
#define ISOCPP_NULL_REFERENCE_ERROR        org::opensplice::core::utils::null_reference_error_code


#define ISOCPP_REPORT_STACK_NC_BEGIN() \
    org::opensplice::core::utils::ReportFinisher __f; \
    /* Added to satisfy compiler (-Wunused-variable) as __f is used for it's destructor  */ \
    (void) __f;                                       \
    org::opensplice::core::utils::report_stack_open(  \
        NULL,                                         \
        __FILE__,                                     \
        __LINE__,                                     \
        OS_PRETTY_FUNCTION)

#define ISOCPP_REPORT_STACK_DDS_BEGIN(e) \
    org::opensplice::core::utils::ReportFinisher __f; \
    /* Added to satisfy compiler (-Wunused-variable) as __f is used for it's destructor  */ \
    (void) __f;                                       \
    org::opensplice::core::utils::report_stack_open(  \
        (((e).is_nil()? NULL : (e).delegate().get())),  \
        __FILE__,                                     \
        __LINE__,                                     \
        OS_PRETTY_FUNCTION)

#define ISOCPP_REPORT_STACK_DELEGATE_BEGIN(obj) \
    org::opensplice::core::utils::ReportFinisher __f; \
    /* Added to satisfy compiler (-Wunused-variable) as __f is used for it's destructor  */ \
    (void) __f;                                       \
    org::opensplice::core::utils::report_stack_open(  \
        (obj),                                        \
        __FILE__,                                     \
        __LINE__,                                     \
        OS_PRETTY_FUNCTION)

#if 0
#define ISOCPP_REPORT_STACK_END() \
    org::opensplice::core::utils::report_stack_close( \
        __FILE__,                                     \
        __LINE__,                                     \
        OS_PRETTY_FUNCTION,                           \
        false)
#else
#define ISOCPP_REPORT_STACK_END()
#endif
/* An exception also closes the report stack, but flushes it as well. */

#define ISOCPP_REPORT_ERROR(...)                      \
    org::opensplice::core::utils::report (            \
        ISOCPP_ERROR,                                 \
        OS_ERROR,                                     \
        __FILE__,                                     \
        __LINE__,                                     \
        OS_PRETTY_FUNCTION,                           \
        __VA_ARGS__)

#define ISOCPP_REPORT_WARNING(...)                    \
    org::opensplice::core::utils::report (            \
        0,                                            \
        OS_WARNING,                                   \
        __FILE__,                                     \
        __LINE__,                                     \
        OS_PRETTY_FUNCTION,                           \
        __VA_ARGS__)

#define ISOCPP_U_RESULT_CHECK_AND_THROW(code, ...)    \
    org::opensplice::core::utils::check_u_result_and_throw_exception(    \
        (code),                                       \
        __FILE__,                                     \
        __LINE__,                                     \
        OS_PRETTY_FUNCTION,                           \
        __VA_ARGS__)                                  \


#define ISOCPP_OS_RESULT_CHECK_AND_THROW(code, ...)   \
    org::opensplice::core::utils::check_os_result_and_throw_exception(    \
        (code),                                       \
        __FILE__,                                     \
        __LINE__,                                     \
        OS_PRETTY_FUNCTION,                           \
        __VA_ARGS__)                                  \

#define ISOCPP_THROW_EXCEPTION(code, ...)             \
    org::opensplice::core::utils::throw_exception(    \
        (code),                                       \
        __FILE__,                                     \
        __LINE__,                                     \
        OS_PRETTY_FUNCTION,                           \
        __VA_ARGS__)                                  \

#define ISOCPP_BOOL_CHECK_AND_THROW(test, code, ...)  \
    if (!test) {                                      \
        ISOCPP_THROW_EXCEPTION(code, __VA_ARGS__);    \
    }


namespace org
{
namespace opensplice
{
namespace core
{
namespace utils
{

const int32_t error_code                       = 1;
const int32_t unsupported_error_code           = 2;
const int32_t invalid_argument_code            = 3;
const int32_t precondition_not_met_error_code  = 4;
const int32_t out_of_resources_error_code      = 5;
const int32_t not_enabled_error_code           = 6;
const int32_t immutable_policy_error_code      = 7;
const int32_t inconsistent_policy_error_code   = 8;
const int32_t already_closed_error_code        = 9;
const int32_t timeout_error_code               = 10;
const int32_t no_data_error_code               = 11;
const int32_t illegal_operation_error_code     = 12;
const int32_t null_reference_error_code        = 13;


OSPL_ISOCPP_IMPL_API void
report_stack_open(
    const org::opensplice::core::ObjectDelegate *objRef,
    const char *file,
    int32_t line,
    const char *signature);

OSPL_ISOCPP_IMPL_API void
report_stack_close(
    const char *file,
    int32_t line,
    const char *signature,
    bool flush);

OSPL_ISOCPP_IMPL_API void
report(
    int32_t code,
    os_reportType reportType,
    const os_char *file,
    int32_t line,
    const os_char *signature,
    const os_char *format,
    ...);

OSPL_ISOCPP_IMPL_API void
throw_exception(
    int32_t code,
    const os_char *file,
    os_int32 line,
    const os_char *signature,
    const os_char *format,
    ...);

OSPL_ISOCPP_IMPL_API void
check_u_result_and_throw_exception(
    u_result code,
    const os_char *file,
    os_int32 line,
    const os_char *signature,
    const os_char *format,
    ...);

OSPL_ISOCPP_IMPL_API void
check_os_result_and_throw_exception(
    os_result code,
    const os_char *file,
    os_int32 line,
    const os_char *signature,
    const os_char *format,
    ...);

class OMG_DDS_API ReportFinisher {
public:
    ~ReportFinisher();
};

}
}
}
}

#endif /* ORG_OPENSPLICE_CORE_REPORT_UTILS_HPP_ */
