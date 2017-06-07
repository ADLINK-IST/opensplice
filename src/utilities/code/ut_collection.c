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
#include <assert.h>
#include <stddef.h>

#include "vortex_os.h"
#include "ut_collection.h"
#include "ut_avl.h"

#define ut_tableNode(node) ((ut_tableNode) (node))
#define ut_setNode(node) ((ut_setNode) (node))

/* ***************************************************************************
 * DATA TYPES
 * ***************************************************************************/
OS_CLASS(ut_tableNode);
OS_CLASS(ut_setNode);

OS_STRUCT(ut_collection) {
    ut_collectionType type;
    ut_compareElementsFunc cmpFunc; /* pointer to user provided compare function */
    void *cmpArg;
    ut_freeElementFunc freeValue;
    void *freeValueArg;
};

OS_STRUCT(ut_setNode) {
    ut_avlNode_t avlnode; /* setNode is a specialized avlNode */
    void *value; /* value of this node */
};

OS_STRUCT(ut_set) {
    OS_EXTENDS(ut_collection);
    ut_avlCTreedef_t td;
    ut_avlCTree_t tree; /* Internal collection that stores the table elements */
};

OS_STRUCT(ut_tableNode) {
    ut_avlNode_t avlnode; /* tableNode is a specialized avlNode */
    void *key; /* identifier for this node */
    void *value; /* value of this node */
};

OS_STRUCT(ut_table) {
    OS_EXTENDS(ut_collection);
    ut_avlCTreedef_t td;
    ut_avlCTree_t tree; /* Internal collection that stores the table elements */
    ut_freeElementFunc freeKey;
    void *freeKeyArg;
};

/* ***************************************************************************
 * FUNCTION DECLARATIONS
 * ***************************************************************************/

/**
 * \brief Initialize the common parent for each collection type.
 *
 * This function is to be used by each collection type to initialize
 * its common parent. The parent class stores information about the
 * exact kind of the collection, it stores a function and argument
 * needed to compare 2 element types of the collection, and it stores
 * a function to deallocate contained values of the element type.
 *
 * \param cmpFunc      Compare function for elements stored in this table.
 * \param cmpArg       Arguments for the compare function.
 * \param freeValue    User provided callback function that takes responsibility
 *                     of the values within the table.
 * \param freeValueArg The generic application hook attribute that is passed to
 *                     the freeValue method.
 */

static void
ut_collectionInit(
        ut_collection c,
        ut_collectionType type,
        const ut_compareElementsFunc cmpFunc,
        void *cmpArg,
        const ut_freeElementFunc freeValue,
        void *freeValueArg)
{
    c->type = type;
    c->cmpFunc = (ut_compareElementsFunc)cmpFunc;
    c->cmpArg = cmpArg;
    c->freeValue = (ut_freeElementFunc) freeValue;
    c->freeValueArg = freeValueArg;
}


/* ***************************************************************************
 * SET FUNCTION IMPLEMENTATION
 * ***************************************************************************/

static ut_setNode
ut_newSetNode(
    void *value)
{
    ut_setNode node;
    node = NULL;

    node = os_malloc(OS_SIZEOF(ut_setNode));
    node->value = value;

    return node;
}

ut_set
ut_setNew(
    const ut_compareElementsFunc cmpFunc,
    void *cmpArg,
    const ut_freeElementFunc freeValue,
    void *freeValueArg)
{
    ut_set set;

    set = os_malloc(sizeof(*set));
    ut_collectionInit(
                ut_collection(set),
                UT_SET,
                cmpFunc,
                cmpArg,
                freeValue,
                freeValueArg);
#if 1
    ut_avlCTreedefInit_r (&set->td,
                          offsetof (struct ut_setNode_s, avlnode),
                          offsetof (struct ut_setNode_s, value),
                          (int (*) (const void *, const void *, void *)) cmpFunc, cmpArg, 0,
                          UT_AVL_TREEDEF_FLAG_INDKEY);
#else
    ut_avlCTreedefInit_r (&set->td,
                          offsetof (struct ut_setNode_s, avlnode),
                          offsetof (struct ut_setNode_s, value),
                          tableCmp, ut_collection(table), 0,
                          UT_AVL_TREEDEF_FLAG_INDKEY);
#endif
    ut_avlCInit (&set->td, &set->tree);

    return set;
}

os_int32
ut_setInsert(
    ut_set s,
    void *value)
{
    os_int32 returnValue;
    ut_setNode node;
    ut_avlIPath_t p;

    returnValue = 0;

    assert(ut_collection(s)->type == UT_SET);

    if (ut_avlCLookupIPath (&s->td, &s->tree, value, &p) == NULL) {
        node = ut_newSetNode(value);
        ut_avlCInsertIPath (&s->td, &s->tree, node, &p);
        returnValue = 1;
    }
    return returnValue;
}

static void *
ut_setRemove(
    ut_set s,
    void *o)
{
    void *result = NULL;
    ut_avlDPath_t p;
    ut_setNode foundNode;

    assert(s != 0);
    assert(ut_collection(s)->type == UT_SET);

    foundNode = ut_avlCLookupDPath (&s->td, &s->tree, o, &p);
    if (foundNode) {
        result = foundNode->value;
        ut_avlCDeleteDPath (&s->td, &s->tree, foundNode, &p);
        os_free (foundNode);
    }
    return result;
}

static os_int32
ut_setContains(
    ut_set s,
    void *o)
{
    os_int32 contains = 0;
    ut_setNode foundNode;

    assert(s != 0);
    assert(ut_collection(s)->type == UT_SET);

    foundNode = ut_avlCLookup (&s->td, &s->tree, o);
    if (foundNode) {
        contains = 1;
    }
    return contains;
}

static os_int32
ut_setWalk(
    ut_set s,
    const ut_actionFunc action,
    void *arg)
{
    os_int32 success;
    ut_avlCIter_t it;
    ut_setNode node;

    assert(s != 0);
    assert(ut_collection(s)->type == UT_SET);
    assert(action);

    success = 1;
    for (node = ut_avlCIterFirst (&s->td, &s->tree, &it);
         node != NULL && success;
         node = ut_avlCIterNext (&it)) {
        success = action (node->value, arg);
    }
    return success;
}

struct ut_setFreeHelper_arg {
    ut_freeElementFunc freeValue;
    void *freeValueArg;
};

static void ut_setFreeHelper (void *vnode, void *varg)
{
    ut_setNode node = vnode;
    struct ut_setFreeHelper_arg *arg = varg;
    if (arg->freeValue) {
        arg->freeValue(node->value, arg->freeValueArg);
    }
    os_free (node);
}

void
ut_setClear(
    ut_set set)
{
    struct ut_setFreeHelper_arg arg;
    arg.freeValue = ut_collection(set)->freeValue;
    arg.freeValueArg = ut_collection(set)->freeValueArg;
    ut_avlCFreeArg (&set->td, &set->tree, ut_setFreeHelper, &arg);
}

void
ut_setFree(
    ut_set set)
{
    ut_setClear(set);
    os_free(set);
}

/* ***************************************************************************
 * LIST FUNCTION IMPLEMENTATION
 * ***************************************************************************/

#ifndef NDEBUG
ut_collection
ut_listNew(
    const ut_compareElementsFunc cmpFunc,
    void *arg)
{
    OS_UNUSED_ARG(cmpFunc);
    OS_UNUSED_ARG(arg);

    fprintf(stderr, "ut_listNew: this function has not been implemented yet\n");
    assert(0);
    return NULL;
}

/* ***************************************************************************
 * BAG FUNCTION IMPLEMENTATION
 * ***************************************************************************/

ut_collection
ut_bagNew (
    const ut_compareElementsFunc cmpFunc,
    void *arg)
{
    OS_UNUSED_ARG(cmpFunc);
    OS_UNUSED_ARG(arg);

    fprintf(stderr, "ut_bagNew: this function has not been implemented yet\n");
    assert(0);
    return NULL;
}
#endif

/* ***************************************************************************
 * TABLE FUNCTION IMPLEMENTATION
 * ***************************************************************************/

#if 1
/* The original interface has type (os_equality (*) (void *, void *,
   void *)), but the compare functions all do treat the first two
   parameters as const.

   Currently os_equality is defined s.t. OS_LT < 0, OS_EQ = 0 and
   OS_GT > 0, which matches the requirements and usually allows using
   a ut_compareElementsFunc as if it were a "C standard" compare
   function returning an int.

   To avoid nasty surprises, we check two things at compile time: that
   the enum constants indeed satisfy this requirement, and that the
   sizeof of the return type matches.

   The wrapper is sitting pretty here in case we run into a platform
   on which the constraints don't hold. */
struct tableCmp_constraints {
    char require_LT_lt_0[-1+2*((int) OS_LT < 0)];
    char require_EQ_eq_0[-1+2*((int) OS_EQ == 0)];
    char require_GT_gt_0[-1+2*((int) OS_GT > 0)];
    char require_sizeof_os_equality_eq_int[-1+2*(sizeof(os_equality) == sizeof (int))];
    char non_empty_dummy_last_member[1];
};
#else
static int tableCmp (const void *a, const void *b, void *arg)
{
    const OS_STRUCT (ut_collection) *c = ut_collection(arg);
    os_equality r = c->cmpFunc ((void *) a, (void *) b, c->args);
    return (r == OS_EQ) ? 0 : (r == OS_LT) ? -1 : 1;
}
#endif

static ut_tableNode
ut_newTableNode(
    void *key,
    void *value)
{
    ut_tableNode node;
    node = NULL;

    node = os_malloc(OS_SIZEOF(ut_tableNode));
    node->key = key;
    node->value = value;

    return node;
}

ut_table
ut_tableNew(
    const ut_compareElementsFunc cmpFunc,
    void *arg,
    const ut_freeElementFunc freeKey,
    void *freeKeyArg,
    const ut_freeElementFunc freeValue,
    void *freeValueArg)
{
    ut_table table;

    table = os_malloc(sizeof(*table));
    ut_collectionInit(
                ut_collection(table),
                UT_TABLE,
                cmpFunc,
                arg,
                freeValue,
                freeValueArg);
#if 1
    ut_avlCTreedefInit_r (&table->td,
                              offsetof (struct ut_tableNode_s, avlnode),
                              offsetof (struct ut_tableNode_s, key),
                              (int (*) (const void *, const void *, void *)) cmpFunc, arg, 0,
                              UT_AVL_TREEDEF_FLAG_INDKEY);
#else
    ut_avlCTreedefInit_r (&table->td,
                              offsetof (struct ut_tableNode_s, avlnode),
                              offsetof (struct ut_tableNode_s, key),
                              tableCmp, ut_collection(table), 0,
                              UT_AVL_TREEDEF_FLAG_INDKEY);
#endif
    ut_avlCInit (&table->td, &table->tree);
    table->freeKey = freeKey;
    table->freeKeyArg = freeKeyArg;
    return table;
}

os_int32
ut_tableInsert(
    ut_table t,
    void *key,
    void *value)
{
    os_int32 returnValue;
    ut_tableNode node;
    ut_avlIPath_t p;

    returnValue = 0;

    assert(t);
    assert(ut_collection(t)->type == UT_TABLE);

    if (ut_avlCLookupIPath (&t->td, &t->tree, key, &p) == NULL) {
        node = ut_newTableNode(key, value);
        ut_avlCInsertIPath (&t->td, &t->tree, node, &p);
        returnValue = 1;
    }
    return returnValue;
}

static void *
ut_tableRemove(
    ut_table t,
    void *o)
{
    void *result = NULL;
    ut_avlDPath_t p;
    ut_tableNode foundNode;

    assert(t != 0);
    assert(ut_collection(t)->type == UT_TABLE);

    foundNode = ut_avlCLookupDPath (&t->td, &t->tree, o, &p);
    if (foundNode) {
        result = foundNode->value;
        ut_avlCDeleteDPath (&t->td, &t->tree, foundNode, &p);
        os_free (foundNode);
    }
    return result;
}

static os_int32
ut_tableContains(
    ut_table t,
    void *o)
{
    os_int32 contains = 0;
    ut_tableNode foundNode;

    assert(t != 0);
    assert(ut_collection(t)->type == UT_TABLE);

    foundNode = ut_avlCLookup (&t->td, &t->tree, o);
    if (foundNode) {
        contains = 1;
    }
    return contains;
}

static os_int32
ut_tableWalk(
    ut_table t,
    const ut_actionFunc action,
    void *arg)
{
    os_int32 success;
    ut_avlCIter_t it;
    ut_tableNode node;

    assert(t != 0);
    assert(ut_collection(t)->type == UT_TABLE);
    assert(action);

    success = 1;
    for (node = ut_avlCIterFirst (&t->td, &t->tree, &it);
         node != NULL && success;
         node = ut_avlCIterNext (&it)) {
        success = action (node->value, arg);
    }
    return success;
}

os_int32
ut_tableKeyValueWalk(
    ut_table t,
    const ut_actionKeyValueFunc action,
    void *arg)
{
    os_int32 success;
    ut_avlCIter_t it;
    ut_tableNode node;

    assert(t != 0);
    assert(ut_collection(t)->type == UT_TABLE);
    assert(action);

    success = 1;
    for (node = ut_avlCIterFirst (&t->td, &t->tree, &it);
         node != NULL && success;
         node = ut_avlCIterNext (&it)) {
        success = action (node->key, node->value, arg);
    }
    return success;
}

void *
ut_tableNext(
    ut_table table,
    void *o)
{
    ut_tableNode nextNode;
    nextNode = ut_avlCLookupSucc (&table->td, &table->tree, o);
    if (nextNode) {
        return nextNode->value;
    } else {
        return NULL;
    }
}

struct ut_tableFreeHelper_arg {
    ut_freeElementFunc freeKey;
    void *freeKeyArg;
    ut_freeElementFunc freeValue;
    void *freeValueArg;
};

static void ut_tableFreeHelper (void *vnode, void *varg)
{
    ut_tableNode node = vnode;
    struct ut_tableFreeHelper_arg *arg = varg;
    if (arg->freeKey) {
        arg->freeKey(node->key, arg->freeKeyArg);
    }
    if (arg->freeValue) {
        arg->freeValue(node->value, arg->freeValueArg);
    }
    os_free (node);
}

void
ut_tableClear(
    ut_table table)
{
    struct ut_tableFreeHelper_arg arg;
    arg.freeKey = table->freeKey;
    arg.freeKeyArg = table->freeKeyArg;
    arg.freeValue = ut_collection(table)->freeValue;
    arg.freeValueArg = ut_collection(table)->freeValueArg;
    ut_avlCFreeArg (&table->td, &table->tree, ut_tableFreeHelper, &arg);
}

void
ut_tableFree(
    ut_table table)
{
    struct ut_tableFreeHelper_arg arg;
    arg.freeKey = table->freeKey;
    arg.freeKeyArg = table->freeKeyArg;
    arg.freeValue = ut_collection(table)->freeValue;
    arg.freeValueArg = ut_collection(table)->freeValueArg;
    ut_avlCFreeArg (&table->td, &table->tree, ut_tableFreeHelper, &arg);
    os_free(table);
}


void
ut_clear(
    ut_collection c)
{
    assert(c);

    switch (c->type) {
    case UT_TABLE :
        ut_tableClear(ut_table(c));
        break;
    case UT_SET :
        ut_setClear(ut_set(c));
        break;
    default :
        fprintf(stderr, "ut_clear: This collection type is not yet supported\n");
        assert(0);
        break;
    }

    return;
}

void
ut_collectionFree(
    ut_collection c)
{
    assert(c);

    switch (c->type) {
    case UT_TABLE :
        ut_tableFree(ut_table(c));
        break;
    case UT_SET :
        ut_setFree(ut_set(c));
        break;
    default :
        fprintf(stderr, "ut_collectionFree: This collection type is not yet supported\n");
        assert(0);
        break;
    }

    return;
}

void *
ut_get(
    ut_collection c,
    void *o)
{
    void *result;

    assert(c);
    assert(o);

    result = NULL;

    switch (c->type) {
    case UT_TABLE :
        {
            ut_table table = ut_table(c);
            ut_tableNode foundNode = ut_avlCLookup (&table->td, &table->tree, o);
            if (foundNode != NULL) {
                result = foundNode->value;
            }
        }
        break;
    case UT_SET :
        {
            ut_set set = ut_set(c);
            ut_setNode foundNode = ut_avlCLookup (&set->td, &set->tree, o);
            if (foundNode != NULL) {
                result = foundNode->value;
            }
        }
        break;
    default :
        fprintf(stderr, "ut_get: This collection type is not yet supported\n");
        assert(0);
        break;
    }

    return result;
}

void *
ut_remove(
    ut_collection c,
    void *o)
{
    void *result;

    assert(c);
    assert(o);

    result = NULL;

    switch (c->type) {
    case UT_TABLE :
        result = ut_tableRemove(ut_table(c), o);
    break;
    case UT_SET :
        result = ut_setRemove(ut_set(c), o);
    break;
    default :
        fprintf(stderr, "ut_remove: This collection type is not yet supported\n");
        assert(0);
    break;
    }

    return result;
}

os_int32
ut_contains(
    ut_collection c,
    void *o)
{
    os_int32 contains;

    assert(c);
    assert(o);

    switch (c->type) {
    case UT_TABLE :
        contains = ut_tableContains(ut_table(c), o);
    break;
    case UT_SET :
        contains = ut_setContains(ut_set(c), o);
    break;
    default :
        fprintf(stderr, "ut_contains: This collection type is not yet supported\n");
        assert(0);
        contains = 0;
    break;
    }

    return contains;
}

os_int32
ut_count(
    ut_collection c)
{
    os_int32 count;

    assert(c);

    switch (c->type) {
    case UT_TABLE :
        count = (os_int32) ut_avlCCount(&ut_table(c)->tree);
    break;
    case UT_SET :
        count = (os_int32) ut_avlCCount(&ut_set(c)->tree);
    break;
    default :
        fprintf(stderr, "ut_count: This collection type is not yet supported\n");
        assert(0);
        count = -1;
    break;
    }

    return count;
}

os_int32
ut_walk(
    ut_collection c,
    const ut_actionFunc action,
    void *arg)
{
    os_int32 success;

    assert(c);
    assert(action);

    switch (c->type) {
    case UT_TABLE :
        success = ut_tableWalk(ut_table(c), action, arg);
        break;
    case UT_SET :
        success = ut_setWalk(ut_set(c), action, arg);
        break;
    default :
        fprintf(stderr, "ut_walk: This collection type is not yet supported\n");
        assert(0);
        success = 0;
        break;
    }

    return success;
}
