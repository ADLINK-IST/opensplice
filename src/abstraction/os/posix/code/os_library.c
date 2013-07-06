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
#include <assert.h>
#include <unistd.h>
#include "os_library.h"
#include "os_stdlib.h"
#include "string.h"
#include "os_report.h"
#include "os_heap.h"

os_result
os_libraryAttrInit(
    os_libraryAttr *attr)
{
    /* Use RTLD_GLOBAL and RTDL_LAZY so that duplicate symbols are not loaded */
    attr->flags = RTLD_GLOBAL | RTLD_LAZY;
    attr->autoTranslate = OS_TRUE;

    return os_resultSuccess;
}

os_library
os_libraryOpen(
    const char *name,
    os_libraryAttr *attr)
{
    /* revolving order
     * when the name contains a path open this path, if that fails
     * add lib in front of the name and .suffix behind it and try to open
     * if that fails try to just open the given name
     */
    os_library handle;
    char* libName;

    if(name && (strlen(name) > 0)){
        if(attr->autoTranslate == OS_TRUE){
#if __APPLE__
            static const char suffix[] = ".dylib";
#else
            static const char suffix[] = ".so";
#endif
            if(strrchr(name, '/') != 0){
                /* found a path that includes a '/' open lib with given path*/
                handle = dlopen (name, attr->flags);
            } else {
                /* add lib and suffix to the name and try to open */
                libName = (char*)(os_malloc(3 + strlen(name) + sizeof(suffix)));
                os_sprintf(libName, "lib%s%s", name, suffix);
                handle = dlopen (libName, attr->flags);
                os_free(libName);
                if (!handle) {
                    /* lib open failed try to open just the name */
                    handle = dlopen (name, attr->flags);
                }
            }
        } else {
            handle = dlopen (name, attr->flags);
        }
        if (!handle) {
            OS_REPORT_1 (OS_ERROR, "os_libraryOpen", 0,
                "dlopen error: %s", dlerror());
        }
    } else {
        OS_REPORT(OS_ERROR, "os_libraryOpen", 0,
            "library name not valid");
        handle = NULL;
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
            OS_REPORT_1 (OS_ERROR, "os_libraryClose", 0,
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
            OS_REPORT_1 (OS_ERROR, "os_libraryGetSymbol", 0,
                "dlsym error: %s", dlerror());
        }
    } else {
        symbol = NULL;
    }
    return symbol;
}
