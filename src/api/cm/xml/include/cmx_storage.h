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
/**@file api/cm/xml/include/cmx_storage.h
 * @brief Represents a storage. It offers facilities to read/append data from/
 * to a storage.
 */
#ifndef CMX_STORAGE_H
#define CMX_STORAGE_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CMXML
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API c_char* cmx_storageOpen (const c_char* attrs);
OS_API c_char* cmx_storageClose (const c_char* storage);
OS_API c_char* cmx_storageAppend (const c_char* storage, const c_char* metadata, const c_char* data);
OS_API c_char* cmx_storageRead (const c_char* storage);
OS_API c_char* cmx_storageGetType (const c_char* storage, const c_char* typeName);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_STORAGE_H */
