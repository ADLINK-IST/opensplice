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
#ifndef C__SCOPE_H
#define C__SCOPE_H

#include "ut_avl.h"
#include "c_metabase.h"
#include "c_sync.h"

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

typedef c_bool (*c_scopeCondition) (c_metaObject o1, c_metaObject o2);
typedef void  *c_scopeWalkActionArg;
typedef void (*c_scopeWalkAction) (c_metaObject o, c_scopeWalkActionArg actionArg);
typedef c_bool (*c_scopeWalkBoolAction) (c_metaObject o, c_scopeWalkActionArg actionArg);

/* Declaration of c_binding for c_scope */
C_CLASS(c_binding);

/* Definition of c_scope */
C_STRUCT(c_scope) {
    ut_avlCTree_t bindings;
    c_binding headInsOrder;
    c_binding tailInsOrder;
};

#define c_scope(o) ((c_scope)(o))

OS_API c_scope
c_scopeNew(
    c_base base) __nonnull_all__;

OS_API c_scope
c_scopeNew_s(
    c_base base) __nonnull_all__;


OS_API void
c_scopeWalk(
    c_scope scope,
    c_scopeWalkAction action,
    c_scopeWalkActionArg actionArg)
    __nonnull((1, 2));

OS_API c_bool
c_scopeWalkBool(
    c_scope scope,
    c_scopeWalkBoolAction action,
    c_scopeWalkActionArg actionArg)
    __nonnull((1, 2));

OS_API c_ulong
c_scopeCount(
    c_scope scope) __nonnull_all__;

void
c_scopeInit(c_scope scope) __nonnull_all__;

void
c_scopeDeinit(
    c_scope scope) __nonnull_all__;

c_metaObject
c_scopeInsert (
    c_scope _this,
    c_metaObject object) __nonnull_all__;

c_baseObject
c_scopeResolve (
    c_scope _this,
    const char *name,
    c_ulong metaFilter) __nonnull_all__;

c_metaObject
c_scopeLookup (
    c_scope _this,
    const c_char *name,
    c_ulong metaFilter) __nonnull_all__;

c_metaObject
c_scopeRemove (
    c_scope _this,
    const c_char *name) __nonnull_all__;

c_bool
c_scopeExists (
    c_scope _this,
    const c_char *name) __nonnull_all__;

void
c_scopeClean (
    c_scope _this) __nonnull_all__;

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
