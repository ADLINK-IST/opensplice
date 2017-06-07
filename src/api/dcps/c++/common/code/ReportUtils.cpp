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
#include "ReportUtils.h"
#include "CppSuperClass.h"
#include "os_defs.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_stdlib.h"

#include <stdarg.h>

os_char *
DDS::OpenSplice::Utils::pretty_function(
    const os_char *signature)
{
    os_char *begin = NULL, *end = NULL, *ptr;
    os_char *function = NULL;
    os_size_t length = 0;

    assert (signature != NULL);

    for (ptr = (os_char *)signature; end == NULL; ptr++) {
         if (*ptr == ' ' || *ptr == '\t') {
             begin = NULL;
         } else if (*ptr == '(' || *ptr == '\0') {
             end = ptr;
         } else if (begin == NULL) {
             begin = ptr;
         }
    }

    assert (begin != NULL);
    assert (end != NULL);
    assert (end >= begin);

    length = (end - begin);
    function = (os_char *)os_malloc(length + 1); /* + '\0' */
    if (function != NULL) {
        (void)memcpy(function, begin, length);
        function[length] = '\0';
    }

    return function;
}

void
DDS::OpenSplice::Utils::report_stack()
{
    os_report_stack();
}

void
DDS::OpenSplice::Utils::panic(
    const os_char *file,
    os_int32 line,
    const os_char *signature,
    const os_char *format,
    ...)
{
    os_char *function = NULL;
    /* os_report truncates messages to <OS_REPORT_BUFLEN> bytes */
    os_char buffer[OS_REPORT_BUFLEN];
    static const os_char panic[] = "Panic: ";
    const os_size_t offset = sizeof(panic) - 1;
    va_list args;

    assert (file != NULL);
    assert (signature != NULL);
    assert (format != NULL);
    /* probably never happens, but you can never be to sure */
    assert (OS_REPORT_BUFLEN > 0);

    va_start(args, format);
    (void)os_vsnprintf(buffer + offset, sizeof(buffer) - offset, format, args);
    va_end(args);

    function = DDS::OpenSplice::Utils::pretty_function(signature);
    if (function == NULL) {
        function = (os_char *)signature;
    }

    os_report_noargs(OS_CRITICAL, function, file, line, DDS::RETCODE_ERROR, buffer);
    os_report_dumpStack(function, file, line);

    if (function != NULL && function != signature) {
        os_free(function);
    }
}

void
DDS::OpenSplice::Utils::report(
    os_reportType reportType,
    const os_char *file,
    os_int32 line,
    const os_char *signature,
    DDS::ReturnCode_t code,
    const os_char *format,
    ...)
{
    os_char *function = NULL;
    const os_char *retcode = NULL;
    /* os_report truncates messages to <OS_REPORT_BUFLEN> bytes */
    os_char buffer[OS_REPORT_BUFLEN];
    os_size_t offset = 0;
    va_list args;

    assert (file != NULL);
    assert (signature != NULL);
    assert (format != NULL);
    /* probably never happens, but you can never be to sure */
    assert (OS_REPORT_BUFLEN > 0);

    switch (code) {
        case DDS::RETCODE_ERROR:
            retcode = "Error: ";
            break;
        case DDS::RETCODE_UNSUPPORTED:
            retcode = "Unsupported: ";
            break;
        case DDS::RETCODE_BAD_PARAMETER:
            retcode = "Bad parameter: ";
            break;
        case DDS::RETCODE_PRECONDITION_NOT_MET:
            retcode = "Precondition not met: ";
            break;
        case DDS::RETCODE_OUT_OF_RESOURCES:
            retcode = "Out of resources: ";
            break;
        case DDS::RETCODE_NOT_ENABLED:
            retcode = "Not enabled: ";
            break;
        case DDS::RETCODE_IMMUTABLE_POLICY:
            retcode = "Immutable policy: ";
            break;
        case DDS::RETCODE_INCONSISTENT_POLICY:
            retcode = "Inconsistent policy: ";
            break;
        case DDS::RETCODE_ALREADY_DELETED:
            retcode = "Already deleted: ";
            break;
        case DDS::RETCODE_TIMEOUT:
            retcode = "Timeout: ";
            break;
        case DDS::RETCODE_NO_DATA:
            retcode = "No data: ";
            break;
        case DDS::RETCODE_ILLEGAL_OPERATION:
            retcode = "Illegal operation: ";
            break;
        default:
            assert (code == DDS::RETCODE_OK);
            break;
    }

    if (retcode != NULL) {
        assert (offset <= OS_REPORT_BUFLEN);
        offset = strlen(retcode);
        (void)memcpy(buffer, retcode, offset);
    }

    va_start(args, format);
    (void)os_vsnprintf(buffer + offset, sizeof(buffer) - offset, format, args);
    va_end(args);

    function = DDS::OpenSplice::Utils::pretty_function(signature);
    if (function == NULL) {
        function = (os_char *)signature;
    }

    os_report_noargs(reportType, function, file, line, code, buffer);

    if (function != NULL && function != signature) {
        os_free(function);
    }
}

void
DDS::OpenSplice::Utils::report_flush(
    const os_char *file,
    os_int32 line,
    const os_char *signature,
    DDS::Boolean flush,
    CppSuperClassInterface *object)
{
    os_char *function = NULL;
    os_int32 domainId = DOMAIN_ID_INVALID;

    assert (file != NULL);
    assert (signature != NULL);

    if (os_report_stack_flush_required(flush ? OS_TRUE : OS_FALSE)) {
        function = DDS::OpenSplice::Utils::pretty_function(signature);
        if (function == NULL) {
            function = (os_char *)signature;
        }
        if (object) {
            domainId = object->getDomainId();
        }

        os_report_stack_unwind((os_boolean)flush, function, file, line, domainId);

        if (function != NULL && function != signature) {
            os_free(function);
        }
    }
}
