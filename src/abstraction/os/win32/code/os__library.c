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
#include <stdio.h>
#include <assert.h>
#include "os_library.h"
#include "os_stdlib.h"
#include "os_report.h"

os_result
os_libraryAttrInit(
    os_libraryAttr *attr)
{
    attr->flags = 0;
    attr->autoTranslate = OS_TRUE;

    return os_resultSuccess;
}

os_library
os_libraryOpen(
    const char *name,
    os_libraryAttr *attr)
{
    os_library handle;
    char dllName[256];
    if(name && (strlen(name) > 0)){
        if(attr->autoTranslate == OS_TRUE){
            /* check if name ends with dll */
            if (strrchr(name, '.') != 0) {
                snprintf(dllName, 256, "%s", name);
            } else {
                snprintf(dllName, 256, "%s.dll", name);
            }
            handle = LoadLibrary(dllName);
        } else {
            handle = LoadLibrary(name);
        }
    } else {
        handle = NULL;
    }
    if (!handle) {
        OS_REPORT_1 (OS_ERROR, "os_libraryOpen", 0,
            "LoadLibrary error: %d", GetLastError());
    }
    return handle;
}

os_result
os_libraryClose(
    os_library library)
{
    os_result result;

    if (library) {
        if (FreeLibrary(library) == 0) {
            OS_REPORT_1 (OS_ERROR, "os_libraryClose", 0,
                "FreeLibrary error: %d", GetLastError());
            result = os_resultFail;
        } else {
            result = os_resultSuccess;
        }
    } else {
        result = os_resultInvalid;
    }
    return result;
}

os_symbol
os_libraryGetSymbol(
    os_library library,
    const char *symbolName)
{
    os_symbol symbol;

    assert (library);
    assert (symbolName);

    if (library && symbolName) {
        symbol = GetProcAddress(library, symbolName);

        if (!symbol) {
            OS_REPORT_1 (OS_ERROR, "os_libraryGetSymbol", 0,
                "GetProcAddress error for %s", symbolName);

            /* GetLastError() crashes in this context
            OS_REPORT_1 (OS_ERROR, "os_libraryGetSymbol", 0,
                "GetProcAddress error: %s", GetLastError());
            */
        }
    } else {
        symbol = NULL;
    }
    return symbol;
}
