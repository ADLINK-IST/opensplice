/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "os.h"
#include "c__scope.h"
#include "c__base.h"
#include "c__metabase.h"
#include "c_mmbase.h"
#include "c_avltree.h"
#include "c_stringSupport.h"

c_mm c_baseMM(c_base base); /* protected method of c_base. */

#define MM(o)     (c_baseMM(c__getType(o)->base))

#define c_binding(o) ((c_binding)(o))

C_STRUCT(c_binding) {
    C_EXTENDS(c_avlNode);
    c_metaObject object;
    c_binding nextInsOrder;
};

typedef struct c_resolveArg {
    char *name;
    c_long metaFilter;
} *c_resolveArg;

static c_equality
c_bindingCompare(
    c_binding o1,
    c_binding o2,
    c_resolveArg resolve)
{
    c_long r;
    if (o2 == NULL) {
        if (resolve->name == NULL) return C_LT;
        if (o1->object->name == NULL) return C_GT;
        r = strcmp(o1->object->name, resolve->name);
    } else {
        if (o1->object->name == NULL) return C_GT;
        if (o2->object->name == NULL) return C_LT;
        r = strcmp(o1->object->name, o2->object->name);
    }
    if (r>0) return C_LT;
    if (r<0) return C_GT;
    return C_EQ;
}

c_scope
c_scopeNew(
    c_base base)
{
    c_scope o;

    o = c_scope(c_new(c_resolve(base,"c_scope")));
    if (o) {
        c_scopeInit(o);
    }

    return o;
}

void
c_scopeInit(
    c_scope s)
{
    assert(s != NULL);

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

static c_bool
c_bindingFree (
    c_binding binding,
    c_mm mm)
{
    if (binding == NULL) {
        return TRUE;
    }
    c_free(binding->object);
    c_mmFree(mm,binding);
    return TRUE;
}

void
c_scopeDeinit(
    c_scope scope)
{
    assert(scope != NULL);
    c_avlTreeWalk(c_avlTree(scope),c_bindingFree,MM(scope),C_POSTFIX);
}

void
c_scopeClean(
    c_scope scope)
{
    if (scope != NULL) {
        c_avlTreeWalk(c_avlTree(scope),c_bindingFree,MM(scope),C_POSTFIX);
    }
}

c_metaObject
c_scopeInsert(
    c_scope scope,
    c_metaObject object)
{
    c_binding binding, found;

    binding = c_bindingNew(scope, object);
    found = c_binding(c_avlTreeInsert(c_avlTree(scope),
                                      (void *)binding,
                                      c_bindingCompare,
                                      NULL));
    if (found == binding) {
        if (!scope->headInsOrder) {
            scope->headInsOrder = binding;
        }
        if (scope->tailInsOrder) {
           scope->tailInsOrder->nextInsOrder = binding;
        }
        scope->tailInsOrder = binding;
    } else {
        if (c_isFinal(found->object) == FALSE) {
            c_metaCopy(object,found->object);
        }
        c_bindingFree(binding,MM(scope));
    }

    c_keep(found->object);

    /** Note that if inserted (found == binding) the object reference count is increased by 2.
        one for being inserted and one for being returned.
     */
    return found->object;
}

c_metaObject
c_scopeLookup(
    c_scope scope,
    const c_char *name,
    c_long metaFilter)
{
    c_binding binding;
    c_metaObject o;
    struct c_resolveArg resolve;

    if (scope == NULL) {
        return NULL;
    }

    resolve.name = (char *)name;
    resolve.metaFilter = metaFilter;
    binding = c_avlTreeFind(c_avlTree(scope),
                            NULL,
                            c_bindingCompare,
                            &resolve);
    if (binding != NULL) {
	if (CQ_KIND_IN_MASK (binding->object, metaFilter)) {
            o = c_keep(binding->object);
        } else {
            o = NULL;
        }
    } else {
        o = NULL;
    }
    return o;
}

typedef void *c_metaFindCompareArg;
typedef c_metaEquality (*c_metaFindCompare) (c_baseObject baseObject, c_metaFindCompareArg arg);

typedef struct c_findArg {
    const char *name;
    c_long metaFilter;
    c_baseObject object;
} *c_findArg;

typedef struct c_walkArg {
    c_metaFindCompare compare;
    c_metaFindCompareArg arg;
    c_baseObject object;
} *c_walkArg;

static c_metaEquality
c_metaNameCompare (
    c_baseObject baseObject,
    c_metaFindCompareArg arg)
{
    c_findArg farg = (c_findArg)arg;
    c_metaEquality equality = E_UNEQUAL;
    char *name;

    if (CQ_KIND_IN_MASK (baseObject, farg->metaFilter)) {
        if (CQ_KIND_IN_MASK (baseObject, CQ_SPECIFIERS)) {
	    name = c_specifier(baseObject)->name;
        } else if (CQ_KIND_IN_MASK (baseObject, CQ_METAOBJECTS)) {
	    name = c_metaObject(baseObject)->name;
	} else {
	    return equality;
	}
	if (farg->metaFilter & CQ_CASEINSENSITIVE) {
            if (os_strcasecmp (name, (const char *)farg->name) == 0) {
	        equality = E_EQUAL;
	    }
        } else {
            if (strcmp (name, (const char *)farg->name) == 0) {
                equality = E_EQUAL;
            }
	}
    }
    return equality;
}

static c_bool
walkCompare (
    c_binding binding,
    c_walkArg arg)
{
    if (arg->compare (c_baseObject(binding->object), arg->arg) == E_EQUAL) {
        arg->object = c_baseObject(binding->object);
        return FALSE;
    }
    return TRUE;
}

c_baseObject
c_scopeResolve(
    c_scope scope,
    const char *name,
    c_long metaFilter)
{
    c_metaObject o = NULL;
    struct c_walkArg warg;
    struct c_findArg farg;

    if (scope == NULL) {
        return NULL;
    }
    if (metaFilter & CQ_CASEINSENSITIVE) {

        warg.arg = &farg;
        warg.compare = c_metaNameCompare;
        warg.object = NULL;
	farg.name = name;
	farg.metaFilter = metaFilter;
	farg.object = NULL;
        if (c_avlTreeWalk(c_avlTree(scope),walkCompare,&warg,C_POSTFIX) == FALSE) {
            if (warg.object) {
                o = c_keep(warg.object);
            }
        } else {
            o = NULL;
        }
    } else {
        o = c_scopeLookup(scope, name, metaFilter);
    }
    return c_baseObject(o);
}

c_metaObject
c_scopeRemove(
    c_scope scope,
    const c_char *name)
{
    c_binding binding, bindingBefore;
    c_metaObject result = NULL;

    binding = c_avlTreeRemove(c_avlTree(scope),
                              NULL,
                              c_bindingCompare,
                              (void *)name,
                              NULL,NULL);
    if (binding) {
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
        c_bindingFree(binding,MM(scope));
    }
    return result;
}

void
c_scopeWalk(
    c_scope scope,
    void (*action)(),
    void *actionArgument)
{
    c_binding binding;

    binding = scope->headInsOrder;
    while(binding) {
        action(binding->object, actionArgument);
        binding = binding->nextInsOrder;
    }
}

c_long
c_scopeCount(
    c_scope scope)
{
    c_long count;
    count = c_avlTreeCount(c_avlTree(scope));
    return count;
}
