/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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
    c_base base);

OS_API void
c_scopeWalk(
    c_scope scope,
    c_scopeWalkAction action,
    c_scopeWalkActionArg actionArg);

OS_API c_long
c_scopeCount(
    c_scope scope);

void
c_scopeInit(c_scope scope);

void
c_scopeDeinit(
    c_scope scope);

c_metaObject
c_scopeInsert (
    c_scope _this,
    c_metaObject object);

c_baseObject
c_scopeResolve (
    c_scope _this,
    const char *name,
    c_long metaFilter);

c_metaObject
c_scopeLookup (
    c_scope _this,
    const c_char *name,
    c_long metaFilter);

c_metaObject
c_scopeRemove (
    c_scope _this,
    const c_char *name);

void
c_scopeClean (
    c_scope _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
