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


/**
 * @file
 */

#include <dds/core/Exception.hpp>
#include <sstream>

#include <org/opensplice/core/exception_helper.hpp>

namespace
{
const char* spec_return_codes[13] =
{
    "DDS::RETCODE_OK",
    "DDS::RETCODE_ERROR",
    "DDS::RETCODE_UNSUPPORTED",
    "DDS::RETCODE_BAD_PARAMETER",
    "DDS::RETCODE_PRECONDITION_NOT_MET",
    "DDS::RETCODE_OUT_OF_RESOURCES",
    "DDS::RETCODE_NOT_ENABLED",
    "DDS::RETCODE_IMMUTABLE_POLICY",
    "DDS::RETCODE_INCONSISTENT_POLICY",
    "DDS::RETCODE_ALREADY_DELETED",
    "DDS::RETCODE_TIMEOUT",
    "DDS::RETCODE_NO_DATA",
    "DDS::RETCODE_ILLEGAL_OPERATION"
};

const std::size_t spec_return_codes_size = (sizeof(spec_return_codes) / sizeof(spec_return_codes[0]));

void populate(std::string& message,
              bool ospl_error_info,
              bool /* stack_trace */)
{
    if(ospl_error_info)
    {
        try
        {
            std::string tmp = message + "\n  Preceding OpenSplice Error Information : ";
            std::string not_available = "Not available - ";
            DDS::ErrorInfo tss_error_interface;
            DDS::ReturnCode_t result = tss_error_interface.update();
            if(result == DDS::RETCODE_OK)
            {
                DDS::ReturnCode_t ospl_error_code = -1;
                DDS::String_var error_info_string;
                result = tss_error_interface.get_code(ospl_error_code);
                tmp += "\n    Error code : ";
                tmp += (result == DDS::RETCODE_OK && ospl_error_code >= 0
                        && static_cast<std::size_t>(ospl_error_code) < spec_return_codes_size
                        ? spec_return_codes[ospl_error_code]
                        :    not_available + (ospl_error_code < 0 || static_cast<std::size_t>(ospl_error_code) > spec_return_codes_size
                                              ? "value out of known range"
                                              : spec_return_codes[result]));
                result = tss_error_interface.get_message(error_info_string.out());
                tmp += "\n    Message: ";
                tmp += (result == DDS::RETCODE_OK
                        ? error_info_string.in()
                        : not_available + spec_return_codes[result]);
                result = tss_error_interface.get_location(error_info_string.out());
                tmp += "\n    Location: ";
                tmp += (result == DDS::RETCODE_OK
                        ? error_info_string.in()
                        : not_available + spec_return_codes[result]);
                result = tss_error_interface.get_source_line(error_info_string.out());
                tmp += "\n    Source line: ";
                tmp += (result == DDS::RETCODE_OK
                        ? error_info_string.in()
                        : not_available + spec_return_codes[result]);
                //result = tss_error_interface.get_stack_trace(error_info_string.out());
                //tmp += "\n    Pseudo stack trace: ";
                //tmp += (result == DDS::RETCODE_OK
                //? error_info_string.in()
                //: not_available + spec_return_codes[result]);
            }
            else
            {
                tmp += not_available + spec_return_codes[result];
            }
            message.swap(tmp);
        }
        catch(...)
        {
            // Probably out of mem. Not much we can do.
        }
    }

    /** @internal @todo Proper stack trace */
}
}

namespace org
{
namespace opensplice
{
namespace core
{

std::string exception_helper(const char* message,
                             const char* function,
                             bool ospl_error_info,
                             bool stack_trace)
{
    std::string str(message);
    str += function;
    populate(str, ospl_error_info, stack_trace);
    return str;
}

std::string exception_helper(const std::string& message,
                             bool ospl_error_info,
                             bool stack_trace)
{
    std::string str(message);
    populate(str, ospl_error_info, stack_trace);
    return str;
}

std::string dds_return_code_to_string(DDS::ReturnCode_t code)
{
    std::string result(code >= 0 && static_cast<std::size_t>(code) < spec_return_codes_size ? spec_return_codes[code] : "out of range / unknown code");
    return result;
}

void check_and_throw_impl(DDS::ReturnCode_t code,
                          const std::string& context)
{
    if(code && code != DDS::RETCODE_NO_DATA)
    {
        std::string message = ". DDS API call returned ";
        message += dds_return_code_to_string(code);
        switch(code)
        {
            case DDS::RETCODE_ERROR:
                throw dds::core::Error(exception_helper("dds::core::Error : " + context + message));
            case DDS::RETCODE_UNSUPPORTED:
                throw dds::core::UnsupportedError(exception_helper("dds::core::UnsupportedError : " + context + message));
            case DDS::RETCODE_BAD_PARAMETER:
                throw dds::core::InvalidArgumentError(exception_helper("dds::core::InvalidArgumentError : " + context + message));
            case DDS::RETCODE_PRECONDITION_NOT_MET:
                throw dds::core::PreconditionNotMetError(exception_helper("dds::core::PreconditionNotMetError : " + context + message));
            case DDS::RETCODE_OUT_OF_RESOURCES:
                throw dds::core::OutOfResourcesError(exception_helper("dds::core::OutOfResourcesError : " + context + message));
            case DDS::RETCODE_NOT_ENABLED:
                throw dds::core::NotEnabledError(exception_helper("dds::core::NotEnabledError : " + context + message));
            case DDS::RETCODE_IMMUTABLE_POLICY:
                throw dds::core::ImmutablePolicyError(exception_helper("dds::core::ImmutablePolicyError : " + context + message));
            case DDS::RETCODE_INCONSISTENT_POLICY:
                throw dds::core::InconsistentPolicyError(exception_helper("dds::core::InconsistentPolicyError : " + context + message));
            case DDS::RETCODE_ALREADY_DELETED:
                throw dds::core::AlreadyClosedError(exception_helper("dds::core::AlreadyClosedError : " + context + message));
            case DDS::RETCODE_TIMEOUT:
                throw dds::core::TimeoutError(exception_helper("dds::core::TimeoutError : " + context + message));
            case DDS::RETCODE_ILLEGAL_OPERATION:
                throw dds::core::IllegalOperationError(exception_helper("dds::core::IllegalOperationError : " + context + message));
            default:
                std::stringstream str_build("dds::core::IllegalOperationError : " + context + message + ". Unknown return value is ");
                str_build << code;
                throw dds::core::IllegalOperationError(exception_helper(str_build.str()));
        }
    }
}
}
}
}
