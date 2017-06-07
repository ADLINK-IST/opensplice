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
#include "dds_dcps.h"
#include "sac_report.h"
#include "sac_object.h"
#include "os_report.h"

void
sac_panic(
    const os_char *function,
    os_int32 line,
    const os_char *file,
    const os_char *format,
    ...)
{
    os_char buffer[OS_REPORT_BUFLEN];
    static const os_char panic[] = "Panic: ";
    const os_size_t offset = sizeof(panic) - 1;
    va_list args;

    assert (function != NULL);
    assert (file != NULL);
    assert (format != NULL);

    (void)memcpy (buffer, panic, offset);
    va_start (args, format);
    (void)os_vsnprintf (buffer + offset, sizeof(buffer) - offset, format, args);
    va_end (args);

    os_report_noargs (OS_CRITICAL, function, file, line, DDS_RETCODE_ERROR, buffer);
    os_report_dumpStack (function, file, line);
}

void
sac_report(
    os_reportType reportType,
    const os_char *function,
    os_int32 line,
    const os_char *file,
    os_int32 code,
    const os_char *format,
    ...)
{
    const os_char *retcode = NULL;
    /* os_report truncates messages to <OS_REPORT_BUFLEN> bytes */
    os_char buffer[OS_REPORT_BUFLEN];
    os_size_t offset = 0;
    va_list args;

    assert (function != NULL);
    assert (file != NULL);
    assert (format != NULL);
    /* probably never happens, but you can never be to sure */
    assert (OS_REPORT_BUFLEN > 0);

    switch (code) {
        case DDS_RETCODE_ERROR:
            retcode = "Error: ";
            break;
        case DDS_RETCODE_UNSUPPORTED:
            retcode = "Unsupported: ";
            break;
        case DDS_RETCODE_BAD_PARAMETER:
            retcode = "Bad parameter: ";
            break;
        case DDS_RETCODE_PRECONDITION_NOT_MET:
            retcode = "Precondition not met: ";
            break;
        case DDS_RETCODE_OUT_OF_RESOURCES:
            retcode = "Out of resources: ";
            break;
        case DDS_RETCODE_NOT_ENABLED:
            retcode = "Not enabled: ";
            break;
        case DDS_RETCODE_IMMUTABLE_POLICY:
            retcode = "Immutable policy: ";
            break;
        case DDS_RETCODE_INCONSISTENT_POLICY:
            retcode = "Inconsistent policy: ";
            break;
        case DDS_RETCODE_ALREADY_DELETED:
            retcode = "Already deleted: ";
            break;
        case DDS_RETCODE_TIMEOUT:
            retcode = "Timeout: ";
            break;
        case DDS_RETCODE_NO_DATA:
            retcode = "No data: ";
            break;
        case DDS_RETCODE_ILLEGAL_OPERATION:
            retcode = "Illegal operation: ";
            break;
        default:
            assert (code == DDS_RETCODE_OK);
            break;
    }

    if (retcode != NULL) {
        assert (offset <= OS_REPORT_BUFLEN);
        offset = strlen(retcode);
        (void)memcpy(buffer, retcode, offset);
    }

    va_start (args, format);
    (void)os_vsnprintf (buffer + offset, sizeof(buffer) - offset, format, args);
    va_end (args);

    os_report_noargs (reportType, function, file, line, code, buffer);
}

void
sac_report_flush(
    DDS_Object object,
    DDS_boolean valid,
    const os_char *file,
    os_int32 line,
    const os_char *function)
{
    if (os_report_stack_flush_required(valid)) {
        os_int32 domainId = DDS_Object_get_domain_id(object);
        os_report_stack_unwind(valid, function, file, line, domainId);
    }
}
