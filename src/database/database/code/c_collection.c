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
#include <math.h>
#include "os.h"
#include "os_report.h"
#include "q_expr.h"
#include "c__base.h"
#include "c__scope.h"
#include "c__collection.h"
#include "c_misc.h"
#include "c_metabase.h"
#include "c__querybase.h"
#include "c_metafactory.h"
#include "c_stringSupport.h"
#include "c_avltree.h"
#include "c_mmbase.h"
#include "c_mmCache.h"

#define _PREALLOC_ (32)

#define MM(c)      ((c_mm)c_baseMM(c__getBase(c)))

#define c_list(c)  ((C_STRUCT(c_list)  *)(c))
#define c_set(c)   ((C_STRUCT(c_set)   *)(c))
#define c_bag(c)   ((C_STRUCT(c_bag)   *)(c))
#define c_table(c) ((C_STRUCT(c_table) *)(c))
#define c_query(c) ((C_STRUCT(c_query) *)(c))
#define c_array(c) ((c_array)(c))

#ifndef NDEBUG
/* Cannot add this checking yet until proper locking on kernel configuration
 * is implemented.
#define _CONSISTENCY_CHECKING_
 */
#endif

#ifdef _CONSISTENCY_CHECKING_

    #define _STATISTICS_ \
            c_ulong accessCount; \
            c_ulong readCount;

    #define _ACCESS_BEGIN_(t) \
            if (pa_increment(&c_table(t)->accessCount) != 1) { \
                abort(); \
            }

    #define _ACCESS_END_(t) \
            if (c_table(t)->accessCount != 1) { \
                abort(); \
            } else { \
                pa_decrement(&c_table(t)->accessCount); \
            }

    #define _READ_BEGIN_(t) \
            if (pa_increment(&c_table(t)->accessCount) != 1) { \
                abort(); \
            }

    #define _READ_END_(t) \
            if (c_table(t)->accessCount != 1) { \
                abort(); \
            } else { \
                pa_decrement(&c_table(t)->accessCount); \
            }

    #define _CHECK_CONSISTENCY_(t,n) \
            if ((c_table(t)->accessCount != 1) || \
                (((c_tableNode)n)->table != (c_voidp)(t))) \
            { \
                abort(); \
            }

#else

    #define _STATISTICS_
    #define _ACCESS_BEGIN_(t)
    #define _ACCESS_END_(t)
    #define _READ_BEGIN_(t)
    #define _READ_END_(t)
    #define _CHECK_CONSISTENCY_(t,n)

#endif

C_CLASS(c_listNode);
C_CLASS(c_setNode);
C_CLASS(c_bagNode);
C_CLASS(c_tableNode);

C_STRUCT(c_list) {
    c_listNode head;
    c_listNode tail;
    c_ulong count;
    c_mmCache cache;
};

C_STRUCT(c_set) {
    C_EXTENDS(c_avlTree);
    c_mmCache cache;
};

C_STRUCT(c_bag) {
    C_EXTENDS(c_avlTree);
    c_ulong count;
    c_mmCache cache;
};

C_STRUCT(c_table) {
    c_object object;   /* is either an object or a collection of objects. */
    c_array key;
    c_ulong count;
    c_mmCache cache;
    _STATISTICS_
};

C_STRUCT(c_query) {
    c_qPred pred;
    c_collection source;
};

const c_long c_listSize  = C_SIZEOF(c_list);
const c_long c_setSize   = C_SIZEOF(c_set);
const c_long c_bagSize   = C_SIZEOF(c_bag);
const c_long c_tableSize = C_SIZEOF(c_table);
const c_long c_querySize = C_SIZEOF(c_query);

/* ============================================================================*/
/* GENERIC COLLECT ACTION METHOD                                               */
/* ============================================================================*/

C_STRUCT(collectActionArg) {
    c_iter results;
    c_long max;
};

C_CLASS(collectActionArg);

static c_bool
collectAction(
    c_object o,
    c_voidp arg)
{
    collectActionArg a = (collectActionArg)arg;

    c_iterInsert(a->results, c_keep(o));
    if (c_iterLength(a->results) < a->max) {
        return TRUE;
    }
    return FALSE;
}

static c_bool
readOne(
    c_object o,
    c_voidp arg)
{
    *(c_object *)arg = c_keep(o);
    return FALSE;
}

static c_bool
c_collectionIsType(
    c_collection c,
    c_collKind kind)
{
    c_type type = c__getType(c);

    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        return FALSE;
    }
    return c_collectionType(type)->kind == kind;
}

/* ============================================================================*/
/* LIST COLLECTION TYPE                                                        */
/* ============================================================================*/

C_STRUCT(c_listNode) {
    c_listNode next;
    c_object object;
};

c_object
c_listInsert (
    c_list _this,
    c_object o)
{
    C_STRUCT(c_list) *l = (C_STRUCT(c_list) *)_this;
    c_listNode n;

    assert(c_collectionIsType(_this, C_LIST));

    n = (c_listNode)c_mmCacheMalloc(l->cache);
    n->object = c_keep(o);
    n->next = l->head;
    l->head = n;
    if (l->tail == NULL) {
        l->tail = n;
    }
    l->count++;
    return n->object;
}

c_object
c_listRemove (
    c_list _this,
    c_object o,
    c_removeCondition condition,
    c_voidp arg)
{
    C_STRUCT(c_list) *l = (C_STRUCT(c_list) *)_this;
    c_listNode n,p;
    c_object found;

    assert(c_collectionIsType(_this, C_LIST));

    p = NULL;
    n = l->head;
    while ((n != NULL) && (n->object != o)) {
        p = n;
        n = n->next;
    }
    if (n == NULL) return NULL;

    if (condition != NULL) {
        if (!condition(n->object,o,arg)) return NULL;
    }
    found = n->object;

    if (p != NULL) {
        p->next = n->next;
    } else {
        l->head = n->next;
    }
    if (n == l->tail) {
        l->tail = p;
    }
    c_mmCacheFree(l->cache,n);
    l->count--;

    return found;
}

c_object
c_listTemplateRemove (
    c_list _this,
    c_action condition,
    c_voidp arg)
{
    C_STRUCT(c_list) *l = (C_STRUCT(c_list) *)_this;
    c_listNode n,p;
    c_object found;

    assert(c_collectionIsType(_this, C_LIST));

    p = NULL;
    n = l->head;
    while ((n != NULL) && (!condition(n->object,arg))) {
        p = n;
        n = n->next;
    }
    if (n == NULL) return NULL;

    found = n->object;

    if (p != NULL) {
        p->next = n->next;
    } else {
        l->head = n->next;
    }
    if (n == l->tail) {
        l->tail = p;
    }
    c_mmCacheFree(l->cache,n);
    l->count--;

    return found;
}

static c_object
c_listReplace (
    c_list _this,
    c_object o,
    c_bool (*condition)(),
    c_voidp arg)
{
    C_STRUCT(c_list) *l = (C_STRUCT(c_list) *)_this;
    c_listNode n;
    c_object found;

    assert(c_collectionIsType(_this, C_LIST));

    n = l->head;
    while ((n != NULL) && (n->object != o)) n = n->next;
    if (n == NULL) {
        return NULL;
    }
    if (condition != NULL) {
        if (!condition(n->object,o,arg)) {
            return n->object;
        }
    }
    found = n->object;
    n->object = c_keep(o);
    return found;
}

c_long
c_listCount (
    c_list _this)
{
    assert(c_collectionIsType(_this, C_LIST));
    return ((C_STRUCT(c_list) *)_this)->count;
}

static c_bool
c_listRead(
    c_list _this,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    C_STRUCT(c_list) *l = (C_STRUCT(c_list) *)_this;
    c_listNode n;
    c_bool proceed;
    c_qPred pred;

    assert(c_collectionIsType(_this, C_LIST));

    proceed = TRUE;
    n = l->head;
    while ((n != NULL) && proceed) {
        pred = q;
        if (pred == NULL) {
            proceed = action(n->object,arg);
        } else {
            while (pred != NULL) {
                if (c_qPredEval(pred,n->object)) {
                    proceed = action(n->object,arg);
                    pred = NULL;
                } else {
                    pred = pred->next;
                }
            }
        }
        n = n->next;
    }
    return proceed;
}

static c_object
c_listReadOne (
    c_list _this,
    c_qPred q)
{
    c_object o = NULL;

    assert(c_collectionIsType(_this, C_LIST));

    c_listRead(_this,q,readOne,&o);
    return o;
}

static c_object
c_listTakeOne (
    c_list _this,
    c_qPred q)
{
    c_object o;
    c_object found;

    assert(c_collectionIsType(_this, C_LIST));

    o = c_listReadOne(_this,q);
    if (o == NULL) {
        return NULL;
    }
    found = c_listRemove(_this,o, NULL, NULL);
    assert(found == o);
    c_free(found);

    return o;
}

static c_bool
c_listTake (
    c_list _this,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    c_object o;

    assert(c_collectionIsType(_this, C_LIST));

    while (proceed) {
        o = c_listTakeOne(_this,q);
        if (o != NULL) {
            proceed = action(o,arg);
            c_free(o);
        } else {
            proceed = FALSE;
        }
    }
    return proceed;
}

static c_iter
c_listSelect (
    c_list _this,
    c_qPred q,
    c_long max)
{
    C_STRUCT(collectActionArg) arg;

    assert(c_collectionIsType(_this, C_LIST));

    arg.results = c_iterNew(NULL);
    if (max < 1) {
        max = 0x7fffffff; /* RP: MAXINT */
    }
    arg.max = max;
    c_listRead(_this,q,collectAction,&arg);
    return arg.results;
}

c_bool
c_listWalk(
    c_list _this,
    c_action action,
    c_voidp actionArg)
{
    c_listNode n;

    assert(c_collectionIsType(_this, C_LIST));

    n = ((C_STRUCT(c_list) *)_this)->head;
    while (n != NULL) {
        if (!action(n->object,actionArg)) {
            return FALSE;
        }
        n = n->next;
    }
    return TRUE;
}

c_object
c_append (
    c_list list,
    c_object o)
{
    C_STRUCT(c_list) *l = c_list(list);
    c_listNode n;

    assert(c_collectionIsType(list, C_LIST));

    if (o == NULL) {
        return NULL;
    }

    n = (c_listNode)c_mmCacheMalloc(l->cache);
    n->object = c_keep(o);
    n->next = NULL;

    if (l->tail == NULL) {
        assert(l->head == NULL);
        l->head = n;
    } else {
        l->tail->next = n;
    }
    l->tail = n;
    l->count++;
    return n->object;
}

c_list
c_concat (
    c_list head,
    c_list tail)
{
    C_STRUCT(c_list) *h = c_list(head);
    C_STRUCT(c_list) *t = c_list(tail);

    assert(c_collectionIsType(head, C_LIST));
    assert(c_collectionIsType(tail, C_LIST));

    if (h->tail == NULL) {
        assert(h->head == NULL);
        h->head = t->head;
    } else {
        h->tail->next = t->head;
    }
    h->count += t->count;
    c_free(t);
    return (c_list)h;
}

c_object
c_replaceAt (
    c_list list,
    c_object o,
    c_long index)
{
    C_STRUCT(c_list) *l = c_list(list);
    c_listNode n;
    c_object r;
    c_long i;

    assert(c_collectionIsType(list, C_LIST));

    n = l->head;
    for (i=0;i<index;i++) {
        if (n == NULL) return NULL;
        n = n->next;
    }
    if (n == NULL) {
        return NULL;
    }
    r = n->object;
    n->object = c_keep(o);
    return r;
}

c_bool
c_insertAfter (
    c_list list,
    c_object o,
    c_long index)
{
    C_STRUCT(c_list) *l = c_list(list);
    c_listNode n,p;
    c_long i;

    assert(c_collectionIsType(list, C_LIST));

    n = l->head;
    for (i=0;i<index;i++) {
        if (n == NULL) return FALSE;
        n = n->next;
    }
    if (n == NULL) {
        return FALSE;
    }
    p = (c_listNode)c_mmCacheMalloc(l->cache);
    p->object = c_keep(o);
    p->next = n->next;
    n->next = p;
    if (l->tail == n) {
        l->tail = p;
    }
    l->count++;
    return TRUE;
}

c_bool
c_insertBefore (
    c_list list,
    c_object o,
    c_long index)
{
    C_STRUCT(c_list) *l = c_list(list);
    c_listNode *n,p;
    c_long i;

    assert(c_collectionIsType(list, C_LIST));

    n = &l->head;
    for (i=0;i<index;i++) {
        if (*n == NULL) return FALSE;
        n = &(*n)->next;
    }
    if (*n == NULL) {
        return FALSE;
    }
    p = (c_listNode)c_mmCacheMalloc(l->cache);
    p->object = c_keep(o);
    p->next = (*n);
    *n = p;
    if (l->tail == NULL) {
        l->tail = p;
    }
    l->count++;
    return TRUE;
}

c_object
c_readAt (
    c_list list,
    c_long index)
{
    C_STRUCT(c_list) *l = c_list(list);
    c_listNode n;
    c_long i;

    assert(c_collectionIsType(list, C_LIST));

    n = l->head;
    for (i=0;i<index;i++) {
        if (n == NULL) return NULL;
        n = n->next;
    }
    if (n == NULL) {
        return NULL;
    }
    return c_keep(n->object);
}

c_object
c_removeAt (
    c_list list,
    c_long index)
{
    C_STRUCT(c_list) *l = c_list(list);
    c_listNode n,p;
    c_object o;
    c_long i;

    assert(c_collectionIsType(list, C_LIST));

    p = NULL;
    n = l->head;
    for (i=0;i<index;i++) {
        if (n == NULL) return NULL;
        p = n;
        n = n->next;
    }
    if (n == NULL) {
        return NULL;
    }
    if (n == l->head) {
        l->head = l->head->next;
        if (n == l->tail) {
            assert(l->head == NULL);
            l->tail = NULL;
        }
    } else if (n == l->tail) {
        if (p != NULL) {
            p->next = NULL;
            l->tail = p;
        } else {
           assert(l->head == l->tail);
        }
    } else {
        p->next = n->next;
    }
    o = n->object;
    n->next = NULL; /* prevent that rest of the list is freed! */
    l->count--;
    c_mmCacheFree(l->cache,n);
    return o;
}

c_object
c_readLast (
    c_list list)
{
    C_STRUCT(c_list) *l = c_list(list);

    assert(c_collectionIsType(list, C_LIST));

    if (l->tail == NULL) {
        return NULL;
    }
    return c_keep(l->tail->object);
}

/* ============================================================================*/
/* SET COLLECTION TYPE                                                         */
/* ============================================================================*/

C_STRUCT(c_setNode) {
    C_EXTENDS(c_avlNode);
    c_object object;
};

static c_equality
c_setCompare(
    c_setNode n1,
    c_setNode n2,
    c_voidp args)
{
    if (args != NULL) {
        OS_REPORT(OS_WARNING,"c_set",0,"c_setCompare: parameter arg not NULL");
    }
    if (n2->object == NULL) {
        return C_GT;
    }
    if (n1->object == NULL) {
        return C_LT;
    }
    if ((c_voidp)n1->object > (c_voidp)n2->object) {
        return C_GT;
    }
    if ((c_voidp)n1->object < (c_voidp)n2->object) {
        return C_LT;
    }
    return C_EQ;
}

c_object
c_setInsert (
    c_set _this,
    c_object o)
{
    C_STRUCT(c_set) *s = (C_STRUCT(c_set) *)_this;
    c_setNode f,n;

    assert(c_collectionIsType(_this, C_SET));

    n = (c_setNode)c_mmCacheMalloc(s->cache);
    n->object = c_keep(o);
    f = c_avlTreeInsert((c_avlTree)s,n,c_setCompare,NULL);
    if (f != n) {
        c_mmCacheFree(s->cache,n);
        c_free(o);
    }
    return f->object;
}

static c_object
c_setReplace (
    c_set _this,
    c_object o,
    c_bool (*condition)(),
    c_voidp arg)
{
    C_STRUCT(c_set) *s = (C_STRUCT(c_set) *)_this;
    c_setNode f,n;

    assert(c_collectionIsType(_this, C_SET));

    if (arg != NULL) {
        /* supress warnings */
    }
    n = (c_setNode)c_mmCacheMalloc(s->cache);
    n->object = c_keep(o);
    f = c_avlTreeReplace((c_avlTree)s,n,c_setCompare,NULL,condition,arg);
    if (f == NULL) {
        return NULL;
    }
    o = f->object;
    c_mmCacheFree(s->cache,f);
    return o;
}

c_object
c_setRemove (
    c_set _this,
    c_object o,
    c_removeCondition condition,
    c_voidp arg)
{
    C_STRUCT(c_set) *s = (C_STRUCT(c_set) *)_this;
    c_setNode f;
    C_STRUCT(c_setNode) n;
    c_object object;

    assert(c_collectionIsType(_this, C_SET));

    n.object = o;
    f = c_avlTreeRemove((c_avlTree)s,&n,c_setCompare,NULL,condition,arg);
    if (f == NULL) {
        return NULL;
    }
    object = f->object;
    c_mmCacheFree(s->cache,f);
    return object;
}

C_STRUCT(setReadActionArg) {
    c_qPred query;
    c_action action;
    c_voidp arg;
};
C_CLASS(setReadActionArg);

static c_bool
setReadAction(
    c_setNode n,
    setReadActionArg arg)
{
    c_qPred pred;

    pred = arg->query;
    if (pred == NULL) {
        return arg->action(n->object,arg->arg);
    } else {
        while (pred != NULL) {
            if (c_qPredEval(pred, n->object)) {
                return arg->action(n->object,arg->arg);
            }
            pred = pred->next;
        }
    }
    return TRUE;
}

static c_bool
c_setRead(
    c_set _this,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    C_STRUCT(setReadActionArg) a;

    assert(c_collectionIsType(_this, C_SET));

    a.query = q;
    a.action = action;
    a.arg = arg;
    return c_avlTreeWalk((c_avlTree)_this,setReadAction,&a,C_INFIX);
}

static c_iter
c_setSelect (
    c_set _this,
    c_qPred q,
    c_long max)
{
    C_STRUCT(collectActionArg) arg;

    assert(c_collectionIsType(_this, C_SET));

    arg.results = c_iterNew(NULL);
    if (max < 1) {
        max = 0x7fffffff; /* RP: MAXINT */
    }
    arg.max = max;
    c_setRead(_this,q,collectAction,&arg);
    return arg.results;
}

static c_object
c_setReadOne (
    c_set _this,
    c_qPred q)
{
    c_object o = NULL;

    assert(c_collectionIsType(_this, C_SET));

    c_setRead(_this,q,readOne,&o);
    return o;
}

static c_object
c_setTakeOne (
    c_set _this,
    c_qPred q)
{
    c_object o;
    c_object found;

    assert(c_collectionIsType(_this, C_SET));

    o = c_setReadOne(_this,q);
    if (o == NULL) {
        return NULL;
    }
    found = c_setRemove(_this,o,NULL,NULL);
    assert(found == o);
    c_free(found);

    return o;
}

static c_bool
c_setTake (
    c_set _this,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    c_object o;

    assert(c_collectionIsType(_this, C_SET));

    while (proceed) {
        o = c_setTakeOne(_this,q);
        if (o != NULL) {
            proceed = action(o,arg);
        } else {
            proceed = FALSE;
        }
    }
    return proceed;
}

c_long
c_setCount (
    c_set _this)
{
    assert(c_collectionIsType(_this, C_SET));
    return c_avlTreeCount((c_avlTree)_this);
}

typedef struct c_setWalkActionArg {
    c_action action;
    c_voidp arg;
} *c_setWalkActionArg;

static c_bool
c_setWalkAction(
    c_setNode n,
    c_setWalkActionArg arg)
{
    return arg->action(n->object,arg->arg);
}

c_bool
c_setWalk(
    c_set s,
    c_action action,
    c_voidp actionArg)
{
    struct c_setWalkActionArg arg;

    assert(c_collectionIsType(s, C_SET));

    arg.action = action;
    arg.arg = actionArg;
    return c_avlTreeWalk((c_avlTree)s,c_setWalkAction,&arg,C_INFIX);
}

/*============================================================================*/
/* BAG COLLECTION TYPE                                                        */
/*============================================================================*/

C_STRUCT(c_bagNode) {
    C_EXTENDS(c_avlNode);
    c_object object;
    c_long count;
};

static c_equality
c_bagCompare(
    c_bagNode n1,
    c_bagNode n2,
    void *args)
{
    if (args != NULL) {
        OS_REPORT(OS_WARNING,"c_bag",0,"c_bagCompare: parameter arg not NULL");
    }
    if (n2->object == NULL) return C_GT;
    if (n1->object == NULL) return C_LT;
    if ((c_voidp)n1->object > (c_voidp)n2->object) return C_GT;
    if ((c_voidp)n1->object < (c_voidp)n2->object) return C_LT;
    return C_EQ;
}

c_object
c_bagInsert (
    c_bag _this,
    c_object o)
{
    C_STRUCT(c_bag) *b = (C_STRUCT(c_bag) *)_this;
    c_bagNode f,n;

    assert(c_collectionIsType(_this, C_BAG));

    n = (c_bagNode)c_mmCacheMalloc(b->cache);
    n->object = c_keep(o);
    n->count = 1;
    f = c_avlTreeInsert((c_avlTree)b,n,c_bagCompare,NULL);
    if (f != n) {
        c_mmCacheFree(b->cache,n);
        c_free(o);
        f->count++;
    }
    b->count++;
    return f->object;
}

static c_object
c_bagReplace (
    c_bag _this,
    c_object o,
    c_bool (*condition)(),
    c_voidp arg)
{
    C_STRUCT(c_bag) *b = (C_STRUCT(c_bag) *)_this;
    c_bagNode f,n;

    assert(c_collectionIsType(_this, C_BAG));

    if (arg != NULL) {
        /* supress warnings */
    }
    n = (c_bagNode)c_mmCacheMalloc(b->cache);
    n->object = c_keep(o);
    n->count = 1;
    f = c_avlTreeReplace((c_avlTree)b,n,c_bagCompare,NULL,condition,arg);
    if (f == NULL) {
        o = NULL;
        b->count++;
    } else {
        o = f->object;
        c_mmCacheFree(b->cache,f);
    }
    return o;
}

c_object
c_bagRemove (
    c_bag _this,
    c_object o,
    c_removeCondition condition,
    c_voidp arg)
{
    C_STRUCT(c_bag) *b = (C_STRUCT(c_bag) *)_this;
    c_bagNode f;
    C_STRUCT(c_bagNode) n;
    c_object object;

    assert(c_collectionIsType(_this, C_BAG));

    n.object = o;
    f = c_avlTreeFind((c_avlTree)b,&n,c_bagCompare,NULL);
    if (f == NULL) return NULL;
    object = f->object;
    if (condition != NULL) {
       if (!condition(object,o,arg)) return NULL;
    }
    f->count--;
    b->count--;
    if (f->count == 0) {
        f = c_avlTreeRemove((c_avlTree)b,&n,c_bagCompare,NULL,NULL,NULL);
        c_mmCacheFree(b->cache,f);
    }
    return object;
}

C_STRUCT(bagReadActionArg) {
    c_qPred query;
    c_action action;
    c_voidp arg;
};
C_CLASS(bagReadActionArg);

static c_bool
bagReadAction(
    c_bagNode n,
    bagReadActionArg arg)
{
    c_long i;
    c_bool proceed = TRUE;
    c_qPred pred;

    pred = arg->query;
    if (pred == NULL) {
        proceed = arg->action(n->object,arg->arg);
    } else {
        while (pred != NULL) {
            if (c_qPredEval(pred, n->object)) {
                for (i=0; (i<n->count) && proceed; i++) {
                    proceed = arg->action(n->object,arg->arg);
                }
                pred = NULL;
            } else {
                pred = pred->next;
            }
        }
    }
    return proceed;
}

static c_bool
c_bagRead(
    c_bag _this,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    C_STRUCT(bagReadActionArg) a;

    assert(c_collectionIsType(_this, C_BAG));

    a.query = q;
    a.action = action;
    a.arg = arg;
    return c_avlTreeWalk((c_avlTree)_this,bagReadAction,&a,C_INFIX);
}

static c_iter
c_bagSelect (
    c_bag _this,
    c_qPred q,
    c_long max)
{
    C_STRUCT(collectActionArg) arg;

    assert(c_collectionIsType(_this, C_BAG));

    arg.results = c_iterNew(NULL);
    if (max < 1) {
        max = 0x7fffffff; /* RP: MAXINT */
    }
    arg.max = max;
    c_bagRead(_this,q,collectAction,&arg);
    return arg.results;
}

static c_object
c_bagReadOne (
    c_bag _this,
    c_qPred q)
{
    c_object o = NULL;

    assert(c_collectionIsType(_this, C_BAG));

    c_bagRead(_this,q,readOne,&o);
    return o;
}

static c_object
c_bagTakeOne (
    c_bag _this,
    c_qPred q)
{
    c_object o;
    c_object found;

    assert(c_collectionIsType(_this, C_BAG));

    o = c_bagReadOne(_this,q);
    if (o == NULL) {
        return NULL;
    }
    found = c_bagRemove(_this,o,NULL,NULL);
    assert(found == o);
    c_free(found);

    return o;
}

static c_bool
c_bagTake (
    c_bag _this,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    c_bool proceed = TRUE;
    c_object o;

    assert(c_collectionIsType(_this, C_BAG));

    while (proceed) {
        o = c_bagTakeOne(_this,q);
        if (o != NULL) {
            proceed = action(o,arg);
        } else {
            proceed = FALSE;
        }
    }
    return proceed;
}

c_long
c_bagCount (
    c_bag _this)
{
    assert(c_collectionIsType(_this, C_BAG));
    return ((C_STRUCT(c_bag) *)_this)->count;
}

typedef struct c_bagWalkActionArg {
    c_action action;
    c_voidp arg;
} *c_bagWalkActionArg;

static c_bool
c_bagWalkAction(
    c_bagNode n,
    c_bagWalkActionArg arg)
{
    arg->action(n->object,arg->arg);
    return TRUE;
}

c_bool
c_bagWalk(
    c_bag bag,
    c_action action,
    c_voidp actionArg)
{
    struct c_bagWalkActionArg arg;

    assert(c_collectionIsType(bag, C_BAG));

    arg.action = action;
    arg.arg = actionArg;
    return c_avlTreeWalk((c_avlTree)bag,c_bagWalkAction,&arg,C_INFIX);
}

/*============================================================================*/
/* TABLE COLLECTION TYPE                                                      */
/*============================================================================*/

C_STRUCT(c_tableNode) {
    C_EXTENDS(c_avlNode);
    c_value keyValue;
    c_object object;
#ifdef _CONSISTENCY_CHECKING_
    c_voidp table;
#endif
};

static c_equality
c_keyCompare(
    c_tableNode n1,
    c_tableNode n2,
    c_value *v)
{
    if (v != NULL) {
        OS_REPORT(OS_WARNING,"c_table",0,"c_keyCompare: parameter v not NULL");
    }
    if (n2 == NULL) {
        return C_GT;
    }
    if (n1 == NULL) {
        return C_LT;
    }
    return c_valueCompare(n1->keyValue,n2->keyValue);
}


c_object
c_tableInsert (
    c_table _this,
    c_object o)
{
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)_this;
    c_tableNode f,n;
    c_object *index;
    c_long i, nrOfKeys;
    c_mm mm;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    if (o == NULL) {
        return NULL;
    }

    _ACCESS_BEGIN_(t);

    mm = MM(t);
    n = NULL;
    index = &t->object;

    if (t->key == NULL) {
        nrOfKeys = 0;
    } else {
        nrOfKeys = c_arraySize(t->key);
    }
    for (i=0; i<nrOfKeys; i++) {
        if (n == NULL) {
            n = (c_tableNode)c_mmCacheMalloc(t->cache);
            memset(n,0,C_SIZEOF(c_tableNode));
#ifdef _CONSISTENCY_CHECKING_
            n->table = (c_voidp)_this;
        } else {
            _CHECK_CONSISTENCY_(t,n);
#endif
        }
        n->keyValue = c_fieldValue(t->key[i],o);
        if (*index == NULL) {
            *index = c_avlTreeNew(mm,0);
        }
        f = c_avlTreeInsert(*index,n,c_keyCompare,NULL);
        if (f == n) {
            n = NULL;
        } else {
            c_valueFreeRef(n->keyValue);
            _CHECK_CONSISTENCY_(t,f);
        }
        index = &f->object;
    }
    if (n != NULL) {
        /* do not free keyvalue here, already done in for loop */
        c_mmCacheFree(t->cache,n);
    }
    if (*index == NULL) {
        t->count++;
        *index = c_keep(o);
    }

    _ACCESS_END_(t);

    return *index;
}

static c_object
c_tableReplace (
    c_table _this,
    c_object o,
    c_bool (*condition)(),
    c_voidp arg)
{
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)_this;
    c_tableNode f,n;
    c_object *index;
    c_object object = NULL;
    c_long i, nrOfKeys;
    c_mm mm;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    if (o == NULL) {
        return NULL;
    }

    _ACCESS_BEGIN_(t);

    mm = MM(t);
    n = NULL;
    index = &t->object;

    if (t->key == NULL) {
        nrOfKeys = 0;
    } else {
        nrOfKeys = c_arraySize(t->key);
    }
    for (i=0; i<nrOfKeys; i++) {
        if (n == NULL) {
            n = (c_tableNode)c_mmCacheMalloc(t->cache);
            memset(n,0,C_SIZEOF(c_tableNode));
#ifdef _CONSISTENCY_CHECKING_
            n->table = (c_voidp)_this;
        } else {
            _CHECK_CONSISTENCY_(t,n);
#endif
        }
        n->keyValue = c_fieldValue(t->key[i],o);
        if (n->keyValue.kind == V_UNDEFINED) {
            OS_REPORT_1(OS_WARNING,"Database Collection",0,
                        "c_tableReplace: Key (%s) value undefined",
                        c_fieldName(t->key[i]));
            c_mmCacheFree(t->cache,n);
            _ACCESS_END_(t);
            return NULL;
        }
        if (*index == NULL) {
            *index = c_avlTreeNew(mm,0);
        }
        f = c_avlTreeInsert(*index,n,c_keyCompare,NULL);
        _CHECK_CONSISTENCY_(t,f);
        if (f != n) {
            c_valueFreeRef(n->keyValue);
            c_mmCacheFree(t->cache,n);
        }
        n = NULL;
        index = &f->object;
    }
    object = *index;
    if (condition != NULL) {
        if (condition(object,o,arg)) {
            *index = c_keep(o);
        } else {
            object = NULL;
        }
    } else {
        if (*index == NULL) {
            t->count++;
        }
        *index = c_keep(o);
    }
    _ACCESS_END_(t);

    return object;
}


C_STRUCT(removeConditionArg) {
    c_removeCondition condition;
    c_object object;
    c_voidp arg;
};

static c_bool
c_tableRemoveConditionWrapper(
    c_object o1,
    c_object o2,
    c_voidp arg)
{
    c_tableNode n1 = (c_tableNode)o1;
    C_STRUCT(removeConditionArg) *remArg = (C_STRUCT(removeConditionArg)*)arg;

    if (remArg->condition != NULL) {
        return remArg->condition(n1->object, remArg->object, remArg->arg);
    } else {
        return TRUE;
    }
}

c_object
c_tableRemove (
    c_table _this,
    c_object o,
    c_removeCondition condition,
    c_voidp arg)
{
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)_this;
    C_STRUCT(c_tableNode) n;
    c_tableNode *stack;
    c_tableNode found;
    c_object object;
    c_object index;
    c_long i, nrOfKeys;
    c_bool allowed = TRUE;
    C_STRUCT(removeConditionArg) wrapperArg;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _ACCESS_BEGIN_(t);

    if (t->object == NULL) {
        _ACCESS_END_(t);
        return NULL;
    }
    if (t->key == NULL) {
        nrOfKeys = 0;
    } else {
        nrOfKeys = c_arraySize(t->key);
    }
    if (nrOfKeys == 0) {
        assert(t->object != NULL);
        if (condition != NULL) {
            allowed = condition(t->object, o, arg);
        }
        if (allowed) {
            t->count--;
            object = t->object;
            t->object = NULL;
        } else {
            object = NULL;
        }
        _ACCESS_END_(t);
        return object;
    }
    stack = (c_tableNode *)os_alloca(sizeof(c_tableNode)*nrOfKeys);
    index = t->object;
    for (i=0; i<(nrOfKeys-1); i++) {
        n.keyValue = c_fieldValue(t->key[i],o);
        if (n.keyValue.kind == V_UNDEFINED) {
            OS_REPORT_1(OS_WARNING,"Database Collection",0,
                        "c_tableRemove: Key (%s) value undefined",
                        c_fieldName(t->key[i]));
            _ACCESS_END_(t);
            return o;
        }
        stack[i] = c_avlTreeFind(index,&n,c_keyCompare,NULL);
        if (stack[i] == NULL) {
            os_freea(stack);
            _ACCESS_END_(t);
            return NULL;
#ifdef _CONSISTENCY_CHECKING_
        } else {
            _CHECK_CONSISTENCY_(t,stack[i]);
#endif
        }
        index = stack[i]->object;
        c_valueFreeRef(n.keyValue);
    }
    n.keyValue = c_fieldValue(t->key[i],o);
    if (n.keyValue.kind == V_UNDEFINED) {
        OS_REPORT_1(OS_WARNING,"Database Collection",0,
                    "c_tableRemove: Key (%s) value undefined",
                     c_fieldName(t->key[i]));
        _ACCESS_END_(t);
        return o;
    }

    wrapperArg.condition = condition;
    wrapperArg.object = o;
    wrapperArg.arg = arg;
    found = c_avlTreeRemove(index,&n,c_keyCompare,NULL,
                            c_tableRemoveConditionWrapper,&wrapperArg);
    if (found != NULL) {
        t->count--;
        for(i-=1;i>0;i--) {
            if (c_avlTreeCount(stack[i]->object) == 0) {
                c_avlTreeRemove(stack[i-1]->object,stack[i],
                                c_keyCompare,
                                NULL, NULL, NULL);
                c_avlTreeFree(stack[i]->object);
                c_valueFreeRef(stack[i]->keyValue);
                c_mmCacheFree(t->cache,stack[i]);
            } else {
                break;
            }
        }
        if (i==0) {
            if (c_avlTreeCount(stack[0]->object) == 0) {
                c_avlTreeRemove(t->object,stack[0],
                                c_keyCompare,
                                NULL, NULL, NULL);
                c_avlTreeFree(stack[0]->object);
                c_valueFreeRef(stack[0]->keyValue);
                c_mmCacheFree(t->cache,stack[0]);
            }
        }
        if (c_avlTreeCount(t->object) == 0) {
            c_avlTreeFree(t->object);
            t->object = NULL;
        }
        object = found->object;
        c_valueFreeRef(found->keyValue);
        c_mmCacheFree(t->cache,found);
    } else {
        object = NULL;
    }
    c_valueFreeRef(n.keyValue);
    os_freea(stack);
    _ACCESS_END_(t);

    return object;
}

#ifdef _NIL_
c_object
c_tableFind (
    c_table _this,
    c_value *keyValues)
{
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)_this;
    C_STRUCT(c_tableNode) n;
    c_tableNode found;
    c_object object;
    c_long i, nrOfKeys;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _READ_BEGIN_(t);

    if (t->object == NULL) {
        _READ_END_(t);
        return NULL;
    }
    if (keyValues == NULL) {
        _READ_END_(t);
        return t->object;
    }
    nrOfKeys = c_arraySize(t->key);

    object = t->object;
    for (i=0; (i<nrOfKeys) && (object); i++) {
        n.keyValue = keyValues[i];
        assert(n.keyValue.kind != V_UNDEFINED);
        found = c_avlTreeFind(object,&n,c_keyCompare,NULL);
        if (found == NULL) {
            _READ_END_(t);
            return NULL;
        }
        object = found->object;
    }
    _READ_END_(t);
    return c_keep(object);
}
#endif

static c_tableNode
tableNext(
    c_object o,
    c_object index,
    c_array keyList,
    c_long keyId)
{
    C_STRUCT(c_tableNode) n;
    c_tableNode found;
    c_long lastKey = c_arraySize(keyList)-1;

    n.keyValue = c_fieldValue(keyList[keyId],o);

    if (n.keyValue.kind == V_UNDEFINED) {
        OS_REPORT_1(OS_WARNING,"Database Collection",0,
                    "c_tableNext: Key (%s) value undefined",
                    c_fieldName(keyList[keyId]));
        return o; /* Note that under normal circumstances it is not
                     possible that the same object is returned,
                     in this case this knowledge is used to indicate that
                     an error has occured. */
    }
    if (keyId < lastKey) {
        found = c_avlTreeFind(index,&n,c_keyCompare,NULL);
        if (found != NULL) {
            found = tableNext(o,found->object,keyList,(keyId+1));
        }
        if (found == NULL) {
            found = c_avlTreeNearest(index,&n,c_keyCompare,NULL,C_GT);
            if (found == NULL) {
                return NULL;
            }
            while (keyId != lastKey) {
                found = c_avlTreeFirst((c_avlTree)found->object);
                keyId++;
            }
        }
    } else {
        found = c_avlTreeNearest(index,&n,c_keyCompare,NULL,C_GT);
    }
    c_valueFreeRef(n.keyValue);

    return found;
}

c_object
c_tableNext (
    c_table table,
    c_object o)
{
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)table;
    c_tableNode node;
    c_long nrOfKeys;
    c_object data;

    assert(c_collectionIsType(table, C_DICTIONARY));

    _READ_BEGIN_(t);
    if (t == NULL) {
        _READ_END_(t);
        return NULL;
    }
    if (t->object == NULL) {
        _READ_END_(t);
        return NULL;
    }
    if (t->key == NULL) {
        _READ_END_(t);
        if (o == NULL) {
            return t->object;
        }
        return NULL;
    }
    nrOfKeys = c_arraySize(t->key);
    if (nrOfKeys == 0) {
        _READ_END_(t);
        if (o == NULL) {
            _CHECK_CONSISTENCY_(t,t->object);
            return t->object;
        }
        return NULL;
    }
    if (o == NULL) {
        node = t->object;
        data = node;
        while (nrOfKeys > 0) {
            node = c_avlTreeFirst((c_avlTree)data);
            if (node == NULL) {
                return NULL;
            } else {
                _CHECK_CONSISTENCY_(t,node);
                data = node->object;
            }
            nrOfKeys--;
        }
        _READ_END_(t);
        return data;
    } else {
        node = tableNext(o,t->object,t->key,0);
        _READ_END_(t);
        if (node == NULL) { /* o is last record of table */
            return NULL;
        } else {
            return node->object;
        }
    }
}

C_STRUCT(tableReadActionArg) {
    c_array key;
    c_long keyIndex;
    c_qPred query;
    c_action action;
    c_voidp arg;
#ifdef _CONSISTENCY_CHECKING_
    C_STRUCT(c_table) *t;
#endif
};
C_CLASS(tableReadActionArg);

static c_bool
tableReadAction(
    c_tableNode n,
    tableReadActionArg arg)
{
    C_STRUCT(c_tableNode) start, end;
    c_tableNode startRef, endRef;
    c_bool startInclude, endInclude;
    c_bool proceed = TRUE;
    c_qKey key;
    c_qRange range;
    c_long i,nrOfRanges;
    c_value v;
    c_qPred pred;

    _CHECK_CONSISTENCY_(arg->t,n);

    if ((arg->keyIndex > 0) && (arg->query != NULL)) {
        key = arg->query->keyField[arg->keyIndex-1];
        if (key->expr != NULL) {
            v = c_qValue(key->expr,n->object);
            assert(v.kind == V_BOOLEAN);
            if (!v.is.Boolean) {
                return TRUE;
            }
        }
    }
    if (arg->query == NULL) {
        if (arg->keyIndex < (c_arraySize(arg->key))) {
            arg->keyIndex++;
            proceed = c_avlTreeWalk((c_avlTree)n->object,
                                    tableReadAction,arg,
                                    C_INFIX);
            arg->keyIndex--;
            return proceed;
        }
    } else if (arg->keyIndex == c_arraySize(arg->key)) {
        pred = arg->query;
        v.is.Boolean = TRUE;
        while (pred != NULL) {
            if (pred->expr != NULL) {
                v = c_qValue(pred->expr, n->object);
                assert(v.kind == V_BOOLEAN);
                if (v.is.Boolean) {
                    pred = NULL;
                } else {
                    pred = pred->next;
                }
            } else {
                pred = pred->next;
            }
        }
        if (!v.is.Boolean) {
            return TRUE;
        }
    } else {
        key = arg->query->keyField[arg->keyIndex];
        arg->keyIndex++;
        if ((key->range == NULL) || (c_arraySize(key->range) == 0)) {
            proceed = c_avlTreeWalk((c_avlTree)n->object,
                                    tableReadAction,arg,
                                    C_INFIX);
        } else {
            nrOfRanges = c_arraySize(key->range);
            i=0;
            while ((i<nrOfRanges) && proceed) {
                range = key->range[i];
                if (range == NULL) {
                    startRef = NULL; startInclude = TRUE;
                    endRef   = NULL; endInclude   = TRUE;
                } else {
                    start.keyValue = c_qRangeStartValue(range);
                    end.keyValue = c_qRangeEndValue(range);
                    switch (range->startKind) {
                    case B_UNDEFINED: startRef = NULL;   startInclude = TRUE;  break;
                    case B_INCLUDE:   startRef = &start; startInclude = TRUE;  break;
                    case B_EXCLUDE:   startRef = &start; startInclude = FALSE; break;
                    default:
                        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                                    "Internal error: undefined range kind %d",range->startKind);
                        assert(FALSE);
                        return FALSE;
                    }
                    switch (range->endKind) {
                    case B_UNDEFINED: endRef = NULL; endInclude = TRUE;  break;
                    case B_INCLUDE:   endRef = &end; endInclude = TRUE;  break;
                    case B_EXCLUDE:   endRef = &end; endInclude = FALSE; break;
                    default:
                        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                                    "Internal error: undefined range kind %d",range->endKind);
                        assert(FALSE);
                        return FALSE;
                    }
                }
                proceed = c_avlTreeRangeWalk((c_avlTree)n->object,
                                              startRef,startInclude,
                                              endRef,endInclude,
                                              c_keyCompare,NULL,
                                              tableReadAction,arg,
                                              C_INFIX);
                i++;
            }
        }
        arg->keyIndex--;
        return proceed;
    }
    return arg->action(n->object,arg->arg);
}

static c_bool
c_tableRead (
    c_table _this,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)_this;
    C_STRUCT(tableReadActionArg) a;
    C_STRUCT(c_tableNode) root;
    c_value v;
    c_bool proceed = TRUE;
    c_qPred pred;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _READ_BEGIN_(t);
    if (t->object == NULL) {
        _READ_END_(t);
        return proceed;
    }

    if ((t->key == NULL) || (c_arraySize(t->key) == 0)) {
        if (q == NULL) {
            proceed = action(t->object,arg);
        } else {
            pred = q;
            while (pred != NULL) {
                if (pred->expr != NULL) {
                    v = c_qValue(c_qExpr(pred->expr), t->object);
                    assert(v.kind == V_BOOLEAN);
                    if (v.is.Boolean) {
                        proceed = action(t->object,arg);
                        pred = NULL;
                    } else {
                        pred = pred->next;
                    }
                } else {
                    pred = pred->next;
                }
            }
        }
        _READ_END_(t);
        return proceed;
    }

    root.object = t->object;
    a.key = t->key;
    a.action = action;
    a.arg = arg;
#ifdef _CONSISTENCY_CHECKING_
    a.t = t;
    root.table = (c_voidp)t;
#endif
    if (q == NULL) {
        a.keyIndex = 0;
        a.query = q;
        proceed = tableReadAction(&root,&a);
    }
    while ((q != NULL) && proceed) {
        a.keyIndex = 0;
        a.query = q;
        proceed = tableReadAction(&root,&a);
        q = q->next;
    }
    _READ_END_(t);
    return proceed;
}

static c_iter
c_tableSelect (
    c_table _this,
    c_qPred q,
    c_long max)
{
    C_STRUCT(collectActionArg) arg;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    arg.results = c_iterNew(NULL);
    if (max < 1) {
        max = 0x7fffffff; /* RP: MAXINT */
    }
    arg.max = max;
    c_tableRead(_this,q,collectAction,&arg);
    return arg.results;
}

static c_object
c_tableReadOne (
    c_table t,
    c_qPred q)
{
    c_object o = NULL;

    assert(c_collectionIsType(t, C_DICTIONARY));

    c_tableRead(t,q,readOne,&o);
    return o;
}

C_STRUCT(tableTakeActionArg) {
    c_array key;
    c_long keyIndex;
    c_qPred pred;
    c_tableNode disposed;
    c_action action;
    c_voidp arg;
    c_long count;
    c_bool proceed;
    c_mm mm;
    c_mmCache cache;
#ifdef _CONSISTENCY_CHECKING_
    C_STRUCT(c_table) *t;
#endif
};

C_CLASS(tableTakeActionArg);

static c_bool
tableTakeAction(
    c_tableNode n,
    tableTakeActionArg arg)
{
    C_STRUCT(c_tableNode) start, end;
    c_tableNode startRef, endRef, found;
    c_bool startInclude, endInclude;
    c_bool proceed = TRUE;
    c_qKey key;
    c_qRange range;
    c_long i,nrOfRanges;
    c_value v;
    c_qPred pred;

    _CHECK_CONSISTENCY_(arg->t,n);

    if ((arg->keyIndex > 0) && (arg->pred != NULL)) {
        key = arg->pred->keyField[arg->keyIndex-1];
        if (key->expr != NULL) {
            v = c_qValue(key->expr,n->object);
            assert(v.kind == V_BOOLEAN);
            if (!v.is.Boolean) {
                return TRUE;
            }
        }
    }
    if (arg->pred == NULL) {
        if (arg->keyIndex < (c_arraySize(arg->key))) {
            arg->keyIndex++;
            do {
                proceed = c_avlTreeWalk((c_avlTree)n->object,
                                        tableTakeAction,
                                        arg,
                                        C_INFIX);
                if ((!proceed) && (arg->disposed != NULL)) {
                    found = c_avlTreeRemove(n->object,
                                            arg->disposed,
                                            c_keyCompare,
                                            NULL,NULL,NULL);
                    assert(found == arg->disposed);
                    if (arg->keyIndex == c_arraySize(arg->key)) {
                        c_free(found->object);
                    } else {
                        c_avlTreeFree(found->object);
                    }
                    c_valueFreeRef(found->keyValue);
                    c_mmCacheFree(arg->cache,found);
                }
            } while ((proceed == FALSE) && (arg->proceed == TRUE));
            proceed = arg->proceed;
            if (c_avlTreeCount(n->object) == 0) {
                arg->disposed = n;
            } else {
                arg->disposed = NULL;
            }
            arg->keyIndex--;
            return proceed;
        }
    } else if (arg->keyIndex == c_arraySize(arg->key)) {
        pred = arg->pred;
        v.is.Boolean = TRUE;
        while (pred != NULL) {
            if (pred->expr != NULL) {
                v = c_qValue(pred->expr, n->object);
                assert(v.kind == V_BOOLEAN);
                if (v.is.Boolean) {
                    pred = NULL;
                } else {
                    pred = pred->next;
                }
            } else {
                pred = pred->next;
            }
        }
        if (!v.is.Boolean) {
            return TRUE;
        }
    } else {
        key = arg->pred->keyField[arg->keyIndex];
        arg->keyIndex++;
        if ((key->range == NULL) || (c_arraySize(key->range) == 0)) {
            do {
                proceed = c_avlTreeWalk((c_avlTree)n->object,
                                        tableTakeAction,
                                        arg,
                                        C_INFIX);
                if ((!proceed) && (arg->disposed != NULL)) {
                    assert(arg->disposed != NULL);
                    found = c_avlTreeRemove(n->object,
                                            arg->disposed,
                                            c_keyCompare,
                                            NULL,NULL,NULL);
                    assert(found == arg->disposed);
                    if (arg->keyIndex == c_arraySize(arg->key)) {
                        c_free(found->object);
                    } else {
                        c_avlTreeFree(found->object);
                    }
                    c_valueFreeRef(found->keyValue);
                    c_mmCacheFree(arg->cache,found);
                }
            } while ((proceed == FALSE) && (arg->proceed == TRUE));
            proceed = arg->proceed;
        } else {
            nrOfRanges = c_arraySize(key->range);
            i=0;
            while ((i<nrOfRanges) && proceed) {
                range = key->range[i];
                if (range == NULL) {
                    startRef = NULL; startInclude = TRUE;
                    endRef   = NULL; endInclude   = TRUE;
                } else {
                    start.keyValue = c_qRangeStartValue(range);
                    end.keyValue = c_qRangeEndValue(range);
                    switch (range->startKind) {
                    case B_UNDEFINED: startRef = NULL;   startInclude = TRUE;  break;
                    case B_INCLUDE:   startRef = &start; startInclude = TRUE;  break;
                    case B_EXCLUDE:   startRef = &start; startInclude = FALSE; break;
                    default:
                        OS_REPORT_1(OS_ERROR,
                                    "Database Collection",0,
                                    "Internal error: undefined range kind %d",
                                    range->startKind);
                        assert(FALSE);
                        return FALSE;
                    }
                    switch (range->endKind) {
                    case B_UNDEFINED: endRef = NULL; endInclude = TRUE;  break;
                    case B_INCLUDE:   endRef = &end; endInclude = TRUE;  break;
                    case B_EXCLUDE:   endRef = &end; endInclude = FALSE; break;
                    default:
                        OS_REPORT_1(OS_ERROR,
                                    "Database Collection",0,
                                    "Internal error: undefined range kind %d",
                                    range->endKind);
                        assert(FALSE);
                        return FALSE;
                    }
                }
                do {
                    proceed = c_avlTreeRangeWalk((c_avlTree)n->object,
                                                  startRef,startInclude,
                                                  endRef,endInclude,
                                                  c_keyCompare,NULL,
                                                  tableTakeAction,arg,
                                                  C_INFIX);
                    if ((!proceed) && (arg->disposed != NULL)) {
                        assert(arg->disposed != NULL);
                        found = c_avlTreeRemove(n->object,
                                                arg->disposed,
                                                c_keyCompare,
                                                NULL,NULL,NULL);
                        assert(found == arg->disposed);
                        if (arg->keyIndex == c_arraySize(arg->key)) {
                            c_free(found->object);
                        } else {
                            c_avlTreeFree(found->object);
                        }
                        c_valueFreeRef(found->keyValue);
                        c_mmCacheFree(arg->cache,found);
                    }
                } while ((proceed == FALSE) && (arg->proceed == TRUE));
                proceed = arg->proceed;
                i++;
            }
        }
        if (c_avlTreeCount(n->object) == 0) {
            arg->disposed = n;
        } else {
            arg->disposed = NULL;
        }
        arg->keyIndex--;
        return proceed;
    }
    arg->disposed = n;
    arg->count++;
    arg->proceed = arg->action(n->object,arg->arg);
    return FALSE;
}

static c_bool
c_tableTake (
    c_table _this,
    c_qPred p,
    c_action action,
    c_voidp arg)
{
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)_this;
    C_STRUCT(tableTakeActionArg) a;
    C_STRUCT(c_tableNode) root;
    c_value v;
    c_bool proceed = TRUE;
    c_long nrOfKeys;
    c_object o = NULL;
    c_qPred pred;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _ACCESS_BEGIN_(t);
    if (t->key == NULL) {
        nrOfKeys = 0;
    } else {
        nrOfKeys = c_arraySize(t->key);
    }

    if (t->object == NULL) {
        _ACCESS_END_(t);
        return FALSE;
    }

    if (nrOfKeys == 0) {
        if (p == NULL) {
            o = t->object;
            t->object = NULL;
            t->count--;
        } else {
            pred = p;
            while (pred != NULL) {
                if (pred->expr != NULL) {
                    v = c_qValue(c_qExpr(pred->expr), t->object);
                    assert(v.kind == V_BOOLEAN);
                    if (v.is.Boolean) {
                        o = t->object;
                        t->object = NULL;
                        t->count--;
                        pred = NULL;
                    } else {
                        pred = pred->next;
                    }
                } else {
                    pred = pred->next;
                }
            }
        }
        proceed = action(o,arg);
        c_free(o);
        _ACCESS_END_(t);
        return proceed;
    }

    a.mm = MM(t);
    a.key = t->key;
    a.action = action;
    a.arg = arg;
    a.count = 0;
    a.proceed = TRUE;
    a.cache = t->cache;
#ifdef _CONSISTENCY_CHECKING_
    a.t = t;
    root.table = (c_voidp)t;
#endif
    root.object = t->object;
    if (p == NULL) {
        a.keyIndex = 0;
        a.pred = p;
        a.disposed = NULL;
        proceed = tableTakeAction(&root,&a);
    }
    while ((p != NULL) && proceed) {
        a.keyIndex = 0;
        a.pred = p;
        a.disposed = NULL;
        proceed = tableTakeAction(&root,&a);
        p = p->next;
    }
    t->count -= a.count;
    _ACCESS_END_(t);
    return proceed;
}

static c_object
c_tableTakeOne(
    c_table t,
    c_qPred p)
{
    c_object o = NULL;

    assert(c_collectionIsType(t, C_DICTIONARY));

    c_tableTake(t,p,readOne,&o);

    return o;
}

c_long
c_tableCount (
    c_table _this)
{
    assert(c_collectionIsType(_this, C_DICTIONARY));
    return ((C_STRUCT(c_table) *)_this)->count;
}

typedef struct c_tableWalkActionArg {
    c_action action;
    c_voidp actionArg;
    c_long nrOfKeys;
} *c_tableWalkActionArg;

static c_bool
c_tableWalkAction (
    c_tableNode n,
    c_tableWalkActionArg arg)
{
    c_bool result;

    if (arg->nrOfKeys == 0) {
        result = arg->action(n->object,arg->actionArg);
    } else {
        arg->nrOfKeys--;
        result = c_avlTreeWalk(n->object,c_tableWalkAction,arg,C_INFIX);
        arg->nrOfKeys++;
    }
    return result;
}

c_bool
c_tableWalk(
    c_table _this,
    c_action action,
    c_voidp actionArg)
{
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)_this;
    struct c_tableWalkActionArg walkActionArg;
    c_bool result = TRUE;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _READ_BEGIN_(t);
    if (t->count > 0) {
        if ((t->key == NULL) || (c_arraySize(t->key) == 0)) {
            result = action(t->object,actionArg);
        } else {
            walkActionArg.action = action;
            walkActionArg.actionArg = actionArg;
            walkActionArg.nrOfKeys = c_arraySize(t->key) - 1;
            result = c_avlTreeWalk(t->object,
                                   c_tableWalkAction,
                                   &walkActionArg,
                                   C_INFIX);
        }
    }
    _READ_END_(t);
    return result;
}

c_long
c_tableGetKeyValues(
    c_table table,
    c_object object,
    c_value *values)
{
    c_long i, nrOfKeys;
    c_value *currentValuePtr = values;
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)table;

    assert(c_collectionIsType(table, C_DICTIONARY));

    assert(table != NULL);
    assert(object != NULL);

    nrOfKeys = c_arraySize(t->key);
    for (i=0;i<nrOfKeys;i++) {
        *currentValuePtr = c_fieldValue(t->key[i], object);
        currentValuePtr = &(currentValuePtr[1]);
    }
    return nrOfKeys;
}

c_long
c_tableSetKeyValues(
    c_table table,
    c_object object,
    c_value *values)
{
    c_long i, nrOfKeys;
    c_value *currentValue;
    c_field *currentField;
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)table;

    assert(c_collectionIsType(table, C_DICTIONARY));

    nrOfKeys = c_arraySize(t->key);
    if (nrOfKeys > 0) {
        currentField = (c_field *)t->key;
        currentValue = values;
        for (i=0;i<nrOfKeys;i++) {
            c_fieldAssign(*currentField, object, *currentValue);
            currentField = &(currentField[1]);
            currentValue = &(currentValue[1]);
        }
    }
    return nrOfKeys;
}

c_long
c_tableNofKeys(
    c_table table)
{
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)table;

    assert(c_collectionIsType(table, C_DICTIONARY));

    return c_arraySize(t->key);
}

c_array
c_tableKeyList(
    c_table table)
{
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)table;

    assert(c_collectionIsType(table, C_DICTIONARY));

    return c_keep(t->key);
}

c_char *
c_tableKeyExpr(
    c_table table)
{
    c_long i, nrOfKeys, size;
    c_char *expr;
    C_STRUCT(c_table) *t = (C_STRUCT(c_table) *)table;

    assert(c_collectionIsType(table, C_DICTIONARY));

    size = 0;
    nrOfKeys = c_arraySize(t->key);
    for (i=0;i<nrOfKeys;i++) {
        size += strlen(c_fieldName(t->key[i])) + 1;
    }
    expr = (c_char *)os_malloc(size);
    expr[0] = (c_char)0;
    for (i=0;i<nrOfKeys;i++) {
        os_strcat(expr,c_fieldName(t->key[i]));
        if (i < (nrOfKeys-1)) {
            os_strcat(expr,",");
        }
    }
    return expr;
}

/*============================================================================*/
/* QUERY COLLECTION TYPE                                                      */
/*============================================================================*/

C_STRUCT(queryReadActionArg) {
    c_action action;
    c_voidp arg;
    c_qPred predicate;
};

C_CLASS(queryReadActionArg);

static c_bool
queryReadAction (
    c_object o,
    c_voidp arg)
{
    queryReadActionArg a = (queryReadActionArg)arg;
    c_bool proceed = TRUE;
    if (c_qPredEval(a->predicate,o)) {
        proceed = a->action(o,a->arg);
    }
    return proceed;
}

static c_bool
c_queryRead(
    C_STRUCT(c_query) *query,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    C_STRUCT(queryReadActionArg) a;
    c_qPred pred;
    c_collection source;
    c_type type;

    assert(c_collectionIsType((c_query)query, C_QUERY));

    pred = query->pred;
    source = query->source;
    type = c__getType(source);
    type = c_typeActualType(type);

    a.action = action;
    a.arg = arg;
    a.predicate = q;

    switch(c_collectionType(type)->kind) {
    case C_QUERY:      return c_queryRead(c_query(source),pred,queryReadAction,&a);
    case C_DICTIONARY: return c_tableRead(source,pred,queryReadAction,&a);
    case C_SET:        return c_setRead(source,pred,queryReadAction,&a);
    case C_BAG:        return c_bagRead(source,pred,queryReadAction,&a);
    case C_LIST:       return c_listRead(source,pred,queryReadAction,&a);
    default:
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_queryRead: illegal collection kind (%d) specified",
                    c_collectionType(type)->kind);
        assert(FALSE);
    break;
    }
    return NULL;
}

static c_object
c_queryReadOne (
    C_STRUCT(c_query) *b,
    c_qPred q)
{
    c_object o = NULL;

    assert(c_collectionIsType((c_query)b, C_QUERY));

    c_queryRead(b,q,readOne,&o);
    return o;
}

static c_iter
c_querySelect (
    C_STRUCT(c_query) *query,
    c_qPred q,
    c_long max)
{
    c_iter r,s;
    c_object o;
    c_type type;
    c_collection source;
    c_qPred pred;

    assert(c_collectionIsType((c_query)query, C_QUERY));

    source = query->source;
    type = c__getType(source);
    type = c_typeActualType(type);
    pred = query->pred;

    switch(c_collectionType(type)->kind) {
        case C_QUERY:      r = c_querySelect(c_query(source),pred,max); break;
        case C_DICTIONARY: r = c_tableSelect(source,pred,max); break;
        case C_SET:        r = c_setSelect(source,pred,max);     break;
        case C_BAG:        r = c_bagSelect(source,pred,max);     break;
        case C_LIST:       r = c_listSelect(source,pred,max);   break;
        default:
            r = NULL;
            OS_REPORT_1(OS_ERROR,"Database Collection",0,
                        "c_querySelect: illegal collection kind (%d) specified",
                        c_collectionType(type)->kind);
            assert(FALSE);
        break;
    }
    s = NULL;
    while ((o = c_iterTakeFirst(r)) != NULL) {
        if (c_qPredEval(q,o)) {
            s = c_iterInsert(s,o);
        } else {
            c_free(o);
        }
    }
    c_iterFree(r);
    return s;
}

static c_bool
c_queryTake(
    C_STRUCT(c_query) *query,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    C_STRUCT(queryReadActionArg) a;
    c_qPred pred;
    c_collection source;
    c_type type;

    assert(c_collectionIsType((c_query)query, C_QUERY));

    pred = query->pred;
    source = query->source;
    type = c__getType(source);
    type = c_typeActualType(type);

    a.action = action;
    a.arg = arg;
    a.predicate = q;

    switch(c_collectionType(type)->kind) {
    case C_QUERY:      return c_queryTake(c_query(source),pred,queryReadAction,&a);
    case C_DICTIONARY: return c_tableTake(source,pred,queryReadAction,&a);
    case C_SET:        return c_setTake(source,pred,queryReadAction,&a);
    case C_BAG:        return c_bagTake(source,pred,queryReadAction,&a);
    case C_LIST:       return c_listTake(source,pred,queryReadAction,&a);
    default:
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_queryTake: illegal collection kind (%d) specified",
                    c_collectionType(type)->kind);
        assert(FALSE);
    break;
    }
    return NULL;
}

static c_object
c_queryTakeOne (
    C_STRUCT(c_query) *b,
    c_qPred q)
{
    c_object o = NULL;

    assert(c_collectionIsType((c_query)b, C_QUERY));

    c_queryTake(b,q,readOne,&o);
    return o;
}

c_qPred
c_queryGetPred(
		c_query _this)
{
	assert(_this);
	if(_this){
		return c_query(_this)->pred;
	} else {
	    OS_REPORT(OS_ERROR,"Database Collection",0,
	                       "c_queryGetPred: given query is NULL");
	}

	return NULL;
}

void
c_querySetPred(
		c_query _this,
		c_qPred p)
{
	assert(_this);

	if(_this){
		c_query(_this)->pred = p;
	} else {
	    OS_REPORT(OS_ERROR,"Database Collection",0,
	                       "c_querySetPred: given query is NULL");
	}
}

/*============================================================================*/
/* C_COLLECTION TYPE                                                          */
/*============================================================================*/

c_object
c_insert (
    c_collection c,
    c_object o)
{
    c_type type = c__getType(c);

    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_insert: given entity (%d) is not a collection",
                    c_collectionType(type)->kind);
        assert(FALSE);
        return NULL;
    }

    switch(c_collectionType(type)->kind) {
        case C_DICTIONARY: return c_tableInsert(c,o);
        case C_SET:        return c_setInsert(c,o);
        case C_BAG:        return c_bagInsert(c,o);
        case C_LIST:       return c_listInsert(c,o);
        default:
            OS_REPORT_1(OS_ERROR,"Database Collection",0,
                        "c_insert: illegal collection kind (%d) specified",
                        c_collectionType(type)->kind);
            assert(FALSE);
        break;
    }
    return NULL;
}

c_object
c_remove (
    c_collection c,
    c_object o,
    c_removeCondition condition,
    c_voidp arg)
{
    c_type type = c__getType(c);

    if (o == NULL) {
        return NULL;
    }
    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_remove: given entity (%d) is not a collection",
                    c_collectionType(type)->kind);
        assert(FALSE);
        return NULL;
    }

    switch(c_collectionType(type)->kind) {
        case C_DICTIONARY: return c_tableRemove(c,o,condition,arg);
        case C_SET:        return c_setRemove(c,o,condition,arg);
        case C_BAG:        return c_bagRemove(c,o,condition,arg);
        case C_LIST:       return c_listRemove(c,o,condition,arg);
        default:
            OS_REPORT_1(OS_ERROR,"Database Collection",0,
                        "c_remove: illegal collection kind (%d) specified",
                        c_collectionType(type)->kind);
            assert(FALSE);
        break;
    }
    return NULL;
}

c_object
c_replace (
    c_collection c,
    c_object o,
    c_replaceCondition condition,
    c_voidp arg)
{
    c_type type = c__getType(c);

    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_replace: given entity (%d) is not a collection",
                    c_collectionType(type)->kind);
        assert(FALSE);
        return NULL;
    }
    switch(c_collectionType(type)->kind) {
        case C_DICTIONARY: return c_tableReplace(c,o,condition,arg);
        case C_SET:        return c_setReplace(c,o,condition,arg);
        case C_BAG:        return c_bagReplace(c,o,condition,arg);
        case C_LIST:       return c_listReplace(c,o,condition,arg);
        default:
            OS_REPORT_1(OS_ERROR,"Database Collection",0,
                        "c_replace: illegal collection kind (%d) specified",
                        c_collectionType(type)->kind);
            assert(FALSE);
        break;
    }
    return NULL;
}

static c_bool checkEquality(c_object org, c_object rep, c_voidp arg)
{
    *((c_bool *)arg) = (org == rep);
    return FALSE;
}

c_bool
c_exists (
    c_collection c,
    c_object o)
{
    c_bool equal = FALSE;
    c_object found;

    found = c_find(c,o);
    if (found == o) {
        equal = TRUE;
    }
    c_free(found);
    return equal;
}

static c_bool
lookupAction(
    c_object org,
    c_object template,
    c_voidp arg)
{
    c_object *found = (c_object *)arg;

    *found = c_keep(org);
    return FALSE;
}

c_object
c_find(
    c_collection c,
    c_object templ)
{
    c_type type;
    c_object found;
    c_object replaced;

    type = c_typeActualType(c__getType(c));
    if (c_baseObject(type)->kind != M_COLLECTION) {
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_lookup: given collection (%d) is not a collection",
                    c_collectionType(type)->kind);
        assert(FALSE);
        return NULL;
    }
    found = NULL;
    replaced = c_remove(c, templ, lookupAction, (c_voidp)&found);
    assert((replaced == NULL) || (replaced == templ));

    return found;
}

c_object
c_read(
    c_collection c)
{
    c_type type = c__getType(c);

    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_read: given entity (%d) is not a collection",
                    c_collectionType(type)->kind);
        assert(FALSE);
        return NULL;
    }

    switch(c_collectionType(type)->kind) {
    case C_QUERY:      return c_queryReadOne(c_query(c),NULL);
    case C_DICTIONARY: return c_tableReadOne(c,NULL);
    case C_SET:        return c_setReadOne(c,NULL);
    case C_BAG:        return c_bagReadOne(c,NULL);
    case C_LIST:       return c_listReadOne(c,NULL);
    default:
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_read: illegal collection kind (%d) specified",
                    c_collectionType(type)->kind);
        assert(FALSE);
    break;
    }
    return NULL;
}

c_bool
c_readAction (
    c_collection c,
    c_action action,
    c_voidp arg)
{
    c_type type = c__getType(c);

    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_readAction: given entity (%d) is not a collection",
                    c_collectionType(type)->kind);
        assert(FALSE);
        return FALSE;
    }
    switch(c_collectionType(type)->kind) {
        case C_QUERY:      return c_queryRead(c_query(c),NULL,action,arg);
        case C_DICTIONARY: return c_tableRead(c,NULL,action,arg);
        case C_SET:        return c_setRead(c,NULL,action,arg);
        case C_BAG:        return c_bagRead(c,NULL,action,arg);
        case C_LIST:       return c_listRead(c,NULL,action,arg);
        default:
            OS_REPORT_1(OS_ERROR,"Database Collection",0,
                        "c_readAction: illegal collection kind (%d) specified",
                        c_collectionType(type)->kind);
            assert(FALSE);
        break;
    }
    return FALSE;
}

c_object
c_take(
    c_collection c)
{
    c_type type = c__getType(c);

    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_take: given entity (%d) is not a collection",
                    c_collectionType(type)->kind);
        assert(FALSE);
        return NULL;
    }
    switch(c_collectionType(type)->kind) {
        case C_QUERY:      return c_queryTakeOne(c_query(c),NULL);
        case C_DICTIONARY: return c_tableTakeOne(c,NULL);
        case C_SET:        return c_setTakeOne(c,NULL);
        case C_BAG:        return c_bagTakeOne(c,NULL);
        case C_LIST:       return c_listTakeOne(c,NULL);
        default:
            OS_REPORT_1(OS_ERROR,"Database Collection",0,
                        "c_take: illegal collection kind (%d) specified",
                        c_collectionType(type)->kind);
            assert(FALSE);
        break;
    }
    return NULL;
}

c_bool
c_takeAction (
    c_collection c,
    c_action action,
    c_voidp arg)
{
    c_type type = c__getType(c);

    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_takeAction: given entity (%d) is not a collection",
                    c_collectionType(type)->kind);
        assert(FALSE);
        return FALSE;
    }
    switch(c_collectionType(type)->kind) {
        case C_QUERY:      return c_queryTake(c_query(c),NULL,action,arg);
        case C_DICTIONARY: return c_tableTake(c,NULL,action,arg);
        case C_SET:        return c_setTake(c,NULL,action,arg);
        case C_BAG:        return c_bagTake(c,NULL,action,arg);
        case C_LIST:       return c_listTake(c,NULL,action,arg);
        default:
            OS_REPORT_1(OS_ERROR,"Database Collection",0,
                        "c_takeAction: illegal collection kind (%d) specified",
                        c_collectionType(type)->kind);
            assert(FALSE);
        break;
    }
    return FALSE;
}

c_iter
c_select (
    c_collection c,
    c_long max)
{
    c_type type = c__getType(c);

    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                    "c_execute: given entity (%d) is not a collection",
                    c_collectionType(type)->kind);
        assert(FALSE);
        return NULL;
    }

    switch(c_collectionType(type)->kind) {
        case C_QUERY:      return c_querySelect(c_query(c),NULL,max);
        case C_DICTIONARY: return c_tableSelect(c,NULL,max);
        case C_SET:        return c_setSelect(c,NULL,max);
        case C_BAG:        return c_bagSelect(c,NULL,max);
        case C_LIST:       return c_listSelect(c,NULL,max);
        default:
            OS_REPORT_1(OS_ERROR,"Database Collection",0,
                        "c_select: illegal collection kind (%d) specified",
                        c_collectionType(type)->kind);
            assert(FALSE);
        break;
    }
    return NULL;
}

static c_bool
countAction(
    c_object data,
    c_voidp arg)
{
    c_long *count = (c_long *)arg;

    if (data != NULL) {
        *count = *count + 1;
    }
    return TRUE;
}

c_long
c_queryCount(
    c_query _this)
{
    c_long count = 0;

    c_walk(((C_STRUCT(c_query) *)_this)->source,countAction,&count);
    return count;
}

c_long
c_count  (
    c_collection c)
{
    c_type type = c__getType(c);

    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        return 0;
    }

    switch(c_collectionType(type)->kind) {
        case C_QUERY:      return c_queryCount((c_query)c);
        case C_DICTIONARY: return c_tableCount(c);
        case C_SET:        return c_setCount(c);
        case C_BAG:        return c_bagCount(c);
        case C_LIST:       return c_listCount(c);
        default:
            OS_REPORT_1(OS_ERROR,"Database Collection",0,
                        "c_count: illegal collection kind (%d) specified",
                        c_collectionType(type)->kind);
            assert(FALSE);
        break;
    }
    return 0;
}

c_type
c_subType (
    c_collection c)
{
    c_type type = c__getType(c);

    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        return NULL;
    }

    return c_keep(c_collectionType(type)->subType);
}

c_bool
c_walk(
    c_collection c,
    c_action action,
    c_voidp actionArg)
{
    c_type type;

    if(c == NULL){
        return TRUE;
    }
    type = c__getType(c);

    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        return FALSE;
    }

    switch(c_collectionType(type)->kind) {
        case C_DICTIONARY: return c_tableWalk((c_table)c,action,actionArg);
        case C_SET:        return c_setWalk((c_set)c,action,actionArg);
        case C_BAG:        return c_bagWalk((c_bag)c,action,actionArg);
        case C_LIST:       return c_listWalk((c_list)c,action,actionArg);
        case C_QUERY:      return c_queryRead(c_query(c),NULL,action,actionArg);
        default:
            OS_REPORT_1(OS_ERROR,"Database Collection",0,
                        "c_walk: illegal collection kind (%d) specified",
                        c_collectionType(type)->kind);
            assert(FALSE);
        break;
    }
    return FALSE;
}

static c_bool
c_uniteAction(
    c_object o,
    c_voidp collection)
{
    c_insert(collection,o);
    return TRUE;
}

static c_bool
subTypeEqual(
    c_collection c1,
    c_collection c2)
{
    c_type t1 = c__getType(c1);
    c_type t2 = c__getType(c2);

    t1 = c_typeActualType(t1);
    t2 = c_typeActualType(t2);
    if (c_baseObject(t1)->kind != M_COLLECTION) {
        return FALSE;
    }
    if (c_baseObject(t2)->kind != M_COLLECTION) {
        return FALSE;
    }
    t1 = c_collectionType(t1)->subType;
    t2 = c_collectionType(t2)->subType;
    t1 = c_typeActualType(t1);
    t2 = c_typeActualType(t2);
    if (t1 != t2) {
        return FALSE;
    }
    return TRUE;
}

c_bool
c_unite(
    c_collection c1,
    c_collection c2)
{
    if (!subTypeEqual(c1,c2)) {
        return FALSE;
    }
    return c_walk(c2,c_uniteAction,c1);
}

static c_bool
c_differAction(
    c_object o,
    c_voidp collection)
{
    c_remove(collection,o,NULL,NULL);
    return TRUE;
}

c_bool
c_differ(
    c_collection c1,
    c_collection c2)
{
    if (!subTypeEqual(c1,c2)) {
        return FALSE;
    }
    return c_walk(c2,c_differAction,c1);
}

static c_bool
c_intersectAction(
    c_object o,
    c_voidp collection)
{
    if (o == NULL) {
        OS_REPORT(OS_WARNING,"c_intersect",0,
                             "c_intersectAction: parameter o == NULL");
    }
    if (collection == NULL) {
        OS_REPORT(OS_WARNING,"c_intersect",0,
                             "c_intersectAction: parameter collection == NULL");
    }
    OS_REPORT(OS_WARNING,"c_intersect",0,"Not yet implemented");
    return TRUE;
}

c_bool
c_intersect(
    c_collection c1,
    c_collection c2)
{
    if (!subTypeEqual(c1,c2)) {
        return FALSE;
    }
    return c_walk(c2,c_intersectAction,c1);
}

c_array
c_arrayNew(
    c_type subType,
    c_long length)
{
    c_base base;
    c_type arrayType;
    c_char *name;
    c_long str_size;
    c_array _this = NULL;

    if (length == 0) {
        /* NULL is specified to represent an array of length 0. */
        return NULL;
    } else if (length < 0) {
        OS_REPORT_1(OS_ERROR,
                    "Database Collection",0,
                    "Illegal array size %d specified",
                    length);
    } else if (c_metaObject(subType)->name != NULL) {
        base = c__getBase(subType);
        str_size =    strlen(c_metaObject(subType)->name)
                    + 7 /* ARRAY<> */
                    + 1; /* \0 character */
        name = (char *)os_alloca(str_size);
        os_sprintf(name,"ARRAY<%s>",c_metaObject(subType)->name);
        arrayType = c_metaArrayTypeNew(c_metaObject(base), name,subType,0);
        assert(arrayType);
        os_freea(name);
        _this = (c_array)c_newArray(c_collectionType(arrayType), length);
        c_free(arrayType);
    }

    return _this;
}

c_sequence
c_sequenceNew(
    c_type subType,
    c_long maxsize,
    c_long length)
{
    c_base base;
    c_type sequenceType;
    c_char *name;
    c_long str_size;
    c_sequence _this = NULL;

    if (length < 0) {
        OS_REPORT_1(OS_ERROR,
                    "Database Collection",0,
                    "Illegal sequence size %d specified",
                    length);
    } else if (c_metaObject(subType)->name != NULL) {
        base = c__getBase(subType);

        if(maxsize == 0){
            str_size =  strlen(c_metaObject(subType)->name)
                        + 10 /* SEQUENCE<> */
                        + 1; /* \0 character */
            name = (char *)os_alloca(str_size);
            os_sprintf(name,"SEQUENCE<%s>",c_metaObject(subType)->name);
        } else {
            str_size =  strlen(c_metaObject(subType)->name)
                        + 11 /* SEQUENCE<,> */
                        + ((int)(log10((double)maxsize))) + 1 /* digits for maxsize */
                        + 1; /* \0 character */
            name = (char *)os_alloca(str_size);
            os_sprintf(name,"SEQUENCE<%s,%d>",c_metaObject(subType)->name, maxsize);
        }
        sequenceType = c_metaSequenceTypeNew(c_metaObject(base), name, subType,
                maxsize);
        assert(sequenceType);
        os_freea(name);
        _this = (c_sequence)c_newSequence(c_collectionType(sequenceType), length);
        c_free(sequenceType);
    }

    return _this;
}

/**
 * Always create new array regardless of length. Functionality is only required in serializer context
 */
c_array
c_arrayNew_w_header(
    c_collectionType arrayType,
    c_long length)
{
    c_array _this = NULL;

    if (length == 0)
    {
        _this = c_newArray (arrayType, length);
    }else {
        _this = c_arrayNew (arrayType->subType, length);
    }

    return _this;
}

#define C_LIST_ANONYMOUS_NAME "LIST<******>"
c_collection
c_listNew(
    c_type subType)
{
    c_metaObject o;
    c_metaObject found;
    c_collection c;
    c_base base;
    char *name;
    c_long size;

    base = c__getBase(subType);
    if (c_metaObject(subType)->name != NULL) {
        size = strlen(c_metaObject(subType)->name)+7;
        name = (char *)os_alloca(size);
        os_sprintf(name,"LIST<%s>",c_metaObject(subType)->name);
        found = c_metaResolve(c_metaObject(base), name);
    } else {
        name = (char *)os_alloca(strlen(C_LIST_ANONYMOUS_NAME) + 1);
        os_strcpy(name, C_LIST_ANONYMOUS_NAME);
        found = NULL;
    }

    if (found == NULL) {
        o = c_metaDefine(c_metaObject(base),M_COLLECTION);
        /*c_metaObject(o)->name = name;*/
        c_metaObject(o)->name = NULL;
        c_collectionType(o)->kind = C_LIST;
        c_collectionType(o)->subType = c_keep(subType);
        c_collectionType(o)->maxSize = C_UNLIMITED;
        c_metaFinalize(o);
        if (strcmp(name, C_LIST_ANONYMOUS_NAME) != 0) { /* only bind if not anonymous! */
            found = c_metaBind(c_metaObject(base), name, o);
            assert(found != NULL);
            c_free(o); /* always free o, since when it is inserted refcount is increased by 2! */
            if (found != o) {
                 o = found;
            }
        }
    } else {
        o = found;
        found = NULL;
    }
    os_freea(name);
    c = (c_collection)c_new(c_type(o));
    if (c) {
        c_list(c)->cache = c_mmCacheCreate(c_baseMM(base),
                                           C_SIZEOF(c_listNode),
                                           _PREALLOC_);
    }
    c_free(o);
    return c;
}
#undef C_LIST_ANONYMOUS_NAME

#define C_SET_ANONYMOUS_NAME "SET<******>"
c_collection
c_setNew(
    c_type subType)
{
    c_metaObject o;
    c_metaObject found;
    c_collection c;
    c_base base;
    char *name;
    c_long size;

    base = c__getBase(subType);
    if (c_metaObject(subType)->name != NULL) {
        size = strlen(c_metaObject(subType)->name)+6;
        name = (char *)os_alloca(size);
        os_sprintf(name,"SET<%s>",c_metaObject(subType)->name);
        found = c_metaResolve(c_metaObject(base), name);
    } else {
        name = (char *)os_alloca(strlen(C_SET_ANONYMOUS_NAME) + 1);
        os_strcpy(name, C_SET_ANONYMOUS_NAME);
        found = NULL;
    }

    if (found == NULL) {
        o = c_metaDefine(c_metaObject(base),M_COLLECTION);
        /*c_metaObject(o)->name = name;*/
        c_metaObject(o)->name = NULL;
        c_collectionType(o)->kind = C_SET;
        c_collectionType(o)->subType = c_keep(subType);
        c_collectionType(o)->maxSize = C_UNLIMITED;
        c_metaFinalize(o);
        if (strcmp(name, C_SET_ANONYMOUS_NAME) != 0) { /* only bind if not anonymous! */
            found = c_metaBind(c_metaObject(base), name, o);
            assert(found != NULL);
            c_free(o); /* always free o, since when it is inserted refcount is increased by 2! */
            if (found != o) {
                 o = found;
            }
        }
    } else {
        o = found;
        found = NULL;
    }
    os_freea(name);
    c = (c_collection)c_new(c_type(o));
    if (c) {
        c_set(c)->cache = c_mmCacheCreate(c_baseMM(base),
                                          C_SIZEOF(c_setNode),
                                          _PREALLOC_);
        c_avlTree(c)->root = NULL;
        c_avlTree(c)->offset = 0;
        c_avlTree(c)->size = 0;
        c_avlTree(c)->mm = NULL;
    }
    c_free(o);
    return c;
}
#undef C_SET_ANONYMOUS_NAME

#define C_BAG_ANONYMOUS_NAME "BAG<******>"
c_collection
c_bagNew(
    c_type subType)
{
    c_metaObject o;
    c_metaObject found;
    c_collection c;
    c_base base;
    c_string name;
    c_long size;

    base = c__getBase(subType);
    if (c_metaObject(subType)->name != NULL) {
        size = strlen(c_metaObject(subType)->name)+6;
        name = (char *)os_alloca(size);
        os_sprintf(name,"BAG<%s>",c_metaObject(subType)->name);
        found = c_metaResolve(c_metaObject(base), name);
    } else {
        name = (char *)os_alloca(strlen(C_BAG_ANONYMOUS_NAME) + 1);
        os_strcpy(name, C_BAG_ANONYMOUS_NAME);
        found = NULL;
    }

    if (found == NULL) {
        o = c_metaDefine(c_metaObject(base),M_COLLECTION);
        /*c_metaObject(o)->name = name;*/
        c_metaObject(o)->name = NULL;
        c_collectionType(o)->kind = C_BAG;
        c_collectionType(o)->subType = c_keep(subType);
        c_collectionType(o)->maxSize = C_UNLIMITED;
        c_metaFinalize(o);
        if (strcmp(name, C_BAG_ANONYMOUS_NAME) != 0) { /* only bind if not anonymous! */
            found = c_metaBind(c_metaObject(base), name, o);
            assert(found != NULL);
            c_free(o); /* always free o, since when it is inserted refcount is increased by 2! */
            if (found != o) {
                 o = found;
            }
        }
    } else {
        o = found;
        found = NULL;
    }
    os_freea(name);
    c = (c_collection)c_new(c_type(o));
    if (c) {
        c_bag(c)->cache = c_mmCacheCreate(c_baseMM(base),
                                          C_SIZEOF(c_bagNode),
                                          _PREALLOC_);
        c_avlTree(c)->root = NULL;
        c_avlTree(c)->offset = 0;
        c_avlTree(c)->size = 0;
        c_avlTree(c)->mm = NULL;
        c_bag(c)->count = 0;
    }
    c_free(o);
    return c;
}
#undef C_BAG_ANONYMOUS_NAME

#define C_TABLE_ANONYMOUS_NAME "MAP<******>"
c_collection
c_tableNew(
    c_type subType,
    const c_char *keyNames)
{
    c_base base;
    c_iter keyNameList, fieldList;
    c_string keyName;
    c_field field;
    c_bool error;
    c_long i,nrOfKeys;
    c_metaObject o;
    c_metaObject found;
    C_STRUCT(c_table) *t;
    char *name;
    c_long size;

    base = c__getBase(subType);
    if (c_metaObject(subType)->name != NULL) {
        if (keyNames) {
            size = strlen(c_metaObject(subType)->name)+strlen(keyNames)+7;
            name = (char *)os_alloca(size);
            os_sprintf(name,"MAP<%s,%s>",c_metaObject(subType)->name, keyNames);
        } else {
            size = strlen(c_metaObject(subType)->name)+6;
            name = (char *)os_alloca(size);
            os_sprintf(name,"MAP<%s>",c_metaObject(subType)->name);
        }
        found = c_metaResolve(c_metaObject(base), name);
    } else {
        name = (char *)os_alloca(strlen(C_TABLE_ANONYMOUS_NAME)+1);
        os_strcpy(name, C_TABLE_ANONYMOUS_NAME);
        found = NULL;
    }

    error = FALSE;
    fieldList = c_iterNew(NULL);
    if (keyNames != NULL) {
        keyNameList = c_splitString(keyNames,", \t");
        while ((keyName = c_iterTakeFirst(keyNameList)) != NULL) {
            if (c_iterResolve(keyNameList,c_compareString,keyName) == NULL) {
                field = c_fieldNew(subType,keyName);
                if (field == NULL) {
                    if (c_metaObject(subType)->name == NULL) {
                        OS_REPORT_1(OS_ERROR,"Database Collection",0,
                                    "c_tableNew: field %s not found in type",keyName);
                    } else {
                        OS_REPORT_2(OS_ERROR,"Database Collection",0,
                                    "c_tableNew: field %s not found in type %s",
                                    keyName, c_metaObject(subType)->name);
                    }
                    error = TRUE;
                }
                c_iterAppend(fieldList,field);
            }
            os_free(keyName);
        }
        c_iterFree(keyNameList);
    }
    if (error) {
        field = c_iterTakeFirst(fieldList);
        while (field != NULL) {
            c_free(field);
            field = c_iterTakeFirst(fieldList);
        }
        c_iterFree(fieldList);
        os_freea(name);
        return NULL;
    }

    if (found == NULL) {
        o = c_metaDefine(c_metaObject(base),M_COLLECTION);
        c_metaObject(o)->name = NULL;
        c_collectionType(o)->kind = C_DICTIONARY;
        c_collectionType(o)->subType = c_keep(subType);
        c_collectionType(o)->maxSize = C_UNLIMITED;
        c_metaFinalize(o);

        if (strcmp(name, C_TABLE_ANONYMOUS_NAME) != 0) { /* only bind if not anonymous! */
            found = c_metaBind(c_metaObject(base), name, o);
            assert(found != NULL);
            c_free(o); /* always free o, since when it is inserted refcount is increased by 2! */
            if (found != o) {
                 o = found;
            }
        }
    } else {
        o = found;
        found = NULL;
    }
    os_freea(name);

    t = c_table(c_new(c_type(o)));
    c_free(o);
    if (t) {
        t->count = 0;
        nrOfKeys = c_iterLength(fieldList);
        if (nrOfKeys>0) {
            t->key = c_arrayNew(c_resolve(base,"c_field"),nrOfKeys);
            for (i=0;i<nrOfKeys;i++) {
                t->key[i] = c_iterTakeFirst(fieldList);
            }
        } else {
            t->key = NULL;
        }
        c_iterFree(fieldList);
        t->cache = c_mmCacheCreate(c_baseMM(base),
                                   C_SIZEOF(c_tableNode),
                                   _PREALLOC_);
        t->object = NULL;
    }
    return (c_collection)t;
}
#undef C_TABLE_ANONYMOUS_NAME

#if 0
c_collection
c_queryNew(
    c_collection c,
    q_expr predicate,
    c_value params[])
{
    c_base base;
    c_type subType;
    c_qPred pred;
    C_STRUCT(c_query) *q;
    c_metaObject o;
    c_string name;
    c_long size;
    c_qResult result;

    base = c__getBase(c);

    subType = c_collectionType(c__getType(c))->subType;
    result = c_qPredNew(subType,c_keyList(c),predicate,params,&pred);
    if (result == CQ_RESULT_OK) {
        if (pred == NULL) {
            return NULL;
        }

        if (c_metaObject(subType)->name != NULL) {
            size = strlen(c_metaObject(subType)->name)+8;
            name = c_stringMalloc(base,size);
            os_sprintf(name,"QUERY<%s>",c_metaObject(subType)->name);
        } else {
            name = c_stringNew(base,"QUERY<******>");
        }

        o = c_metaDefine(c_metaObject(base),M_COLLECTION);
        c_metaObject(o)->name = name;
        c_collectionType(o)->kind = C_QUERY;
        c_collectionType(o)->subType = c_keep(subType);
        c_collectionType(o)->maxSize = C_UNLIMITED;
        c_metaFinalize(o);

        q = c_query(c_new(c_type(o)));
        c_free(o);
        if (q) {
            q->source = c;
            q->pred = pred;

            c_qPredOptimize(q->pred);
        }
    } else {
        q = NULL;
    }

    return (c_collection)q;
}
#else
#define C_QUERY_ANONYMOUS_NAME "QUERY<******>"
c_collection
c_queryNew(
    c_collection c,
    q_expr predicate,
    c_value params[])
{
    c_base base;
    c_type subType;
    c_qPred pred;
    C_STRUCT(c_query) *q;
    c_metaObject o;
    c_string name;
    c_long size;
    c_qResult result;
    c_metaObject found;

    base = c__getBase(c);
    subType = c_collectionType(c__getType(c))->subType;
    result = c_qPredNew(subType,c_keyList(c),predicate,params,&pred);
    if (result == CQ_RESULT_OK) {
        if (pred == NULL) {
            return NULL;
        }

        if (c_metaObject(subType)->name != NULL) {
            size = strlen(c_metaObject(subType)->name)+8;
            name = (char *)os_alloca(size);
            os_sprintf(name,"QUERY<%s>",c_metaObject(subType)->name);
            found = c_metaResolve(c_metaObject(base), name);
        } else {
            name = (char *)os_alloca(strlen(C_QUERY_ANONYMOUS_NAME) + 1);
            os_strcpy(name, C_QUERY_ANONYMOUS_NAME);
            found = NULL;
        }

        if (found == NULL) {
            o = c_metaDefine(c_metaObject(base),M_COLLECTION);
            c_metaObject(o)->name = NULL;
            c_collectionType(o)->kind = C_QUERY;
            c_collectionType(o)->subType = c_keep(subType);
            c_collectionType(o)->maxSize = C_UNLIMITED;
            c_metaFinalize(o);
            if (strcmp(name, C_QUERY_ANONYMOUS_NAME) != 0) { /* only bind if not anonymous! */
                found = c_metaBind(c_metaObject(base), name, o);
                assert(found != NULL);
                c_free(o); /* always free o, since when it is inserted refcount is increased by 2! */
                if (found != o) {
                     o = found;
                }
            }
        } else {
            o = found;
            found = NULL;
        }
        os_freea(name);
        q = c_query(c_new(c_type(o)));
        c_free(o);
        if (q) {
            q->source = c;
            q->pred = pred;
        }
    } else {
        q = NULL;
    }
    return (c_collection)q;
}
#undef C_QUERY_ANONYMOUS_NAME
#endif

void
c_clear (
    c_collection c)
{
    c_type type;
    c_object o;
    C_STRUCT(c_query) *q;

    if (c == NULL) return;
    type = c__getType(c);
    type = c_typeActualType(type);
    assert(type != NULL);
    assert(c_baseObject(type)->kind == M_COLLECTION);

    switch (c_collectionType(type)->kind) {
    case C_DICTIONARY:
        while ((o = c_take(c)) != NULL) {
            c_free(o);
        }
        c_free(c_table(c)->key);
        if (c_table(c)->object) {
            c_avlTreeFree(c_table(c)->object);
        }
        c_mmCacheDestroy(c_table(c)->cache);
    break;
    case C_SET:
        while ((o = c_take(c)) != NULL) {
            c_free(o);
        }
        c_mmCacheDestroy(c_set(c)->cache);
    break;
    case C_BAG:
        while ((o = c_take(c)) != NULL) {
            c_free(o);
        }
        c_mmCacheDestroy(c_bag(c)->cache);
    break;
    case C_LIST:
        while ((o = c_take(c)) != NULL) {
            c_free(o);
        }
        c_mmCacheDestroy(c_list(c)->cache);
    break;
    case C_QUERY:
        q = c_query(c);
        c_free(q->pred);
    break;
    case C_STRING:
    break;
    case C_SCOPE:
        c_scopeClean((c_scope)c);
    break;
    default:
        OS_REPORT_1(OS_ERROR,
                    "Database Collection",0,
                    "c_walk: illegal collection kind (%d) specified",
                    c_collectionType(type)->kind);
        assert(FALSE);
    break;
    }
}

c_bool
c_querySetParams (
    c_collection _this,
    c_value params[])
{
    c_type type;
    c_bool result = TRUE;

    if (_this != NULL) {
        type = c__getType(_this);
        type = c_typeActualType(type);
        if (c_baseObject(type)->kind != M_COLLECTION) {
            OS_REPORT(OS_ERROR,
                      "Database Collection",0,
                      "c_querySetParams: malformed query specified");
            result = FALSE;
        } else if (c_collectionType(type)->kind != C_QUERY) {
            OS_REPORT_1(OS_ERROR,
                        "Database Collection",0,
                        "c_querySetParams: illegal collection kind (%d) specified",
                        c_collectionType(type)->kind);
            result = FALSE;
        } else {
            result = c_qPredSetArguments(c_query(_this)->pred,params);
        }
    }
    return result;
}

c_bool
c_queryEval(
    c_collection _this,
    c_object o)
{
    c_qPred pred;

    pred = c_query(_this)->pred;
    while (pred != NULL) {
        if (c_qPredEval(pred,o)) {
            return TRUE;
        }
        pred = pred->next;
    }
    return FALSE;
}

c_array
c_keyList(
    c_collection o)
{
    c_type type;

    if (o == NULL) {
        return NULL;
    }
    type = c__getType(o);
    type = c_typeActualType(type);
    if (c_baseObject(type)->kind != M_COLLECTION) {
        return NULL;
    }
    if (c_collectionType(type)->kind != C_DICTIONARY) {
        return NULL;
    }
    return (c_table(o))->key;
}

void
c_collectionInit (
    c_base base)
{
    c_collectionType o;
    c_metaObject scope = c_metaObject(base);

#define INITCOLL(s,n,k,t) \
    o = (c_collectionType)c_metaDeclare(s,#n,M_COLLECTION); \
    o->kind = k; \
    o->subType = (c_type)c_metaResolve(s,#t); \
    o->maxSize = C_UNLIMITED; \
    c_metaFinalize(c_metaObject(o)); \
    c_free(o)

    INITCOLL(scope,c_string, C_STRING,    c_char);
    INITCOLL(scope,c_wstring,C_WSTRING,   c_wchar);
    INITCOLL(scope,c_list,   C_LIST,      c_object);
    INITCOLL(scope,c_set,    C_SET,       c_object);
    INITCOLL(scope,c_bag,    C_BAG,       c_object);
    INITCOLL(scope,c_table,  C_DICTIONARY,c_object);
    INITCOLL(scope,c_array,  C_ARRAY,     c_object);
    INITCOLL(scope,c_query,  C_QUERY,     c_object);

#undef INITCOLL
}
