/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <stdio.h>
#include <assert.h>

#include "os_report.h"

static void *
open_userClockModule (
    char *userClockModule
    )
{
    HINSTANCE handle;

    handle = LoadLibrary (userClockModule);
    if (handle == NULL) {
        OS_REPORT_1 (OS_ERROR, "open_userClockModule", 0,
            "LoadLibrary error: %d", GetLastError());
    }
    return (void *)handle;
}

static void *
find_userClockFunction (
    void *moduleHandle,
    const char *functionName
    )
{
    void *function = NULL;

    assert (moduleHandle);

    if ((functionName != NULL) &&
    	(strlen (functionName)) != 0) {
        function = GetProcAddress ((HINSTANCE)moduleHandle, functionName);
        if (function == NULL) {
            OS_REPORT_1 (OS_ERROR, "find_userClockFunction", 0,
                "GetProcAddress error: %d", GetLastError());
        }
    }
    return function;
}

static void
close_userClockModule (
    void *moduleHandle
    )
{
    if (moduleHandle) {
        if (!FreeLibrary ((HINSTANCE)moduleHandle)) {
            /* Zero is failure */
    	    OS_REPORT_1 (OS_ERROR, "close_userClockModule", 0,
                "FreeLibrary error: %d", GetLastError());
        }
    }
}
