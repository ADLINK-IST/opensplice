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
#include "dds_dcps_private.h"
#include "dds.h"
#include "dds_report.h"
#include "os_report.h"

void
dds_panic(
    const char *function,
    int32_t line,
    const char *file,
    const char *format,
    ...)
{
    char buffer[OS_REPORT_BUFLEN];
    va_list args;

    assert (function != NULL);
    assert (file != NULL);
    assert (format != NULL);

    snprintf(buffer, sizeof(buffer), "Panic: %s", format);

    va_start (args, format);
    os_report_va (OS_CRITICAL, function, file, line, DDS_RETCODE_ERROR, -1, OS_TRUE, buffer, args);
    os_report_dump(OS_TRUE, function, file, line, -1);
    va_end (args);
}

void
dds_report(
    os_reportType reportType,
    const char *function,
    int32_t line,
    const char *file,
    int32_t code,
    const char *format,
    ...)
{
    const char *retcode = NULL;
    /* os_report truncates messages to <OS_REPORT_BUFLEN> bytes */
    char buffer[OS_REPORT_BUFLEN];
    va_list args;

    assert (function != NULL);
    assert (file != NULL);
    assert (format != NULL);

    switch (code) {
    case DDS_RETCODE_ERROR:
        retcode = "Error";
    break;
    case DDS_RETCODE_UNSUPPORTED:
        retcode = "Unsupported";
    break;
    case DDS_RETCODE_BAD_PARAMETER:
        retcode = "Bad parameter";
    break;
    case DDS_RETCODE_PRECONDITION_NOT_MET:
        retcode = "Precondition not met";
    break;
    case DDS_RETCODE_OUT_OF_RESOURCES:
        retcode = "Out of resources";
    break;
    case DDS_RETCODE_NOT_ENABLED:
        retcode = "Not enabled";
    break;
    case DDS_RETCODE_IMMUTABLE_POLICY:
        retcode = "Immutable policy";
    break;
    case DDS_RETCODE_INCONSISTENT_POLICY:
        retcode = "Inconsistent policy";
    break;
    case DDS_RETCODE_ALREADY_DELETED:
        retcode = "Already deleted";
    break;
    case DDS_RETCODE_TIMEOUT:
        retcode = "Timeout";
    break;
    case DDS_RETCODE_NO_DATA:
        retcode = "No data";
    break;
    case DDS_RETCODE_ILLEGAL_OPERATION:
        retcode = "Illegal operation";
    break;
    default:
        assert (code == DDS_RETCODE_OK);
        retcode = "Unknown error";
    break;
    }

    snprintf(buffer, OS_REPORT_BUFLEN, "%s %s", retcode, format);

    va_start (args, format);
    os_report_va (reportType, function, file, line, code, -1, OS_TRUE, buffer, args);
    va_end (args);
}

int
dds_report_get_error_code()
{
    int result = DDS_RETCODE_ERROR;
    os_reportEvent report;

    report = os_report_read(0);
    if (report) {
        if ((report->code > 0) && (report->code <= DDS_RETCODE_ILLEGAL_OPERATION)) {
            result = report->code;
        }
    }
    return result;
}

void
dds_report_flush(
    void *object,
    bool valid,
    const char *file,
    int32_t line,
    const char *function)
{
	if (os_report_status(valid)) {
		os_int32 domainId = DDS_Object_get_domain_id(object);
		os_report_flush(valid, function, file, line, domainId);
	}
}
