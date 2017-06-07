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

#ifndef __EXAMPLE_ERROR_SACPP_H__
#define __EXAMPLE_ERROR_SACPP_H__

/**
 * @file
 * This file defines some simple error handling functions for use in the OpenSplice Standalone C++ examples.
 */

#include "ccpp_dds_dcps.h"

namespace {

#define INT_TO_STRING_MACRO(n) I_TO_STR_MACRO(n)
#define I_TO_STR_MACRO(n) #n
#define CHECK_STATUS_MACRO(returnCode) checkStatus (returnCode, " : Invalid return code at " __FILE__ ":" INT_TO_STRING_MACRO(__LINE__) )
#define CHECK_HANDLE_MACRO(handle) checkHandle (handle, "Failed to create entity : Invalid handle at " __FILE__ ":" INT_TO_STRING_MACRO(__LINE__) )

const char* ReturnCodeName[13] =
{
    "DDS::RETCODE_OK", "DDS::RETCODE_ERROR", "DDS::RETCODE_UNSUPPORTED",
    "DDS::RETCODE_BAD_PARAMETER", "DDS::RETCODE_PRECONDITION_NOT_MET",
    "DDS::RETCODE_OUT_OF_RESOURCES", "DDS::RETCODE_NOT_ENABLED",
    "DDS::RETCODE_IMMUTABLE_POLICY", "DDS::RETCODE_INCONSISTENT_POLICY",
    "DDS::RETCODE_ALREADY_DELETED", "DDS::RETCODE_TIMEOUT", "DDS::RETCODE_NO_DATA",
    "DDS::RETCODE_ILLEGAL_OPERATION"
};

/**
 * Function to convert DDS return codes into an exception with meaningful output.
 * @param returnCode DDS return code
 * @param where A string detailing where the error occurred
 */
void checkStatus(DDS::ReturnCode_t returnCode, const char *where)
{
    if (returnCode && returnCode != DDS::RETCODE_NO_DATA)
    {
        size_t length = strlen(ReturnCodeName[returnCode]) + strlen (where);
        char* buffer = DDS::string_alloc((DDS::ULong)length);
        DDS::String_var exception = buffer;
        snprintf(buffer, length + 1, "%s%s", ReturnCodeName[returnCode], where);
        throw exception;
    }
}

/**
 * Function to check for a null handle and throw an exception with meaningful output.
 * @param Handle to check for null
 * @param A string detailing where the error occured
 */
void checkHandle(void *handle, const char *where)
{
    if (!handle)
    {
        size_t length = strlen (where);
        char* buffer = DDS::string_alloc((DDS::ULong)length);
        DDS::String_var exception = buffer;
        snprintf(buffer, length + 1, "%s", where);
        throw exception;
    }
}

}

#endif
