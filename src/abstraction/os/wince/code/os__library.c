/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "os_library.h"
#include "os_stdlib.h"
#include "os_report.h"
#include "os_heap.h"
#include "os_errno.h"

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
    LPWSTR wStringName;

    if(name && (strlen(name) > 0)){
        if(attr->autoTranslate == OS_TRUE){
            if (strrchr(name, '.') != 0) {
                _snprintf(dllName, 256, "%s", name);
            } else {
                _snprintf(dllName, 256, "%s.dll", name);
            }
            wStringName = wce_mbtowc(dllName);
            handle = LoadLibrary(wStringName);
        } else {
            wStringName = wce_mbtowc(name);
            handle = LoadLibrary(wStringName);
        }
        os_free(wStringName);
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
        if (FreeLibrary((HMODULE)library) == 0) {
            OS_REPORT (OS_ERROR, "os_libraryClose", 0,
                "FreeLibrary error: %s", os_getErrno());
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
    wchar_t* wStringSymbolName;

    assert (library);
    assert (symbolName);

    if (library && symbolName) {
        wStringSymbolName = wce_mbtowc(symbolName);
        symbol = GetProcAddress((HMODULE)library, wStringSymbolName);

        if (!symbol) {
            OS_REPORT (OS_ERROR, "os_libraryGetSymbol", 0,
                "GetProcAddress error: %s", os_getErrno());
        }
        os_free (wStringSymbolName);
    } else {
        symbol = NULL;
    }
    return symbol;
}
