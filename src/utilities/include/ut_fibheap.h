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
#ifndef UT_FIBHEAP_H
#define UT_FIBHEAP_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

typedef struct ut_fibheapNode {
  struct ut_fibheapNode *parent, *children;
  struct ut_fibheapNode *prev, *next;
  unsigned mark: 1;
  unsigned degree: 31;
} ut_fibheapNode_t;

typedef struct ut_fibheapDef {
    os_address offset;
    int (*cmp) (const void *va, const void *vb);
} ut_fibheapDef_t;

typedef struct ut_fibheap {
  ut_fibheapNode_t *roots; /* points to root with min key value */
} ut_fibheap_t;

#define UT_FIBHEAPDEF_INITIALIZER(offset, cmp) { (offset), (cmp) }

OS_API void ut_fibheapDefInit (ut_fibheapDef_t *fhdef, os_address offset, int (*cmp) (const void *va, const void *vb));
OS_API void ut_fibheapInit (const ut_fibheapDef_t *fhdef, ut_fibheap_t *fh);
OS_API void *ut_fibheapMin (const ut_fibheapDef_t *fhdef, const ut_fibheap_t *fh);
OS_API void ut_fibheapMerge (const ut_fibheapDef_t *fhdef, ut_fibheap_t *a, ut_fibheap_t *b);
OS_API void ut_fibheapInsert (const ut_fibheapDef_t *fhdef, ut_fibheap_t *fh, const void *vnode);
OS_API void ut_fibheapDelete (const ut_fibheapDef_t *fhdef, ut_fibheap_t *fh, const void *vnode);
OS_API void *ut_fibheapExtractMin (const ut_fibheapDef_t *fhdef, ut_fibheap_t *fh);
OS_API void ut_fibheapDecreaseKey (const ut_fibheapDef_t *fhdef, ut_fibheap_t *fh, const void *vnode); /* to be called AFTER decreasing the key */

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_FIBHEAP_H */
