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
#ifndef OS_WIN32_LIBARY_H
#define OS_WIN32_LIBARY_H

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct os_os_libraryAttr {
    os_boolean autoTranslate; /* Determines whether the library name is automatically mapped to its platform dependent name*/
} os_os_libraryAttr;

typedef void *os_os_library;
typedef void *os_os_symbol;

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_LIBRARY_H */
