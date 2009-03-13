/************************************************************************
 *  
 * Copyright (c) 2007
 * PrismTech Ltd.
 * All rights Reserved.
 * 
 * LOGICAL_NAME:    CheckStatus.c
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the C programming language.
 * DATE             june 2007.
 ************************************************************************
 * 
 * This file contains the implementation for the error handling operations.
 * 
 ***/

#include "CheckStatus.h"
#include "Windows.h"

/* Array to hold the names for all ReturnCodes. */
char *RetCodeName[13] = { 
    "DDS_RETCODE_OK",
    "DDS_RETCODE_ERROR",
    "DDS_RETCODE_UNSUPPORTED",
    "DDS_RETCODE_BAD_PARAMETER",
    "DDS_RETCODE_PRECONDITION_NOT_MET",
    "DDS_RETCODE_OUT_OF_RESOURCES",
    "DDS_RETCODE_NOT_ENABLED",
    "DDS_RETCODE_IMMUTABLE_POLICY",
    "DDS_RETCODE_INCONSISTENT_POLICY",
    "DDS_RETCODE_ALREADY_DELETED",
    "DDS_RETCODE_TIMEOUT",
    "DDS_RETCODE_NO_DATA",
    "DDS_RETCODE_ILLEGAL_OPERATION" };

/**
 * Returns the name of an error code.
 **/
char *getErrorName(DDS_ReturnCode_t status)
{
    return RetCodeName[status];
}

/**
 * Check the return status for errors. If there is an error, then terminate.
 **/
void checkStatus(
    DDS_ReturnCode_t status,
    const char *info)
{ 
    if ((status == DDS_RETCODE_OK) || (status == DDS_RETCODE_NO_DATA)) {
        return; /* no problems */
    }
    fprintf(stderr, "Error in %s: %s\n", info, getErrorName(status));
    /* If status==RETCODE_ALREADY_DELETED the application is already being
     * disconnected from the OpenSplice DDS domain
     */
    if (status != DDS_RETCODE_ALREADY_DELETED) {
        /* Generate Ctrl-C event to terminate process */
        GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
    }
    /* Wait in infinite loop as process will be terminated,
     * This is needed on windows as calling exit() or ExitProcess()
     * will fail to cleanup resources properly.
     */
    while (1) {
        Sleep(300000); /* 5 minutes */
    }
}

/**
 * Check whether a valid handle has been returned. If not, then terminate.
 **/
void checkHandle(
    void *handle,
    char *info )
{
     if (!handle) {
        fprintf(stderr, "Error in %s: Creation failed: invalid handle\n", info);
        /* Generate Ctrl-C event to terminate process */
        GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);

        /* Wait in infinite loop as process will be terminated,
         * This is needed on windows as calling exit() or ExitProcess()
         * will fail to cleanup resources properly.
         */
        while (1) {
            Sleep(300000); /* 5 minutes */
        }
     }
}
