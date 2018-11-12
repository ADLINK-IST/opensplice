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
#ifndef SD_SERIALIZERXMLTYPEINFO_H
#define SD_SERIALIZERXMLTYPEINFO_H

#include "sd_serializer.h"
#include "c_base.h"
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

OS_API sd_serializer sd_serializerXMLTypeinfoNew(c_base base, c_bool escapeQuote) __nonnull_all__ __attribute_warn_unused_result__;

#if defined (__cplusplus)
}
#endif

#undef OS_API

#endif /* SD_SERIALIZERXMLTYPEINFO_H */
