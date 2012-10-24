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

/* Implements user clock support functions, generic for UNIX
 * operating systems that support dlopen, dlsym, dlerror and dlclose
 */

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "os_report.h"

static void *
open_userClockModule (
    char *userClockModule
    )
{
    void *handle;
    
    handle = dlopen (userClockModule, RTLD_LAZY | RTLD_NODELETE);
    if (handle == NULL) {
        OS_REPORT_1 (OS_ERROR, "open_userClockModule", 0,
            "dlopen error: %s", dlerror());
    }
    return handle;
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
        function = dlsym (moduleHandle, functionName);
        if (function == NULL) {
            OS_REPORT_1 (OS_ERROR, "find_userClockFunction", 0,
                "dlsym error: %s", dlerror());
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
    	if (dlclose (moduleHandle) != 0) {
    	    OS_REPORT_1 (OS_ERROR, "close_userClockModule", 0,
                "dlclose error: %s", dlerror());
        }
    }
}
