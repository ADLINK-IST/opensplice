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
#include <org/opensplice/core/ReportUtils.hpp>

#include <os_stdlib.h>
#include <os_report.h>
#include <os_version.h>
#include <os_string.h>

#include <string>
#include <map>

namespace org
{
namespace opensplice
{
namespace core
{
namespace utils
{

OMG_DDS_API
std::string&
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


OMG_DDS_API
std::string
convert_classname(
    const char *c)
{
    uint32_t index1, index2;
    std::string s = c;

    s = pretty_function(s);

    index1 = s.rfind("<DELEGATE>");
    if ((int)index1 > 0) {
        index2 = s.rfind("::T", index1);
        s.erase(index1, 10);
        s.erase(index2+2, 1);
    } else {
        index1 = s.rfind(", DELEGATE>");
        if ((int)index1 > 0) {
            s.erase(index1, 10);
        } else {
            index1 = s.find("org::opensplice");
            if (index1 == 0) {
                index1 = s.rfind("Delegate");
                if ((int)index1 > 0) {
                    index2 = s.rfind("Delegate::", index1);
                    if ((int)index2 > 0) {
                        s.erase(index1, 8);
                        s.erase(index2, 8);
                        s.replace(0, 16, "dds");
                    }
                }
            }
        }
    }

    return s;
}

class CodeToString {
public:
    static const char *
    get_prefix(int32_t code) {
        switch (code) {
            case error_code:
                return "Error: ";
            case unsupported_error_code:
                return "Unsupported: ";
            case invalid_argument_code:
                return "Bad parameter: ";
            case precondition_not_met_error_code:
                return "Precondition not met: ";
            case out_of_resources_error_code:
                return "Out of resources: ";
            case not_enabled_error_code:
                return "Not enabled: ";
            case immutable_policy_error_code:
                return "Immutable policy: ";
            case inconsistent_policy_error_code:
                return "Inconsistent policy: ";
            case already_closed_error_code:
                return "Already deleted: ";
            case timeout_error_code:
                return "Timeout: ";
            case illegal_operation_error_code:
                return "Illegal operation: ";
            case null_reference_error_code:
                return "Null reference: ";
            default:
                return "Warning: ";
        }
    }
};

class ExceptionFactory {
public:
    ExceptionFactory() : code(error_code), line(-1), signature(NULL), domainId(-1) {}
    ~ExceptionFactory() {}

    void prepare(int32_t _code,
                 const char *_file,
                 int32_t _line,
                 const char *_signature,
                 const char *_descr)
    {
        assert (_file != NULL);
        assert (_signature != NULL);
        assert (_descr != NULL);

        this->code = _code;
        this->file = _file;
        this->line = _line;
        this->signature = _signature;

        /* Prettify originating method name for context and logs. */
        this->function = _signature;
        this->function = pretty_function(this->function);
        if (this->function.empty()) {
            this->function = _signature;
        }

        /* Prepare exception description and context. */
        prepare_description(_descr);

        prepare_context();
    }

    void throw_exception()
    {
        /* Add this exception as error to the logs. */
        os_report_noargs(OS_ERROR,
                         this->function.c_str(),
                         this->file.c_str(),
                         this->line,
                         this->code,
                         this->description.c_str());

        /* Get reports into exception info before flushing the reports. */
        get_reports();

        /* Flush reports before throwing the exception. */
        os_report_flush_unconditional(
                        (os_boolean) (this->code != timeout_error_code),
                        this->method.c_str(),
                        this->file.c_str(),
                        this->line,
                        this->domainId);

        /* Last but not least: throw proper exception type. */
        switch (this->code) {
            case error_code:
                throw dds::core::Error(this->message());
            case unsupported_error_code:
                throw dds::core::UnsupportedError(this->message());
            case invalid_argument_code:
                throw dds::core::InvalidArgumentError(this->message());
            case precondition_not_met_error_code:
                throw dds::core::PreconditionNotMetError(this->message());
            case out_of_resources_error_code:
                throw dds::core::OutOfResourcesError(this->message());
            case not_enabled_error_code:
                throw dds::core::NotEnabledError(this->message());
            case immutable_policy_error_code:
                throw dds::core::ImmutablePolicyError(this->message());
            case inconsistent_policy_error_code:
                throw dds::core::InconsistentPolicyError(this->message());
            case already_closed_error_code:
                throw dds::core::AlreadyClosedError(this->message());
            case timeout_error_code:
                throw dds::core::TimeoutError(this->message());
            case illegal_operation_error_code:
                throw dds::core::IllegalOperationError(this->message());
            case null_reference_error_code:
                throw dds::core::NullReferenceError(this->message());
            default:
                assert(false);
                break;
        }
    }

    std::string message()
    {
        std::stringstream tmp;
        tmp << this->description << "\n" <<
               this->context <<
               this->reports;
        return tmp.str();
    }

private:
    void prepare_description(const char *descr)
    {
        /* Prepend prefix to exception description. */
        this->description = CodeToString::get_prefix(this->code);
        this->description += descr;
    }

    void prepare_context()
    {
        /* Get and set context information. */
        os_timeW ostime;
        char node[64];
        char date_time[128];
        std::stringstream tmp;
        const char *_file;
        int32_t _line;
        const char *_signature = NULL;
        void *objRef;

        /* Get the context signature from the os report stack. */
        if (os_report_get_context(&_file, (os_int *)&_line, &_signature, &objRef)) {
            /* Convert internal classname to external for context and logs. */
            this->method = convert_classname(_signature);
            if (objRef) {
                org::opensplice::core::ObjectDelegate *obj = reinterpret_cast<org::opensplice::core::ObjectDelegate *>(objRef);
                if (obj) {
                    this->domainId = obj->get_domain_id();
                }
            }
        } else {
            /* We don't have a context class (this can happen when
             * ISOCPP_REPORT_STACK_DELEGATE_BEGIN() is not used).
             * Just use the prettified function as context. */
            this->method = this->function;
        }
        if (this->method.empty()) {
            this->method = this->signature;
        }

        ostime = os_timeWGet();
        os_ctimeW_r(&ostime, date_time, sizeof(date_time));

        if (os_gethostname(node, sizeof(node)-1) == os_resultSuccess) {
           node[sizeof(node)-1] = '\0';
        } else {
           os_strcpy(node, "UnkownNode");
        }

        tmp << "========================================================================================\n" <<
               "Context     : " << this->method.c_str() << "\n" <<
               "Date        : " << date_time << "\n" <<
               "Node        : " << node << "\n";
        this->context = tmp.str();
    }

    void get_reports()
    {
        const char *file;
        int32_t i;
        std::stringstream tmp;
        os_reportEventV1 report;
        int32_t stack_size = os_report_stack_size();

        if (stack_size <= 0) {
            /* No proper stack:
             * Try to create a meaningfull 'internals' ourselves. */
            file = os_strrchrs(this->file.c_str(), os_fileSep(), OS_TRUE);
            if (file == NULL) {
                file = (const char*)this->file.c_str();
            } else {
                file++;
            }
            tmp.str("");
            tmp << "Internals   : " << file << "/" << this->line << "/" << OSPL_VERSION_STR << "\n";
            this->context += tmp.str();
        } else {
            /* Go through stack reversed to get proper order. */
            for (i = stack_size - 1; i >= 0; i--)
            {
                /* Extract and add information of current report. */
                report = os_report_read(i);
                assert(report);
                tmp.str("");
                tmp << "----------------------------------------------------------------------------------------\n" <<
                       "Report      : " << report->description << "\n" <<
                       "Internals   : " << report->reportContext << "/" << report->fileName << "/" << report->lineNo << "\n";
                this->reports += tmp.str();

                /* Add some more context of the last inserted report. */
                if (i == (stack_size - 1)) {
                    tmp.str("");
                    tmp << "Process     : " << report->processDesc << "\n" <<
                           "Thread      : " << report->threadDesc << "\n" <<
                           "Internals   : " << report->fileName << "/" << report->lineNo << "/" << OSPL_VERSION_STR << "\n";
                    this->context += tmp.str();
                }
            }
        }
    }

private:
    std::string description;
    std::string context;
    std::string reports;

    std::string function;
    std::string method;
    std::string file;

    const char *signature;
    int32_t code;
    int32_t line;
    int32_t domainId;
};




ReportFinisher::~ReportFinisher()
{
    const char *function = NULL;

    if (os_report_stack_flush_required(OS_FALSE)) {
        const char *_file;
        int32_t _line;
        const char *_signature;
        void *objRef;
        int32_t domainId = -1;

        (void)os_report_get_context(&_file, (os_int *)&_line, &_signature, &objRef);
        std::string s(_signature);
        if (!pretty_function(s).empty()) {
            function = s.c_str();
        } else {
            function = _signature;
        }
        if (objRef) {
            org::opensplice::core::ObjectDelegate *obj = reinterpret_cast<org::opensplice::core::ObjectDelegate *>(objRef);
            if (obj) {
            domainId = obj->get_domain_id();
            }
        }
        os_report_stack_unwind(OS_TRUE, function, _file, _line, domainId);
    }
}




}
}
}
}


void
org::opensplice::core::utils::report_stack_open(
    const org::opensplice::core::ObjectDelegate *objRef,
    const char *file,
    int32_t line,
    const char *signature)
{
    void *ptr = reinterpret_cast<void *>((ObjectDelegate *)objRef);
    os_report_stack_open(file, line, signature, ptr);
}

void
org::opensplice::core::utils::report_stack_close(
    const char *file,
    int32_t line,
    const char *signature,
    bool flush)
{
    const char *function = NULL;

    assert (file != NULL);
    assert (signature != NULL);

    if (os_report_stack_flush_required((flush ? OS_TRUE : OS_FALSE))) {
        const char *_file = file;
        int32_t _line = line;
        const char *_signature = signature;
        void *objRef;
        int32_t domainId = -1;

        (void)os_report_get_context(&_file, (os_int *)&_line, &_signature, &objRef);
        std::string s(_signature);
        if (!pretty_function(s).empty()) {
            function = s.c_str();
        } else {
            function = _signature;
        }
        if (objRef) {
            org::opensplice::core::ObjectDelegate *obj = reinterpret_cast<org::opensplice::core::ObjectDelegate *>(objRef);
            domainId = obj->get_domain_id();
        }
        os_report_stack_unwind((flush ? OS_TRUE : OS_FALSE), function, file, _line, domainId);
    }
}

void
org::opensplice::core::utils::report(
    int32_t code,
    os_reportType reportType,
    const char *file,
    int32_t line,
    const char *signature,
    const char *format,
    ...)
{
    char buffer[OS_REPORT_BUFLEN];
    const char *function;
    const char *retcode;
    os_size_t offset = 0;
    va_list args;

    assert (file != NULL);
    assert (signature != NULL);
    assert (format != NULL);

    /* Prepare error description. */
    retcode = CodeToString::get_prefix(code);
    if (retcode != NULL) {
        offset = strlen(retcode);
        assert (offset <= OS_REPORT_BUFLEN);
        (void)memcpy(buffer, retcode, offset);
    }
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
    os_report_noargs(reportType, function, file, line, code, buffer);
}


void
org::opensplice::core::utils::throw_exception(
    int32_t code,
    const char *file,
    os_int32 line,
    const char *signature,
    const char *format,
    ...)
{
    char description[OS_REPORT_BUFLEN];
    ExceptionFactory factory;

    va_list args;
    va_start(args, format);
    (void)os_vsnprintf(description, sizeof(description), format, args);
    va_end(args);

    factory.prepare(code, file, line, signature, description);
    factory.throw_exception();
}


void
org::opensplice::core::utils::check_u_result_and_throw_exception(
    u_result code,
    const os_char *file,
    os_int32 line,
    const os_char *signature,
    const os_char *format,
    ...)
{
    if (code != U_RESULT_OK && code != U_RESULT_NO_DATA) {
        char description[OS_REPORT_BUFLEN];
        ExceptionFactory factory;

        va_list args;
        va_start(args, format);
        (void)os_vsnprintf(description, sizeof(description), format, args);
        va_end(args);

        switch (code) {
        case U_RESULT_TIMEOUT:
            factory.prepare(ISOCPP_TIMEOUT_ERROR, file, line, signature, description);
            break;
        case U_RESULT_UNSUPPORTED:
            factory.prepare(ISOCPP_UNSUPPORTED_ERROR, file, line, signature, description);
            break;
        case U_RESULT_PRECONDITION_NOT_MET:
            factory.prepare(ISOCPP_PRECONDITION_NOT_MET_ERROR, file, line, signature, description);
            break;
        case U_RESULT_IMMUTABLE_POLICY:
            factory.prepare(ISOCPP_IMMUTABLE_POLICY_ERROR, file, line, signature, description);
            break;
        case U_RESULT_INCONSISTENT_QOS:
            factory.prepare(ISOCPP_INCONSISTENT_POLICY_ERROR, file, line, signature, description);
            break;

        case U_RESULT_ILL_PARAM:
        case U_RESULT_CLASS_MISMATCH:
            factory.prepare(ISOCPP_INVALID_ARGUMENT_ERROR, file, line, signature, description);
            break;

        case U_RESULT_OUT_OF_MEMORY:
        case U_RESULT_OUT_OF_RESOURCES:
            factory.prepare(ISOCPP_OUT_OF_RESOURCES_ERROR, file, line, signature, description);
            break;

        case U_RESULT_HANDLE_EXPIRED:
        case U_RESULT_ALREADY_DELETED:
            factory.prepare(ISOCPP_ALREADY_CLOSED_ERROR, file, line, signature, description);
            break;

        case U_RESULT_DETACHING:
        case U_RESULT_INTERNAL_ERROR:
        case U_RESULT_INTERRUPTED:
        case U_RESULT_NOT_INITIALISED:
        case U_RESULT_UNDEFINED:
        default:
            factory.prepare(ISOCPP_ERROR, file, line, signature, description);
            break;
        }

        factory.throw_exception();
    }
}



void
org::opensplice::core::utils::check_os_result_and_throw_exception(
    os_result code,
    const os_char *file,
    os_int32 line,
    const os_char *signature,
    const os_char *format,
    ...)
{
    if (code != os_resultSuccess) {
        char description[OS_REPORT_BUFLEN];
        ExceptionFactory factory;

        va_list args;
        va_start(args, format);
        (void)os_vsnprintf(description, sizeof(description), format, args);
        va_end(args);

        switch (code) {
        case os_resultTimeout:
            factory.prepare(ISOCPP_TIMEOUT_ERROR, file, line, signature, description);
            break;
        case os_resultInvalid:
            factory.prepare(ISOCPP_INVALID_ARGUMENT_ERROR, file, line, signature, description);
            break;

        case os_resultFail:
        case os_resultBusy:
        case os_resultUnavailable:
        default:
            factory.prepare(ISOCPP_ERROR, file, line, signature, description);
            break;
        }

        factory.throw_exception();
    }
}
