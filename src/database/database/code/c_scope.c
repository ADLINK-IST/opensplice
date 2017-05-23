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
#include <stddef.h>
#include "vortex_os.h"
#include "ut_avl.h"
#include "c__scope.h"
#include "c__base.h"
#include "c__metabase.h"
#include "c_mmbase.h"
#include "c__mmbase.h"
#include "c_stringSupport.h"

c_mm c_baseMM(c_base base); /* protected method of c_base. */

#define MM(o)     (c_baseMM(c__getType(o)->base))

#define c_binding(o) ((c_binding)(o))

C_STRUCT(c_binding) {
    ut_avlNode_t avlnode;
    c_metaObject object;
    c_binding nextInsOrder;
};

typedef struct c_resolveArg {
    char *name;
    c_long metaFilter;
} *c_resolveArg;

static int c_bindingCompare (const void *a, const void *b);

static const ut_avlCTreedef_t c_scope_bindings_td =
    UT_AVL_CTREEDEF_INITIALIZER_INDKEY (offsetof (C_STRUCT(c_binding), avlnode),
                                        offsetof (C_STRUCT(c_binding), object),
                                        c_bindingCompare, 0);

static int
c_bindingCompare(const void *va, const void *vb)
{
    const C_STRUCT(c_metaObject) *a = va;
    const C_STRUCT(c_metaObject) *b = vb;
    if (a->name == NULL) return 1;
    if (b->name == NULL) return -1;
    return strcmp (a->name, b->name);
}

static c_scope
c__scopeNewCommon(
    c_base base,
    c_bool check)
{
    c_scope o;
    c_type scopeType;

    scopeType = c_resolve(base,"c_scope");
    if (check) {
        o = c_scope(c_new_s(scopeType));
        if (!o) {
            goto err_alloc_scope;
        }
    } else {
        o = c_scope(c_new(scopeType));
    }
    assert(o);
    c_scopeInit(o);

err_alloc_scope:
    c_free(scopeType);
    return o;
}

c_scope
c_scopeNew(
    c_base base)
{
    return c__scopeNewCommon(base, FALSE);
}

c_scope
c_scopeNew_s(
    c_base base)
{
    return c__scopeNewCommon(base, TRUE);
}

void
c_scopeInit(
    c_scope s)
{
    assert(s != NULL);

    ut_avlCInit (&c_scope_bindings_td, &s->bindings);
    s->headInsOrder = NULL;
    s->tailInsOrder = NULL;
}

static c_binding
c_bindingNew(
    c_scope scope,
    c_metaObject object)
{
    c_binding result;

    result = c_binding(c_mmMalloc(MM(scope),C_SIZEOF(c_binding)));
    if (result) {
        result->object = c_keep(object);
        result->nextInsOrder = NULL;
    }

    return result;
}

static void
c_bindingFree (
    c_binding binding,
    c_mm mm)
{
    c_free(binding->object);
    c_mmFree(mm,binding);
}

static void c_bindingFreeWrapper (void *binding, void *mm)
{
    c_bindingFree (binding, mm);
}


void
c_scopeClean(
    c_scope scope)
{
    ut_avlCFreeArg (&c_scope_bindings_td, &scope->bindings, c_bindingFreeWrapper, MM (scope));
}

void
c_scopeDeinit(
    c_scope scope)
{
    c_scopeClean (scope);
}

c_metaObject
c_scopeInsert(
    c_scope scope,
    c_metaObject object)
{
    ut_avlIPath_t p;
    c_binding binding = NULL;

    if ((binding = ut_avlCLookupIPath (&c_scope_bindings_td, &scope->bindings, object, &p)) == NULL) {
        binding = c_bindingNew(scope, object);
        ut_avlCInsertIPath (&c_scope_bindings_td, &scope->bindings, binding, &p);
        if (!scope->headInsOrder) {
            scope->headInsOrder = binding;
        }
        if (scope->tailInsOrder) {
            scope->tailInsOrder->nextInsOrder = binding;
        }
        scope->tailInsOrder = binding;
    } else {
        if (c_isFinal(binding->object) == FALSE) {
            c_metaCopy(object,binding->object);
        }
    }
    c_keep(binding->object);

    /** Note that if inserted the object reference count is increased
        by 2.  one for being inserted and one for being returned.
     */
    return binding->object;
}

c_metaObject
c_scopeLookup(
    c_scope scope,
    const c_char *name,
    c_ulong metaFilter)
{
    C_STRUCT(c_metaObject) tmp;
    c_binding binding;
    c_metaObject o;

    tmp.name = (char *) name;
    if ((binding = ut_avlCLookup (&c_scope_bindings_td, &scope->bindings, &tmp)) == NULL) {
        o = NULL;
    } else if (CQ_KIND_IN_MASK (binding->object, metaFilter)) {
        o = c_keep(binding->object);
    } else {
        o = NULL;
    }
    return o;
}

static c_metaEquality
c_metaNameCompare (
    c_baseObject baseObject,
    const char *key,
    c_ulong metaFilter)
{
    c_metaEquality equality = E_UNEQUAL;
    char *name;

    if (CQ_KIND_IN_MASK (baseObject, metaFilter)) {
        if (CQ_KIND_IN_MASK (baseObject, CQ_SPECIFIERS)) {
            name = c_specifier(baseObject)->name;
        } else if (CQ_KIND_IN_MASK (baseObject, CQ_METAOBJECTS)) {
            name = c_metaObject(baseObject)->name;
        } else {
            return equality;
        }
        if (metaFilter & CQ_CASEINSENSITIVE) {
            if (os_strcasecmp (name, key) == 0) {
                equality = E_EQUAL;
            }
        } else {
            if (strcmp (name, key) == 0) {
                equality = E_EQUAL;
            }
        }
    }
    return equality;
}

c_baseObject
c_scopeResolve(
    c_scope scope,
    const char *name,
    c_ulong metaFilter)
{
    c_metaObject o = NULL;
    if (!(metaFilter & CQ_CASEINSENSITIVE)) {
        o = c_scopeLookup(scope, name, metaFilter);
    } else {
        ut_avlCIter_t it;
        c_binding b;
        for (b = ut_avlCIterFirst (&c_scope_bindings_td, &scope->bindings, &it);
             b != NULL && o == NULL;
             b = ut_avlCIterNext (&it)) {
            if (c_metaNameCompare (c_baseObject(b->object), name, metaFilter) == E_EQUAL) {
                o = c_keep (c_baseObject (b->object));
            }
        }
    }
    return c_baseObject(o);
}

c_metaObject
c_scopeRemove(
    c_scope scope,
    const c_char *name)
{
    C_STRUCT(c_metaObject) tmp;
    ut_avlDPath_t p;
    c_binding binding;
    tmp.name = (char *) name;
    if ((binding = ut_avlCLookupDPath (&c_scope_bindings_td, &scope->bindings, &tmp, &p)) == NULL) {
        return NULL;
    } else {
        c_binding bindingBefore;
        c_metaObject result;
        ut_avlCDeleteDPath (&c_scope_bindings_td, &scope->bindings, binding, &p);
        if (binding == scope->headInsOrder) {
            scope->headInsOrder = binding->nextInsOrder;
            if (binding == scope->tailInsOrder) {
                scope->tailInsOrder = NULL;
            }
        } else {
            bindingBefore = scope->headInsOrder;
            while (bindingBefore && (bindingBefore->nextInsOrder != binding)) {
                bindingBefore = bindingBefore->nextInsOrder;
            }
            assert(bindingBefore);
            bindingBefore->nextInsOrder = binding->nextInsOrder;
            if (binding == scope->tailInsOrder) {
                scope->tailInsOrder = bindingBefore;
            }
        }
        result = binding->object;
        c_bindingFree(binding, MM(scope));
        return result;
    }
}

void
c_scopeWalk(
    c_scope scope,
    c_scopeWalkAction action,
    c_scopeWalkActionArg actionArg)
{
    c_binding binding;

    binding = scope->headInsOrder;
    while(binding) {
        action(binding->object, actionArg);
        binding = binding->nextInsOrder;
    }
}

c_bool
c_scopeWalkBool(
    c_scope scope,
    c_scopeWalkBoolAction action,
    c_scopeWalkActionArg actionArg)
{
    c_binding binding;

    binding = scope->headInsOrder;
    while(binding) {
        if (!action(binding->object, actionArg)) {
            return FALSE;
        }
        binding = binding->nextInsOrder;
    }
    return TRUE;
}

c_ulong
c_scopeCount(
    c_scope scope)
{
    return (c_ulong) ut_avlCCount (&scope->bindings);
}

c_bool
c_scopeExists (
    c_scope scope,
    const c_char *name)
{
    C_STRUCT(c_metaObject) tmp;
    tmp.name = (char *) name;
    return (ut_avlCLookup (&c_scope_bindings_td, &scope->bindings, &tmp) != NULL);
}
