/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef OS_LIBRARY_H
#define OS_LIBRARY_H

#include "os_defs.h"

#include "os_if.h"
#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#if defined (__cplusplus)
extern "C" {
#endif

typedef os_os_library os_library;
typedef os_os_symbol os_symbol;

typedef os_os_libraryAttr os_libraryAttr;

OS_API void         os_libraryAttrInit      (os_libraryAttr *attr) __nonnull_all__;

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
