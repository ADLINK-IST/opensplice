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
    attr->flags = RTLD_LAZY;
    attr->autoTranslate = OS_TRUE;
    
    return os_resultSuccess;
}

os_library
os_libraryOpen(
    const char *name,
    os_libraryAttr *attr)
{
    os_library handle;
    char* libName; 
    
    if(name && (strlen(name) > 0)){
        if(attr->autoTranslate == OS_TRUE){
            libName = (char*)(os_malloc(strlen(name)+7));
            os_sprintf(libName, "lib%s.so", name);
            handle = dlopen (libName, attr->flags);
            os_free(libName);
        } else {
            handle = dlopen (name, attr->flags);
        }
    } else {
        handle = NULL;
    }
    
    if (!handle) {
        OS_REPORT_1 (OS_ERROR, "os_libraryOpen", 0,
            "dlopen error: %s", dlerror());
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
