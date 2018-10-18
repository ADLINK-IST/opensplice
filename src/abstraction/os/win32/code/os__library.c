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
#include <stdio.h>
#include <assert.h>
#include "os_library.h"
#include "os_stdlib.h"
#include "os_errno.h"
#include "os_report.h"

#include "os_win32incs.h"

void
os_libraryAttrInit(
    os_libraryAttr *attr)
{
    attr->autoTranslate = OS_TRUE;
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
        OS_REPORT (OS_ERROR, "os_libraryOpen", 0,
            "LoadLibrary error: %d", os_getErrno());
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
            OS_REPORT (OS_ERROR, "os_libraryClose", 0,
                "FreeLibrary error: %d", os_getErrno());
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
            OS_REPORT (OS_ERROR, "os_libraryGetSymbol", 0,
                "GetProcAddress error for %s", symbolName);

            /* GetLastError() crashes in this context
            OS_REPORT (OS_ERROR, "os_libraryGetSymbol", 0,
                "GetProcAddress error: %s", GetLastError());
            */
        }
    } else {
        symbol = NULL;
    }
    return symbol;
}
