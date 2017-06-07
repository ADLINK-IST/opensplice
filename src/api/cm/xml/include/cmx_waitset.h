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
/**@file api/cm/xml/include/cmx_waitset.h
 * Represents a waitset in Splice in XML format.
 */
#ifndef CMX_WAITSET_H
#define CMX_WAITSET_H

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

OS_API c_char*         cmx_waitsetNew          (const c_char* participant);

OS_API const c_char*   cmx_waitsetAttach       (const c_char* waitset,
                                                const c_char* entity);
    
OS_API const c_char*   cmx_waitsetDetach       (const c_char* waitset, 
                                                const c_char* entity);
 
OS_API c_char*         cmx_waitsetWait         (const c_char* waitset);

OS_API c_char*         cmx_waitsetTimedWait    (const c_char* waitset,
                                                const os_duration t);

OS_API c_ulong         cmx_waitsetGetEventMask (const c_char* waitset);

OS_API const c_char*   cmx_waitsetSetEventMask (const c_char* waitset,
                                                c_ulong mask);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_WAITSET_H*/
