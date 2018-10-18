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
#ifndef C_ITERATOR_H
#define C_ITERATOR_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#include "c_list_tmpl.h"
C__LIST_TYPES_TMPL(c__iterImpl, void *, /**/, 32)

C_CLASS(c_iter);

/* List iterator */
typedef struct c_iterIter {
    struct c__iterImplIter_s it;
    void *current;
} c_iterIter;

typedef struct c_iterIterD {
    struct c__iterImplIterD_s it;
    c_iter initSource;
} c_iterIterD;

typedef void *c_iterResolveCompareArg;
typedef c_equality (*c_iterResolveCompare) (c_voidp o, c_iterResolveCompareArg arg);

typedef void    *c_iterActionArg;
typedef c_bool (*c_iterAction)     (void *o, c_iterActionArg arg);
typedef void   (*c_iterWalkAction) (void *o, c_iterActionArg arg);

OS_API c_iter  c_iterNew        (void *object);
OS_API c_iter  c_iterInsert     (c_iter i, void *object);
OS_API c_iter  c_iterAppend     (c_iter i, void *object);
OS_API void   *c_iterTakeFirst  (c_iter i);
OS_API void   *c_iterTakeLast   (c_iter i);
OS_API void   *c_iterTake       (c_iter i, void *object);
OS_API void   *c_iterTakeAction (c_iter iter, c_iterAction condition, c_iterActionArg arg);
OS_API void   *c_iterReadAction (c_iter iter, c_iterAction condition, c_iterActionArg arg);
OS_API c_iter  c_iterConcat     (c_iter head, c_iter tail);
OS_API c_iter  c_iterCopy       (c_iter i);
OS_API c_ulong c_iterLength     (c_iter i);
OS_API void   *c_iterResolve    (c_iter i, c_iterResolveCompare compare, c_iterResolveCompareArg arg);
OS_API void   *c_iterObject     (c_iter i, c_ulong index);
OS_API void    c_iterWalk       (c_iter i, c_iterWalkAction action, c_iterActionArg arg);
OS_API c_bool  c_iterWalkUntil  (c_iter i, c_iterAction action, c_iterActionArg arg);
OS_API void    c_iterArray      (c_iter i, void *ar[]);
OS_API void    c_iterFree       (c_iter i);
OS_API c_bool  c_iterContains   (c_iter i, void *object);
OS_API c_iterIter c_iterIterGet (c_iter i);
OS_API void   *c_iterNext       (c_iterIter* iterator);
OS_API c_iterIterD c_iterIterGetD(c_iter i);
OS_API void   *c_iterNextD      (c_iterIterD* iterator);
OS_API void    c_iterRemoveD    (c_iterIterD* iterator);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
