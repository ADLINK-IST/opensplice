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
#include "Vortex_FACE.hpp"
#include "Vortex/FACE/ReportSupport.hpp"
#include "Vortex/FACE/FaceInstance.hpp"

#include <string>


std::string
Vortex::FACE::returnCodeToString(::FACE::RETURN_CODE_TYPE return_code)
{
    std::string ret("unknown");
    switch (return_code) {
        case ::FACE::NO_ERROR:               ret = "NO_ERROR"; break;
        case ::FACE::NO_ACTION:              ret = "NO_ACTION"; break;
        case ::FACE::NOT_AVAILABLE:          ret = "NOT_AVAILABLE"; break;
        case ::FACE::ADDR_IN_USE:            ret = "ADDR_IN_USE"; break;
        case ::FACE::INVALID_PARAM:          ret = "INVALID_PARAM"; break;
        case ::FACE::INVALID_CONFIG:         ret = "INVALID_CONFIG"; break;
        case ::FACE::PERMISSION_DENIED:      ret = "PERMISSION_DENIED"; break;
        case ::FACE::INVALID_MODE:           ret = "INVALID_MODE"; break;
        case ::FACE::TIMED_OUT:              ret = "TIMED_OUT"; break;
        case ::FACE::MESSAGE_STALE:          ret = "MESSAGE_STALE"; break;
        case ::FACE::CONNECTION_IN_PROGRESS: ret = "CONNECTION_IN_PROGRESS"; break;
        case ::FACE::CONNECTION_CLOSED:      ret = "CONNECTION_CLOSED"; break;
        case ::FACE::DATA_BUFFER_TOO_SMALL:  ret = "DATA_BUFFER_TOO_SMALL"; break;
    }
    return ret;
}


::FACE::RETURN_CODE_TYPE
Vortex::FACE::exceptionToReturnCode(const dds::core::Exception& e)
{
    ::FACE::RETURN_CODE_TYPE status = ::FACE::NO_ACTION;

    if (dynamic_cast<const dds::core::InvalidArgumentError*>(&e)) {
        status = ::FACE::INVALID_PARAM;
    } else if (dynamic_cast<const dds::core::TimeoutError*>(&e)) {
        status = ::FACE::TIMED_OUT;
    } else if (dynamic_cast<const dds::core::UnsupportedError*>(&e)) {
        status = ::FACE::NOT_AVAILABLE;
    } else if (dynamic_cast<const dds::core::AlreadyClosedError*>(&e)) {
        status = ::FACE::CONNECTION_CLOSED;
    } else if (dynamic_cast<const dds::core::ImmutablePolicyError*>(&e)) {
        status = ::FACE::INVALID_CONFIG;
    } else if (dynamic_cast<const dds::core::InconsistentPolicyError*>(&e)) {
        status = ::FACE::INVALID_CONFIG;
    } else if (dynamic_cast<const dds::core::OutOfResourcesError*>(&e)) {
        status = ::FACE::DATA_BUFFER_TOO_SMALL;
    }

    return status;
}

static std::string&
pretty_function(
    std::string& s)
{
    /* Last part is erased before the first part to
     * be able to prettify template functions. */

    /* Erase last part of the function name. */
    int end = s.find_first_of("(");
    if (end > 0) {
        s.erase(end);
    }

    /* Erase first part of the function name. */
    int last_tab = s.find_last_of("\t");
    int tmpl_space = s.rfind(", ");
    int last_space;
    if ((int)tmpl_space > 0) {
        last_space = s.find_last_of(" ", tmpl_space - 1);
    } else {
        last_space = s.find_last_of(" ");
    }
    if (((int)last_tab > 0) && (last_tab > last_space)) {
        s.erase(0, last_tab + 1);
    }
    if ((last_space > 0) && (last_space > last_tab)) {
        s.erase(0, last_space + 1);
    }

    return s;
}


void
Vortex::FACE::report_stack_open(
    const char *file,
    int32_t line,
    const char *signature)
{
    os_report_stack_open(file, line, signature, NULL);
}

void
Vortex::FACE::report_stack_close(
    ::FACE::CONNECTION_ID_TYPE connectionId,
    const char *file,
    int32_t line,
    const char *signature,
    bool flush)
{
    const char *function = NULL;

    assert (file != NULL);
    assert (signature != NULL);

    if (os_report_stack_flush_required((os_boolean)flush)) {
        const char *_file = file;
        int32_t _line = line;
        const char *_signature = signature;
        int32_t domainId = -1;

        (void)os_report_get_context(&_file, (os_int *)&_line, &_signature, NULL);
        std::string s(_signature);
        if (!pretty_function(s).empty()) {
            function = s.c_str();
        } else {
            function = _signature;
        }

        Vortex::FACE::FaceInstance::shared_ptr instance = Vortex::FACE::FaceInstance::getInstance();
        if (instance) {
            domainId = instance->getDomainId();
            if ((domainId == (int32_t)org::opensplice::domain::default_id()) && (connectionId != -1)) {
                Vortex::FACE::AnyConnection::shared_ptr con = instance->getConnection(connectionId);
                if (con.get()) {
                    domainId = con->getDomainId();
                }
            }
        }
        os_report_stack_unwind((os_boolean)flush, function, _file, _line, domainId);
    }
}



void
Vortex::FACE::report(
    ::FACE::RETURN_CODE_TYPE code,
    os_reportType reportType,
    const char *file,
    int32_t line,
    const char *signature,
    const char *format,
    ...)
{
    char buffer[OS_REPORT_BUFLEN];
    const char *function;
    std::string retcode;
    os_size_t offset = 0;
    va_list args;

    assert (file != NULL);
    assert (signature != NULL);
    assert (format != NULL);

    /* Prepare error description. */
    retcode = returnCodeToString(code);
    offset = snprintf(buffer, OS_REPORT_BUFLEN, "%s: ", retcode.c_str());

    va_start(args, format);
    (void)os_vsnprintf(buffer + offset, sizeof(buffer) - offset, format, args);
    va_end(args);

    /* Prettify function name. */
    std::string s(signature);
    if (!pretty_function(s).empty()) {
        function = s.c_str();
    } else {
        function = signature;
    }

    /* Add this report to the logs. */
    os_report_noargs(reportType, function, file, line, (os_int32)code, buffer);
}
