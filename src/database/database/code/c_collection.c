/* -*- mode: c; c-file-style: "k&r"; c-basic-offset: 4; -*- */

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
#include <math.h>
#include <stddef.h>
#include "os.h"
#include "os_report.h"
#include "ut_avl.h"
#include "q_expr.h"
#include "c__base.h"
#include "c__scope.h"
#include "c__collection.h"
#include "c_misc.h"
#include "c_metabase.h"
#include "c__querybase.h"
#include "c_metafactory.h"
#include "c_stringSupport.h"
#include "c_mmbase.h"
#include "c__mmbase.h"

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
    c_mm mm;
};

C_STRUCT(c_set) {
    ut_avlCTree_t tree;
    c_mm mm;
};

C_STRUCT(c_bag) {
    ut_avlTree_t tree;
    c_ulong count;
    c_mm mm;
};

union c_tableContents {
    c_object object;
    ut_avlTree_t tree;
};

C_STRUCT(c_table) {
    union c_tableContents contents;
    c_array cursor;
    c_array key;
    c_ulong count;
    c_mm mm;
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

static int ptrCompare(const void *a, const void *b)
{
    return (a == b) ? 0 : (a < b) ? -1 : 1;
}

static c_bool predMatches (c_qPred qp, c_object o)
{
    if (qp == NULL) {
        return TRUE;
    } else {
        do {
            if (c_qPredEval (qp, o)) {
                return TRUE;
            }
            qp = qp->next;
        } while (qp);
        return FALSE;
    }
}

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

#ifndef NDEBUG

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

#endif

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

    n = c_mmMalloc (l->mm, C_SIZEOF (c_listNode));
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
    c_mmFree (l->mm,n);
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
    c_mmFree(l->mm,n);
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

    n = c_mmMalloc (l->mm, C_SIZEOF (c_listNode));
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
    p = c_mmMalloc (l->mm, C_SIZEOF (c_listNode));
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
    p = c_mmMalloc (l->mm, C_SIZEOF (c_listNode));
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
    c_mmFree(l->mm,n);
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
    ut_avlNode_t avlnode;
    c_object object;
};

static const ut_avlCTreedef_t c_set_td =
    UT_AVL_CTREEDEF_INITIALIZER_INDKEY (
        offsetof (C_STRUCT(c_setNode), avlnode), offsetof (C_STRUCT(c_setNode), object),
        ptrCompare, 0);

c_object
c_setInsert (
    c_set _this,
    c_object o)
{
    C_STRUCT(c_set) *set = (C_STRUCT(c_set) *) _this;
    ut_avlIPath_t p;
    c_setNode f;
    assert(c_collectionIsType(_this, C_SET));
    if ((f = ut_avlCLookupIPath (&c_set_td, &set->tree, o, &p)) == NULL) {
        f = c_mmMalloc (set->mm, sizeof (*f));
        f->object = c_keep (o);
        ut_avlCInsertIPath (&c_set_td, &set->tree, f, &p);
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
    C_STRUCT(c_set) *set = (C_STRUCT(c_set) *) _this;
    ut_avlIPath_t p;
    c_setNode f;
    assert(c_collectionIsType(_this, C_SET));
    c_keep (o);
    if ((f = ut_avlCLookupIPath (&c_set_td, &set->tree, o, &p)) == NULL) {
        f = c_mmMalloc (set->mm, sizeof (*f));
        f->object = o;
        ut_avlCInsertIPath (&c_set_td, &set->tree, f, &p);
        return NULL;
    } else if (condition == 0 || condition (f->object, o, arg)) {
        c_object old = f->object;
        f->object = o;
        return old;
    } else {
        return o;
    }
}

c_object
c_setRemove (
    c_set _this,
    c_object o,
    c_removeCondition condition,
    c_voidp arg)
{
    C_STRUCT(c_set) *set = (C_STRUCT(c_set) *) _this;
    ut_avlDPath_t p;
    c_setNode f;
    assert(c_collectionIsType(_this, C_SET));
    if ((f = ut_avlCLookupDPath (&c_set_td, &set->tree, o, &p)) == NULL) {
        return NULL;
    } else if (condition && !condition (f->object, o, arg)) {
        return NULL;
    } else {
        c_object old = f->object;
        ut_avlCDeleteDPath (&c_set_td, &set->tree, f, &p);
        c_mmFree (set->mm, f);
        return old;
    }
}

static c_bool
c_setRead(
    c_set _this,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    C_STRUCT(c_set) *set = (C_STRUCT(c_set) *) _this;
    ut_avlCIter_t it;
    c_setNode n;
    c_bool proceed;
    assert(c_collectionIsType(_this, C_SET));
    proceed = TRUE;
    for (n = ut_avlCIterFirst (&c_set_td, &set->tree, &it);
         n != NULL && proceed;
         n = ut_avlCIterNext (&it)) {
        if (predMatches (q, n->object)) {
            proceed = action (n->object, arg);
        }
    }
    return proceed;
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
    C_STRUCT(c_set) *set = (C_STRUCT(c_set) *) _this;
    assert(c_collectionIsType(_this, C_SET));
    return ut_avlCCount(&set->tree);
}

c_bool
c_setWalk(
    c_set _this,
    c_action action,
    c_voidp actionArg)
{
    C_STRUCT(c_set) *set = (C_STRUCT(c_set) *) _this;
    ut_avlCIter_t it;
    c_setNode n;
    c_bool proceed;
    assert(c_collectionIsType(_this, C_SET));
    proceed = TRUE;
    for (n = ut_avlCIterFirst (&c_set_td, &set->tree, &it);
         n != NULL && proceed;
         n = ut_avlCIterNext (&it)) {
        proceed = action (n->object, actionArg);
    }
    return proceed;
}

/*============================================================================*/
/* BAG COLLECTION TYPE                                                        */
/*============================================================================*/

C_STRUCT(c_bagNode) {
    ut_avlNode_t avlnode;
    c_object object;
    c_long count;
};

static const ut_avlTreedef_t c_bag_td =
    UT_AVL_TREEDEF_INITIALIZER_INDKEY (
        offsetof (C_STRUCT(c_bagNode), avlnode), offsetof (C_STRUCT(c_bagNode), object),
        ptrCompare, 0);

c_object
c_bagInsert (
    c_bag _this,
    c_object o)
{
    C_STRUCT(c_bag) *bag = (C_STRUCT(c_bag) *) _this;
    ut_avlIPath_t p;
    c_bagNode f;
    assert(c_collectionIsType(_this, C_BAG));
    if ((f = ut_avlLookupIPath (&c_bag_td, &bag->tree, o, &p)) == NULL) {
        f = c_mmMalloc (bag->mm, sizeof (*f));
        f->object = c_keep (o);
        f->count = 1;
        ut_avlInsertIPath (&c_bag_td, &bag->tree, f, &p);
    } else {
        f->count++;
    }
    bag->count++;
    return f->object;
}

static c_object
c_bagReplace (
    c_bag _this,
    c_object o,
    c_bool (*condition)(),
    c_voidp arg)
{
    C_STRUCT(c_bag) *bag = (C_STRUCT(c_bag) *) _this;
    ut_avlIPath_t p;
    c_bagNode f;
    assert(c_collectionIsType(_this, C_BAG));
    c_keep (o);
    if ((f = ut_avlLookupIPath (&c_bag_td, &bag->tree, o, &p)) == NULL) {
        f = c_mmMalloc (bag->mm, sizeof (*f));
        f->object = o;
        f->count = 1;
        ut_avlInsertIPath (&c_bag_td, &bag->tree, f, &p);
        bag->count++;
        return NULL;
    } else if (condition == 0 || condition (f->object, o, arg)) {
        c_object old = f->object;
        f->object = o;
        return old;
    } else {
        return o;
    }
}

c_object
c_bagRemove (
    c_bag _this,
    c_object o,
    c_removeCondition condition,
    c_voidp arg)
{
    C_STRUCT(c_bag) *bag = (C_STRUCT(c_bag) *) _this;
    ut_avlDPath_t p;
    c_bagNode f;
    assert(c_collectionIsType(_this, C_BAG));
    if ((f = ut_avlLookupDPath (&c_bag_td, &bag->tree, o, &p)) == NULL) {
        return NULL;
    } else if (condition && !condition (f->object, o, arg)) {
        return NULL;
    } else {
        c_object obj = f->object;
        if (--f->count == 0) {
            ut_avlDeleteDPath (&c_bag_td, &bag->tree, f, &p);
            c_mmFree (bag->mm, f);
        }
        bag->count--;
        return obj;
    }
}

static c_bool
c_bagRead(
    c_bag _this,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    C_STRUCT(c_bag) *bag = (C_STRUCT(c_bag) *) _this;
    ut_avlIter_t it;
    c_bagNode n;
    c_bool proceed;
    assert(c_collectionIsType(_this, C_BAG));
    proceed = TRUE;
    for (n = ut_avlIterFirst (&c_bag_td, &bag->tree, &it);
         n != NULL && proceed;
         n = ut_avlIterNext (&it)) {
        if (predMatches (q, n->object)) {
            c_long i;
            for (i = 0; i < n->count && proceed; i++) {
                proceed = action (n->object, arg);
            }
        }
    }
    return proceed;
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
    C_STRUCT(c_bag) *bag = (C_STRUCT(c_bag) *) _this;
    assert(c_collectionIsType(_this, C_BAG));
    return bag->count;
}

c_bool
c_bagWalk(
    c_bag _this,
    c_action action,
    c_voidp actionArg)
{
    C_STRUCT(c_bag) *bag = (C_STRUCT(c_bag) *) _this;
    ut_avlIter_t it;
    c_bagNode n;
    c_bool proceed;
    assert(c_collectionIsType(_this, C_BAG));
    proceed = TRUE;
    for (n = ut_avlIterFirst (&c_bag_td, &bag->tree, &it);
         n != NULL && proceed;
         n = ut_avlIterNext (&it)) {
        c_long i;
        for (i = 0; i < n->count && proceed; i++) {
            proceed = action (n->object, actionArg);
        }
    }
    return proceed;
}

/*============================================================================*/
/* TABLE COLLECTION TYPE                                                      */
/*============================================================================*/

C_STRUCT(c_tableNode) {
    ut_avlNode_t avlnode;
    c_value keyValue;
    union c_tableContents contents;
#ifdef _CONSISTENCY_CHECKING_
    c_voidp table;
#endif
};

struct tableCmp_constraints {
    char require_LT_lt_0[-1+2*((int) C_LT < 0)];
    char require_EQ_eq_0[-1+2*((int) C_EQ == 0)];
    char require_GT_gt_0[-1+2*((int) C_GT > 0)];
    char non_empty_dummy_last_member[1];
};

static int tableCmp (const void *va, const void *vb)
{
    const c_value *a = va;
    const c_value *b = vb;
    return (int) c_valueCompare (*a, *b);
}

static const ut_avlTreedef_t c_table_td =
    UT_AVL_TREEDEF_INITIALIZER (
        offsetof (C_STRUCT(c_tableNode), avlnode), offsetof (C_STRUCT(c_tableNode), keyValue),
        tableCmp, 0);

static union c_tableContents *c_tableLookupInsert (C_STRUCT(c_table) *table, c_object o)
{
    union c_tableContents *index;
    c_long i, nrOfKeys;
    nrOfKeys = c_arraySize (table->key);
    index = &table->contents;
    for (i = 0; i < nrOfKeys; i++) {
        c_value k = c_fieldValue (table->key[i], o);
        ut_avlIPath_t p;
        c_tableNode f;
        if ((f = ut_avlLookupIPath (&c_table_td, &index->tree, &k, &p)) == NULL) {
            f = c_mmMalloc (table->mm, sizeof (*f));
            f->keyValue = k;
            if (i < nrOfKeys-1) {
                ut_avlInit (&c_table_td, &f->contents.tree);
            } else {
                f->contents.object = NULL;
            }
#ifdef _CONSISTENCY_CHECKING_
            f->table = table;
#endif
            ut_avlInsertIPath (&c_table_td, &index->tree, f, &p);
        } else {
            c_valueFreeRef (k);
            _CHECK_CONSISTENCY_ (table, f);
        }
        index = &f->contents;
    }
    return index;
}

c_object
c_tableInsert (
    c_table _this,
    c_object o)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    union c_tableContents *index;
    assert(c_collectionIsType(_this, C_DICTIONARY));

    if (o == NULL) {
        return NULL;
    }

    _ACCESS_BEGIN_(table);

    index = c_tableLookupInsert (table, o);
    if (index->object == NULL) {
        table->count++;
        index->object = c_keep (o);
    }

    _ACCESS_END_(table);
    return index->object;
}

static c_object
c_tableReplace (
    c_table _this,
    c_object o,
    c_bool (*condition)(),
    c_voidp arg)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    union c_tableContents *index;
    c_object retobj;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    if (o == NULL) {
        return NULL;
    }

    _ACCESS_BEGIN_(table);

    index = c_tableLookupInsert (table, o);
    if (index->object == NULL) {
        table->count++;
        index->object = c_keep (o);
        retobj = NULL;
    } else if (condition == 0 || condition (index->object, o, arg)) {
        retobj = index->object;
        index->object = c_keep (o);
    } else {
        retobj = o;
    }

    _ACCESS_END_(table);
    return retobj;
}

c_object
c_tableRemove (
    c_table _this,
    c_object o,
    c_removeCondition condition,
    c_voidp arg)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    c_object object = NULL;
    c_long nrOfKeys;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _ACCESS_BEGIN_(table);

    if (table->key == NULL || (nrOfKeys = c_arraySize (table->key)) == 0) {
        if (table->contents.object != NULL) {
            if (condition == 0 || condition (table->contents.object, o, arg)) {
                table->count--;
                object = table->contents.object;
                table->contents.object = NULL;
                table->cursor[0] = NULL;
            }
        }
    } else {
        struct {
            c_tableNode node;
            ut_avlDPath_t path;
        } *stk;
        C_STRUCT(c_tableNode) root_index;
        c_long i;

        stk = os_alloca ((nrOfKeys+1) * sizeof (*stk));

        root_index.contents = table->contents;
        stk[0].node = &root_index;
        for (i = 0; i < nrOfKeys; i++) {
            c_value k = c_fieldValue (table->key[i], o);
            stk[i+1].node = ut_avlLookupDPath (&c_table_td, &stk[i].node->contents.tree, &k, &stk[i+1].path);
            c_valueFreeRef (k);
            if (stk[i+1].node == NULL) {
                goto done;
            }
            _CHECK_CONSISTENCY_ (table, stk[i+1].node);
        }

        if (condition == 0 || condition (stk[i].node->contents.object, o, arg)) {
            object = stk[i].node->contents.object;
            table->count--;
            /* delete object */
            if (table->cursor[i-1] == stk[i].node) {
                table->cursor[i-1] = ut_avlFindPred (&c_table_td, &stk[i-1].node->contents.tree, stk[i].node);
            }
            ut_avlDeleteDPath (&c_table_td, &stk[i-1].node->contents.tree, stk[i].node, &stk[i].path);
            c_valueFreeRef (stk[i].node->keyValue);
            c_mmFree (table->mm, stk[i].node);
            /* prune empty trees */
            for (--i; i > 0 && ut_avlIsEmpty (&stk[i].node->contents.tree); i--) {
                /* FIXME: changed the condition here (from != NULL to
                 * == stk[i].node): the tree at level-1 isn't
                 * guaranteed empty, right? */
                if (table->cursor[i-1] == stk[i].node) {
                    assert (table->cursor[i] == NULL);
                    table->cursor[i-1] = ut_avlFindPred (&c_table_td, &stk[i-1].node->contents.tree, stk[i].node);
                }
                ut_avlDeleteDPath (&c_table_td, &stk[i-1].node->contents.tree, stk[i].node, &stk[i].path);
                c_valueFreeRef (stk[i].node->keyValue);
                c_mmFree (table->mm, stk[i].node);
            }
            table->contents = root_index.contents;
        }
    done:
        os_freea (stk);
    }
    _ACCESS_END_ (table);
    return object;
}

#ifdef _NIL_
c_object
c_tableFind (
    c_table _this,
    c_value *keyValues)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *)_this;
    union c_tableContents *contents;
    c_tableNode found;
    c_object object;
    c_long i, nrOfKeys;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _READ_BEGIN_(_this);

    if (table->key == NULL || (nrOfKeys = c_arraySize(table->key)) == 0) {
        if (table->contents.object == NULL) {
            goto notfound;
        }
        object = table->contents.object;
    } else {
        contents = &table->contents;
        for (i = 0; i < nrOfKeys; i++) {
            if ((found = ut_avlLookup (&c_table_td, &contents->tree, &keyValues[i])) == NULL) {
                goto notfound;
            } else {
                contents = &found->contents;
            }
        }
        object = contents->object;
    }
    _READ_END_(_this);
    return c_keep (object);
notfound:
    _READ_END_ (table);
    return NULL;
}
#endif

static c_tableNode
tableNext(
    c_object o,
    ut_avlTree_t *index,
    c_array keyList,
    c_long keyId)
{
    c_tableNode found;
    c_long lastKey = c_arraySize(keyList)-1;
    c_value k = c_fieldValue(keyList[keyId], o);

    if (keyId < lastKey) {
        found = ut_avlLookup (&c_table_td, index, &k);
        if (found != NULL) {
            found = tableNext (o, &found->contents.tree, keyList, keyId+1);
        }
        if (found == NULL) {
            found = ut_avlLookupSucc (&c_table_td, index, &k);
            if (found == NULL) {
                return NULL;
            }
            while (keyId != lastKey) {
                found = ut_avlFindMin (&c_table_td, &found->contents.tree);
                keyId++;
            }
        }
    } else {
        found = ut_avlLookupSucc (&c_table_td, index, &k);
    }

    c_valueFreeRef (k);
    return found;
}

static c_tableNode
tableFastNext(
    ut_avlTree_t *index,
    c_array cursorList,
    const c_long lastCursor,
    c_long cursor)
{
    if (cursor < lastCursor) {
        c_tableNode n;

        if (cursorList[cursor] == NULL) {
            cursorList[cursor] = ut_avlFindMin (&c_table_td, index);
        }

        if ((n = tableFastNext(&((c_tableNode) cursorList[cursor])->contents.tree, cursorList, lastCursor, cursor + 1)) != NULL)
            return n;
        else if ((cursorList[cursor] = ut_avlFindSucc(&c_table_td, index, cursorList[cursor])) == NULL)
            return NULL;
        else
            return tableFastNext(&((c_tableNode) cursorList[cursor])->contents.tree, cursorList, lastCursor, cursor + 1);
    } else {
        if (cursorList[cursor] == NULL) {
            cursorList[cursor] = ut_avlFindMin (&c_table_td, index);
        } else {
            cursorList[cursor] = ut_avlFindSucc (&c_table_td, index, cursorList[cursor]);
        }
        return cursorList[cursor];
    }
}

c_object
c_tableReadCursor (
    c_table _this)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    c_tableNode node;
    c_long nrOfKeys;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _READ_BEGIN_(table);
    if (table->key == NULL || (nrOfKeys = c_arraySize (table->key)) == 0) {
        /* Special case when no key is defined; single place
         * buffer */
        _READ_END_(table);
        if (table->cursor[0] == NULL) {
            _CHECK_CONSISTENCY_(table,table->contents.object);
            table->cursor[0] = table->contents.object;
        } else {
            table->cursor[0] = NULL;
        }
        return table->cursor[0];
    } else if (ut_avlIsEmpty (&table->contents.tree)) {
        _READ_END_(table);
        return NULL;
    } else {
        node = tableFastNext (&table->contents.tree, table->cursor, nrOfKeys - 1, 0);
        _READ_END_(table);
        if (node == NULL) { /* o is last record of table */
            return NULL;
        } else {
            return node->contents.object;
        }
    }
}

c_object
c_tablePeekCursor (
    c_table _this)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    c_tableNode node;
    c_long nrOfKeys, lastKey;

    assert(c_collectionIsType(_this, C_DICTIONARY));
    _READ_BEGIN_(table);

    /* If table->key == NULL, table is a single place buffer. In that case cursor[0]
     * contains the cursor for that buffer. */
    if (table->key == NULL || (nrOfKeys = c_arraySize(table->key)) == 0) {
        return table->cursor[0];
    }

    lastKey = nrOfKeys - 1;
    _READ_END_(table);
    node = table->cursor[lastKey];

    if (node == NULL) { /* o is last record of table */
        return NULL;
    } else {
        return node->contents.object;
    }
}

c_object
c_tableNext (
    c_table _this,
    c_object o)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    c_long nrOfKeys;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _READ_BEGIN_(table);
    if (table->key == NULL || (nrOfKeys = c_arraySize(table->key)) == 0) {
        _READ_END_(table);
        if (o == NULL) {
            return table->contents.object;
        }
        return NULL;
    } else if (ut_avlIsEmpty (&table->contents.tree)) {
        _READ_END_(table);
        return NULL;
    } else if (o == NULL) {
        ut_avlTree_t *index = &table->contents.tree;
        c_tableNode node;
        while (nrOfKeys > 1) {
            assert (!ut_avlIsEmpty (index));
            node = ut_avlFindMin (&c_table_td, index);
            _CHECK_CONSISTENCY_ (table, node);
            index = &node->contents.tree;
            nrOfKeys--;
        }
        assert (!ut_avlIsEmpty (index));
        node = ut_avlFindMin (&c_table_td, index);
        _READ_END_(table);
        return node->contents.object;
    } else {
        c_tableNode node = tableNext (o, &table->contents.tree, table->key, 0);
        _READ_END_(table);
        return node ? node->contents.object : NULL;
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

static c_bool tableReadTakeWalk (ut_avlTree_t *tree, c_bool (*action) (), void *actionarg)
{
    ut_avlIter_t it;
    c_tableNode *n;
    c_bool proceed = TRUE;
    for (n = ut_avlIterFirst (&c_table_td, tree, &it);
         n && proceed;
         n = ut_avlIterNext (&it)) {
        proceed = action (n, actionarg);
    }
    return proceed;
}

static c_bool tableReadTakeRangeWalk (ut_avlTree_t *tree, const c_value *start, int include_start, const c_value *end, int include_end, c_bool (*action) (), void *actionarg)
{
    ut_avlIter_t it;
    c_tableNode n, endn;
    c_bool proceed = TRUE;
    /* Starting point for iteration */
    if (start == NULL) {
        n = ut_avlIterFirst (&c_table_td, tree, &it);
    } else if (include_start) {
        n = ut_avlIterSuccEq (&c_table_td, tree, &it, start);
    } else {
        n = ut_avlIterSucc (&c_table_td, tree, &it, start);
    }
    /* Don't bother looking at the end of the range if there's no
     * matching data */
    if (n == NULL) {
        return proceed;
    }
    /* Endpoint of the iteration: if N is outside the range, abort,
     * else look up the first node beyond the range */
    if (end == NULL) {
        endn = NULL;
    } else if (include_end) {
        if (tableCmp (&n->keyValue, end) > 0) {
            return proceed;
        }
        endn = ut_avlLookupSucc (&c_table_td, tree, end);
    } else {
        if (tableCmp (&n->keyValue, end) >= 0) {
            return proceed;
        }
        endn = ut_avlLookupSuccEq (&c_table_td, tree, end);
    }
    /* Then, just iterate starting at n until endn is reached */
    while (n != endn && proceed) {
        proceed = action (n, actionarg);
        n = ut_avlIterNext (&it);
    }
    return proceed;
}

static c_bool tableReadTakeActionNonMatchingKey (c_tableNode n, c_qPred query, c_long keyIndex)
{
    /* FIXME: this can't be right: for any key but the one nested most
       deeply, N doesn't actually point to an object.  Hence any table
       with multi-level keys can be tricked into returning the wrong
       result or crashes (in practice, only if the key is a string) */
    if ((keyIndex > 0) && (query != NULL)) {
        c_qKey key = query->keyField[keyIndex-1];
        if (key->expr != NULL) {
            c_value v = c_qValue(key->expr,n->contents.object);
            assert(v.kind == V_BOOLEAN);
            if (!v.is.Boolean) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

static void tableReadTakeActionGetRange (c_qRange range, c_value *start, c_value **startRef, c_bool *startInclude, c_value *end, c_value **endRef, c_bool *endInclude)
{
    if (range == NULL) {
        *startRef = NULL; *startInclude = TRUE;
        *endRef   = NULL; *endInclude   = TRUE;
    } else {
        *start = c_qRangeStartValue(range);
        *end = c_qRangeEndValue(range);
        switch (range->startKind) {
        case B_UNDEFINED: *startRef = NULL;  *startInclude = TRUE;  break;
        case B_INCLUDE:   *startRef = start; *startInclude = TRUE;  break;
        case B_EXCLUDE:   *startRef = start; *startInclude = FALSE; break;
        default:
            OS_REPORT_1(OS_ERROR,
                        "Database Collection",0,
                        "Internal error: undefined range kind %d",
                        range->startKind);
            assert(FALSE);
        }
        switch (range->endKind) {
        case B_UNDEFINED: *endRef = NULL; *endInclude = TRUE;  break;
        case B_INCLUDE:   *endRef = end;  *endInclude = TRUE;  break;
        case B_EXCLUDE:   *endRef = end;  *endInclude = FALSE; break;
        default:
            OS_REPORT_1(OS_ERROR,
                        "Database Collection",0,
                        "Internal error: undefined range kind %d",
                        range->endKind);
            assert(FALSE);
        }
    }
}

static c_bool
tableReadAction(
    c_tableNode n,
    tableReadActionArg arg)
{
    c_value start, end;
    c_value *startRef = NULL, *endRef = NULL;
    c_bool startInclude = TRUE, endInclude = TRUE;
    c_bool proceed = TRUE;
    c_qKey key;
    c_long i,nrOfRanges;

    _CHECK_CONSISTENCY_(arg->t,n);

    if (tableReadTakeActionNonMatchingKey (n, arg->query, arg->keyIndex)) {
        return TRUE;
    } else if (arg->keyIndex == c_arraySize(arg->key)) {
        if (!predMatches (arg->query, n->contents.object)) {
            return TRUE;
        } else {
            return arg->action (n->contents.object, arg->arg);
        }
    } else {
        key = arg->query ? arg->query->keyField[arg->keyIndex] : NULL;
        arg->keyIndex++;
        if (key == NULL || key->range == NULL || (nrOfRanges = c_arraySize (key->range)) == 0) {
            proceed = tableReadTakeWalk (&n->contents.tree, tableReadAction, arg);
        } else {
            for (i = 0; i < nrOfRanges && proceed; i++) {
                tableReadTakeActionGetRange (key->range[i], &start, &startRef, &startInclude, &end, &endRef, &endInclude);
                proceed = tableReadTakeRangeWalk(&n->contents.tree,
                                                 startRef,startInclude,
                                                 endRef,endInclude,
                                                 tableReadAction,arg);
            }
        }
        arg->keyIndex--;
        return proceed;
    }
}

static c_bool
c_tableRead (
    c_table _this,
    c_qPred q,
    c_action action,
    c_voidp arg)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    C_STRUCT(tableReadActionArg) a;
    C_STRUCT(c_tableNode) root;
    c_bool proceed = TRUE;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _READ_BEGIN_(t);

    if ((table->key == NULL) || (c_arraySize(table->key) == 0)) {
        if (table->contents.object == NULL) {
            /* skip */
        } else if (predMatches (q, table->contents.object)) {
            proceed = action(table->contents.object,arg);
        }
        _READ_END_(table);
        return proceed;
    }

    root.contents = table->contents;
    a.key = table->key;
    a.action = action;
    a.arg = arg;
#ifdef _CONSISTENCY_CHECKING_
    a.t = table;
    root.table = (c_voidp)table;
#endif
    if (q == NULL) {
        a.keyIndex = 0;
        a.query = q;
        proceed = tableReadAction(&root,&a);
    } else {
        while ((q != NULL) && proceed) {
            a.keyIndex = 0;
            a.query = q;
            proceed = tableReadAction(&root,&a);
            q = q->next;
        }
    }
    _READ_END_(table);
    return proceed;
}

c_bool
c_tableReadCircular (
    c_table _this,
    c_action action,
    c_voidp arg)
{
    c_object obj, pivot;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    pivot = c_tablePeekCursor(_this);
    do {
        obj = c_tableReadCursor(_this);
        if (!obj) {
            /* Read till the end of the table, so wrap around to read up to
             * the pivot. */
            continue;
        }

        /* Perform the action-routine. If it return FALSE we have to stop walking
         * over the table. */
        if (!action(obj, arg)) {
            return FALSE;
        }
    } while (pivot != obj);

    return TRUE;
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
    c_table _this,
    c_qPred q)
{
    c_object o = NULL;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    c_tableRead(_this,q,readOne,&o);
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
#ifdef _CONSISTENCY_CHECKING_
    C_STRUCT(c_table) *t;
#endif
};

C_CLASS(tableTakeActionArg);

static c_bool
tableTakeAction(
    c_tableNode n,
    tableTakeActionArg arg);

static void tableTakeActionDeleteDisposed (c_tableNode n, tableTakeActionArg arg)
{
    c_tableNode d = arg->disposed;
    ut_avlDelete (&c_table_td, &n->contents.tree, d);
    if (arg->keyIndex == c_arraySize(arg->key)) {
        c_free(d->contents.object);
    }
    c_valueFreeRef(d->keyValue);
    c_mmFree(arg->mm, d);
}

static c_bool tableTakeActionWalk (c_tableNode n, tableTakeActionArg arg)
{
    c_bool proceed;
    do {
        proceed = tableReadTakeWalk(&n->contents.tree, tableTakeAction, arg);
        if ((!proceed) && (arg->disposed != NULL)) {
            tableTakeActionDeleteDisposed (n, arg);
        }
    } while ((proceed == FALSE) && (arg->proceed == TRUE));
    return arg->proceed;
}

static c_bool tableTakeActionWalkRange (c_tableNode n, tableTakeActionArg arg, const c_value *startRef, c_bool startInclude, const c_value *endRef, c_bool endInclude)
{
    c_bool proceed;
    do {
        proceed = tableReadTakeRangeWalk(&n->contents.tree,
                                         startRef,startInclude,
                                         endRef,endInclude,
                                         tableTakeAction,arg);
        if ((!proceed) && (arg->disposed != NULL)) {
            tableTakeActionDeleteDisposed (n, arg);
        }
    } while ((proceed == FALSE) && (arg->proceed == TRUE));
    return arg->proceed;
}

static c_bool
tableTakeAction(
    c_tableNode n,
    tableTakeActionArg arg)
{
    c_value start, end;
    c_value *startRef = NULL, *endRef = NULL;
    c_bool startInclude = TRUE, endInclude = TRUE;
    c_bool proceed = TRUE;
    c_qKey key;
    c_long i,nrOfRanges;

    _CHECK_CONSISTENCY_(arg->t,n);

    if (tableReadTakeActionNonMatchingKey (n, arg->pred, arg->keyIndex)) {
        return TRUE;
    } else if (arg->keyIndex == c_arraySize (arg->key)) {
        if (!predMatches (arg->pred, n->contents.object)) {
            return TRUE;
        } else {
            arg->disposed = n;
            arg->count++;
            arg->proceed = arg->action (n->contents.object, arg->arg);
            return FALSE;
        }
    } else {
        key = arg->pred ? arg->pred->keyField[arg->keyIndex] : NULL;
        arg->keyIndex++;
        if (key == NULL || key->range == NULL || (nrOfRanges = c_arraySize (key->range)) == 0) {
            proceed = tableTakeActionWalk (n, arg);
        } else {
            for (i = 0; i < nrOfRanges && proceed; i++) {
                tableReadTakeActionGetRange (key->range[i], &start, &startRef, &startInclude, &end, &endRef, &endInclude);
                proceed = tableTakeActionWalkRange (n, arg, startRef, startInclude, endRef, endInclude);
            }
        }
        if (ut_avlIsEmpty (&n->contents.tree)) {
            arg->disposed = n;
        } else {
            arg->disposed = NULL;
        }
        arg->keyIndex--;
        return proceed;
    }
}

static c_bool
c_tableTake (
    c_table _this,
    c_qPred p,
    c_action action,
    c_voidp arg)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    C_STRUCT(tableTakeActionArg) a;
    C_STRUCT(c_tableNode) root;
    c_bool proceed = TRUE;
    c_long nrOfKeys;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _ACCESS_BEGIN_(table);

    if (table->key == NULL || (nrOfKeys = c_arraySize(table->key)) == 0) {
        if (table->contents.object != NULL) {
            c_object o = NULL;
            /* FIXME: this can't be right: action is called regardless
             * of whether the predicate is satisfied */
            if (predMatches (p, table->contents.object)) {
                o = table->contents.object;
                table->contents.object = NULL;
                table->count--;
            }
            proceed = action (o, arg);
            c_free (o);
        }
    } else {
        a.mm = MM(table);
        a.key = table->key;
        a.action = action;
        a.arg = arg;
        a.count = 0;
        a.proceed = TRUE;
#ifdef _CONSISTENCY_CHECKING_
        a.t = table;
        root.table = (c_voidp)table;
#endif
        root.contents = table->contents;
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
        table->contents = root.contents;
        table->count -= a.count;
    }
    _ACCESS_END_(table);
    return proceed;
}

static c_object
c_tableTakeOne(
    c_table _this,
    c_qPred p)
{
    c_object o = NULL;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    c_tableTake(_this,p,readOne,&o);

    return o;
}

c_long
c_tableCount (
    c_table _this)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    assert(c_collectionIsType(_this, C_DICTIONARY));
    return table->count;
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
        result = arg->action(n->contents.object,arg->actionArg);
    } else {
        arg->nrOfKeys--;
        result = tableReadTakeWalk(&n->contents.tree,c_tableWalkAction,arg);
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
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    struct c_tableWalkActionArg walkActionArg;
    c_bool result = TRUE;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    _READ_BEGIN_(table);
    if (table->count > 0) {
        if ((table->key == NULL) || (c_arraySize(table->key) == 0)) {
            result = action(table->contents.object,actionArg);
        } else {
            walkActionArg.action = action;
            walkActionArg.actionArg = actionArg;
            walkActionArg.nrOfKeys = c_arraySize(table->key) - 1;
            result = tableReadTakeWalk(&table->contents.tree, c_tableWalkAction, &walkActionArg);
        }
    }
    _READ_END_(table);
    return result;
}

c_long
c_tableGetKeyValues(
    c_table _this,
    c_object object,
    c_value *values)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    c_long i, nrOfKeys;
    c_value *currentValuePtr = values;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    assert(table != NULL);
    assert(object != NULL);

    nrOfKeys = c_arraySize(table->key);
    for (i=0;i<nrOfKeys;i++) {
        *currentValuePtr = c_fieldValue(table->key[i], object);
        currentValuePtr = &(currentValuePtr[1]);
    }
    return nrOfKeys;
}

c_long
c_tableSetKeyValues(
    c_table _this,
    c_object object,
    c_value *values)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    c_long i, nrOfKeys;
    c_value *currentValue;
    c_field *currentField;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    nrOfKeys = c_arraySize(table->key);
    if (nrOfKeys > 0) {
        currentField = (c_field *)table->key;
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
    c_table _this)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    return c_arraySize(table->key);
}

c_array
c_tableKeyList(
    c_table _this)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    return c_keep(table->key);
}

c_char *
c_tableKeyExpr(
    c_table _this)
{
    C_STRUCT(c_table) *table = (C_STRUCT(c_table) *) _this;
    c_long i, nrOfKeys, size;
    c_char *expr;

    assert(c_collectionIsType(_this, C_DICTIONARY));

    size = 0;
    nrOfKeys = c_arraySize(table->key);
    for (i=0;i<nrOfKeys;i++) {
        size += strlen(c_fieldName(table->key[i])) + 1;
    }
    expr = (c_char *)os_malloc(size);
    expr[0] = (c_char)0;
    for (i=0;i<nrOfKeys;i++) {
        os_strcat(expr,c_fieldName(table->key[i]));
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

    OS_UNUSED_ARG(template);

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
ospl_c_select (
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
                        "ospl_c_select: illegal collection kind (%d) specified",
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
        c_list(c)->mm = c_baseMM (base);
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
        c_set(c)->mm = c_baseMM (base);
        ut_avlCInit (&c_set_td, &(c_set(c))->tree);
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
        c_bag(c)->mm = c_baseMM(base);
        c_bag(c)->count = 0;
        ut_avlInit (&c_bag_td, &(c_bag(c))->tree);
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
            if (c_iterResolve(keyNameList,(c_iterResolveCompare)c_compareString,keyName) == NULL) {
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
            t->key = c_arrayNew(c_field_t(base), nrOfKeys);
            for (i=0;i<nrOfKeys;i++) {
                t->key[i] = c_iterTakeFirst(fieldList);
            }
        } else {
            t->key = NULL;
        }
        t->cursor = c_arrayNew(c_voidp_t(base),nrOfKeys ? nrOfKeys : 1);
        c_iterFree(fieldList);
        t->mm = c_baseMM(base);
        if (nrOfKeys == 0) {
            t->contents.object = NULL;
        } else {
            ut_avlInit (&c_table_td, &t->contents.tree);
        }
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
        c_free(c_table(c)->cursor);
    break;
    case C_SET:
        while ((o = c_take(c)) != NULL) {
            c_free(o);
        }
    break;
    case C_BAG:
        while ((o = c_take(c)) != NULL) {
            c_free(o);
        }
    break;
    case C_LIST:
        while ((o = c_take(c)) != NULL) {
            c_free(o);
        }
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
