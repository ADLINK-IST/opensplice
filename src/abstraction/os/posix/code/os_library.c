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
#include <assert.h>
#include <unistd.h>
#include "os_library.h"
#include "os_stdlib.h"
#include "string.h"
#include "os_report.h"
#include "os_heap.h"

void
os_libraryAttrInit(
    os_libraryAttr *attr)
{
    /* Use RTLD_GLOBAL and RTLD_NOW so that duplicate symbols are not loaded */
    attr->flags = RTLD_GLOBAL | RTLD_NOW;
    attr->autoTranslate = OS_TRUE;
}

os_library
os_libraryOpen(
    const char *name,
    os_libraryAttr *attr)
{
    os_library handle = NULL;

    if(name && (strlen(name) > 0)) {
        if((attr->autoTranslate == OS_TRUE) &&
           (strrchr(name, '/') == NULL)) {
            /* add lib and suffix to the name and try to open */
#if __APPLE__
            static const char suffix[] = ".dylib";
#else
            static const char suffix[] = ".so";
#endif
            char* libName = os_malloc(3 + strlen(name) + sizeof(suffix));
            os_sprintf(libName, "lib%s%s", name, suffix);
            handle = dlopen(libName, attr->flags);
            if (!handle) {
                OS_REPORT(OS_WARNING, "os_libraryOpen", 0,
                    "dlopen of auto-translated name '%s' failed: %s",
                    libName, dlerror());
            }
            os_free(libName);
        }

        if (!handle) {
            /* Name contains a path, auto-translate is disabled or dlopen on translated name failed */
            handle = dlopen(name, attr->flags);
            if (!handle) {
                OS_REPORT(OS_ERROR, "os_libraryOpen", 0,
                    "dlopen of '%s' failed: %s",
                    name, dlerror());
            }
        }
    } else {
        OS_REPORT(OS_ERROR, "os_libraryOpen", 0,
            "library name not valid");
    }
    return handle;
}

os_result
os_libraryClose(
    os_library library)
{
    os_result result;

    if (library) {
        if (dlclose (library) != 0) {
            OS_REPORT (OS_ERROR, "os_libraryClose", 0,
                "dlclose error: %s", dlerror());
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
        symbol = dlsym (library, symbolName);

        if (!symbol) {
            OS_REPORT (OS_ERROR, "os_libraryGetSymbol", 0,
                "dlsym error: %s", dlerror());
        }
    } else {
        symbol = NULL;
    }
    return symbol;
}
