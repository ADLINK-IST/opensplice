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

#ifndef __EXAMPLE_ERROR_SAC_H__
#define __EXAMPLE_ERROR_SAC_H__

/**
 * @file
 * This file defines some simple error handling functions for use in the OpenSplice Standalone C examples.
 */

#include "dds_dcps.h"

static

#define INT_TO_STRING_MACRO(n) I_TO_STR_MACRO(n)
#define I_TO_STR_MACRO(n) #n
#define CHECK_STATUS_MACRO(returnCode) checkStatus (returnCode, " : Invalid return code at " __FILE__ ":" INT_TO_STRING_MACRO(__LINE__) )
#define CHECK_HANDLE_MACRO(handle) checkHandle (handle, "Failed to create entity : Invalid handle at " __FILE__ ":" INT_TO_STRING_MACRO(__LINE__) )
#define CHECK_ALLOC_MACRO(handle) checkHandle (handle, "Failed to allocate memory at " __FILE__ ":" INT_TO_STRING_MACRO(__LINE__) )

const char* ReturnCodeName[13] =
{
    "DDS_RETCODE_OK", "DDS_RETCODE_ERROR", "DDS_RETCODE_UNSUPPORTED",
    "DDS_RETCODE_BAD_PARAMETER", "DDS_RETCODE_PRECONDITION_NOT_MET",
    "DDS_RETCODE_OUT_OF_RESOURCES", "DDS_RETCODE_NOT_ENABLED",
    "DDS_RETCODE_IMMUTABLE_POLICY", "DDS_RETCODE_INCONSISTENT_POLICY",
    "DDS_RETCODE_ALREADY_DELETED", "DDS_RETCODE_TIMEOUT", "DDS_RETCODE_NO_DATA",
    "DDS_RETCODE_ILLEGAL_OPERATION"
};

/**
 * Function to convert DDS return codes into an error message on the standard output.
 * @param returnCode DDS return code
 * @param where A string detailing where the error occurred
 */
void checkStatus(DDS_ReturnCode_t returnCode, const char *where)
{
    if (returnCode && returnCode != DDS_RETCODE_NO_DATA)
    {
        fprintf(stderr, "%s%s\n", ReturnCodeName[returnCode], where);
        exit(1);
    }
}

/**
 * Function to check for a null handle and display an error message on the standard output.
 * @param Handle to check for null
 * @param A string detailing where the error occured
 */
void checkHandle(void *handle, const char *where)
{
    if (!handle)
    {
        fprintf(stderr, "%s\n", where);
        exit(1);
    }
}

#endif
