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
#ifndef DDS_COPYOUT_H
#define DDS_COPYOUT_H

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

C_CLASS(DDS_dstInfo);

C_STRUCT(DDS_dstInfo) {
    void * dst;
    DDS_copyCache copyProgram;
    void * buf;
};

OS_API void
DDS_copyOutStruct (
    void *src,
    void *dst);

OS_API void *
DDS_copyOutAllocBuffer (
    DDS_copyCache copyCache,
    DDS_unsigned_long len);

/*
 * This macro inserts the copycache in a header,
 * if we are using the generic copy routines
 */
#define PREPEND_COPYOUTCACHE(cc,d,b) \
        if (cc){ \
            void * tmp = d; \
            d = os_malloc(C_SIZEOF(DDS_dstInfo)); \
            ((DDS_dstInfo)d)->dst = tmp; \
            ((DDS_dstInfo)d)->copyProgram = cc; \
            ((DDS_dstInfo)d)->buf = b; \
        }
/*
 * This macro removes the copycache from a header,
 * if we are using the generic copy routines
 */
#define REMOVE_COPYOUTCACHE(cc,d) \
        if (cc != NULL && d != NULL){ \
            void * tmp = ((DDS_dstInfo)d)->dst; \
            os_free(d); \
            d = tmp; \
        }        


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
