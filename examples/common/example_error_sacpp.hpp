/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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
