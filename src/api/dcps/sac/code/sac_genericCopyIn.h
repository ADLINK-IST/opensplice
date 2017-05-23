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
#ifndef DDS_COPYIN_H
#define DDS_COPYIN_H

#include "c_typebase.h"

#include "sac_genericCopyCache.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSSAC
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(DDS_srcInfo);

C_STRUCT(DDS_srcInfo) {
    void * src;
    DDS_copyCache copyProgram;
};

OS_API v_copyin_result
DDS_copyInStruct (
    c_base base,
    void *src,
    void *dst);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
