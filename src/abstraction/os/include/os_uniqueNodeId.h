
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
#ifndef OS_UNQIUENODEID_H
#define OS_UNQIUENODEID_H


#include <os_defs.h>

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if defined (__cplusplus)
extern "C" {
#endif

OS_API os_uint32 os_uniqueNodeIdGet(os_uint32 min, os_uint32 max, size_t entropySize, const void *entropy);
struct _SHA256_CTX;

/* Internal hook to add platform specific entropy to the ID*/
typedef void (os_platformEntropyHook)(struct _SHA256_CTX *);
void os_uniqueIdSetEntropyHook(os_platformEntropyHook peh);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
