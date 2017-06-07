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

#include "ErrorInfo.h"
#include "os_report.h"
#include "cmn_errorInfo.h"

DDS::ErrorInfo::ErrorInfo( void ) :
    DDS::OpenSplice::CppSuperClass(DDS::OpenSplice::ERRORINFO),
    valid(FALSE)
{
    DDS::OpenSplice::CppSuperClass::nlReq_init();
}

DDS::ErrorInfo::~ErrorInfo( void )
{
    (void) deinit();
}

DDS::ReturnCode_t
DDS::ErrorInfo::wlReq_deinit(
    void
) THROW_ORB_EXCEPTIONS
{
    return DDS::OpenSplice::CppSuperClass::wlReq_deinit();
}

DDS::ReturnCode_t
DDS::ErrorInfo::update(
    void
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    os_reportInfo *osInfo;

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        osInfo = os_reportGetApiInfo();
        if (osInfo != NULL) {
            this->source_line = (char*)NULL;
            if (osInfo->sourceLine != NULL) {
                this->source_line = DDS::string_dup(osInfo->sourceLine);
            }
            this->stack_trace = (char*)NULL;
            if (osInfo->callStack != NULL) {
                this->stack_trace = DDS::string_dup(osInfo->callStack);
            }
            this->message = (char*)NULL;
            if (osInfo->description != NULL) {
                this->message = DDS::string_dup(osInfo->description);
            }
            this->location = (char*)NULL;
            if (osInfo->reportContext != NULL) {
                this->location = DDS::string_dup(osInfo->reportContext);
            }

            this->code = static_cast<DDS::ReturnCode_t>(
                cmn_errorInfo_reportCodeToCode (osInfo->reportCode));
            this->valid = TRUE;
        } else {
            this->valid = FALSE;
            result = DDS::RETCODE_NO_DATA;
        }

        this->unlock();
    }
    return result;
}

DDS::ReturnCode_t
DDS::ErrorInfo::get_code(
    DDS::ReturnCode_t & code
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    result = this->read_lock();
    if (result == DDS::RETCODE_OK) {
        if (this->valid) {
            code = this->code;
        } else {
            result = DDS::RETCODE_NO_DATA;
        }

        this->unlock();
    }
    return result;
}

DDS::ReturnCode_t
DDS::ErrorInfo::get_message(
    char * & message
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    result = this->read_lock();
    if (result == DDS::RETCODE_OK) {
        if (this->valid) {
            if (message != NULL) {
                DDS::string_free(message);
            }
            if (this->message != NULL) {
                message = DDS::string_dup(this->message);
            } else {
                message = NULL;
            }
        } else {
            result = DDS::RETCODE_NO_DATA;
        }

        this->unlock();
    }
    return result;
}

DDS::ReturnCode_t
DDS::ErrorInfo::get_location(
    char * & location
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    result = this->read_lock();
    if (result == DDS::RETCODE_OK) {
        if (this->valid) {
            if (location != NULL) {
                DDS::string_free(location);
            }
            if (this->location != NULL) {
                location = DDS::string_dup(this->location);
            } else {
                location = NULL;
            }
        } else {
            result = DDS::RETCODE_NO_DATA;
        }

        this->unlock();
    }
    return result;
}

DDS::ReturnCode_t
DDS::ErrorInfo::get_source_line(
    char * & source_line
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    result = this->read_lock();
    if (result == DDS::RETCODE_OK) {
        if (this->valid) {
            if (source_line != NULL) {
                DDS::string_free(source_line);
            }
            if (this->source_line != NULL) {
                source_line = DDS::string_dup(this->source_line);
            } else {
                source_line = NULL;
            }
        } else {
            result = DDS::RETCODE_NO_DATA;
        }

        this->unlock();
    }
    return result;
}

DDS::ReturnCode_t
DDS::ErrorInfo::get_stack_trace(
    char * & stack_trace
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    result = this->read_lock();
    if (result == DDS::RETCODE_OK) {
        if (this->valid) {
            if (stack_trace != NULL) {
                DDS::string_free(stack_trace);
            }
            if (this->stack_trace != NULL) {
                stack_trace = DDS::string_dup(this->stack_trace);
            } else {
                stack_trace = NULL;
            }
        } else {
            result = DDS::RETCODE_NO_DATA;
        }

        this->unlock();
    }
    return result;
}
