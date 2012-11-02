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
#ifndef OS_LIBRARY_H
#define OS_LIBRARY_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"
#include "include/os_library.h"

#include "os_if.h"
#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef os_os_library os_library;
typedef os_os_symbol os_symbol;

typedef struct os_libraryAttr {
    os_int32 flags;
    os_boolean autoTranslate; /* Determines whether the library name is automatically mapped to its platform dependent name*/ 
} os_libraryAttr;

OS_API os_result    os_libraryAttrInit      (os_libraryAttr *attr);

OS_API os_library   os_libraryOpen          (const char *name,
                                             os_libraryAttr *attr);

OS_API os_result    os_libraryClose         (os_library library);

OS_API os_symbol    os_libraryGetSymbol     (os_library library,
                                             const char *symbolName);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /*OS_LIBRARY_H*/
