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
#include <assert.h>
#include <stddef.h>

#include "os.h"
#include "ut_collection.h"
#include "ut_avl.h"

/* ***************************************************************************
 * DATA TYPES
 * ***************************************************************************/
OS_CLASS(ut_tableWalkActionArg);
OS_CLASS(ut_tableNodesCollection);
OS_CLASS(ut_tableNode);

#define ut_tableNode(a) ((ut_tableNode) (a))

OS_STRUCT(ut_collection) {
    ut_collectionType type;
    ut_compareElementsFunc cmpFunc; /* pointer to user provided compare fuction */
    void *args;
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
};

OS_STRUCT(ut_tableNodesCollection) {
    os_uint32 numberOfNodes;
    os_int32 nodeSeqNumber;
    ut_tableNode * tableNodes;
};

/* ***************************************************************************
 * FUNCTION DECLARATIONS
 * ***************************************************************************/
static ut_tableNode
ut_newTableNode(
    void *key,
    void *value)
{
    ut_tableNode node;
    node = NULL;

    node = os_malloc((os_uint32)OS_SIZEOF(ut_tableNode));
    assert(node);

    node->key = key;
    node->value = value;

    return node;
}

/* ***************************************************************************
 * FUNCTION IMPLEMENTATION
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

ut_collection
ut_tableNew(
        const ut_compareElementsFunc cmpFunc,
        void *arg)
{
    ut_table table;

    table = os_malloc(sizeof(*table));
    if(table){
        /* This 'super'-init should optimally be done in a ut_collectionInit */
        ut_collection(table)->type = UT_TABLE;
        ut_collection(table)->cmpFunc = cmpFunc;
        ut_collection(table)->args = arg;
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
    }

    return ut_collection(table);
}

void
ut_collectionFree(
    ut_collection c,
    const ut_freeElementFunc action,
    void *arg)
{
    assert(c);
    assert(action);
    (void) arg;

    switch (c->type) {
    case UT_TABLE :
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
        {
            ut_avlDPath_t p;
            ut_table table = ut_table(c);
            ut_tableNode foundNode = ut_avlCLookupDPath (&table->td, &table->tree, o, &p);
            if (foundNode) {
                result = foundNode->value;
                ut_avlCDeleteDPath (&table->td, &table->tree, foundNode, &p);
                os_free (foundNode);
            }
        }
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

    contains = 0;

    switch (c->type) {
    case UT_TABLE :
        {
            ut_table table = ut_table(c);
            ut_tableNode foundNode = ut_avlCLookup (&table->td, &table->tree, o);
            if (foundNode) {
                contains = 1;
            }
        }
    break;
    default :
        fprintf(stderr, "ut_contains: This collection type is not yet supported\n");
        assert(0);
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

    count = -1;

    switch (c->type) {
    case UT_TABLE :
        count = (os_int32) ut_avlCCount(&ut_table(c)->tree);
    break;
    default :
        fprintf(stderr, "ut_count: This collection type is not yet supported\n");
        assert(0);
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

    success = 0;

    switch (c->type) {
    case UT_TABLE :
        {
            ut_table table = ut_table (c);
            ut_avlCIter_t it;
            ut_tableNode node;

            success = 1;
            for (node = ut_avlCIterFirst (&table->td, &table->tree, &it);
                 node != NULL && success;
                 node = ut_avlCIterNext (&it)) {
                success = action (node->value, arg);
            }
        }
    break;
    default :
        fprintf(stderr, "ut_walk: This collection type is not yet supported\n");
        assert(0);
        break;
    }

    return success;
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

    assert(ut_collection(t)->type == UT_TABLE);

    if (ut_avlCLookupIPath (&t->td, &t->tree, key, &p) == NULL) {
        node = ut_newTableNode(key, value);
        ut_avlCInsertIPath (&t->td, &t->tree, node, &p);
        returnValue = 1;
    }

    return returnValue;
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
ut_tableFree(
    ut_table table,
    const ut_freeElementFunc freeKey,
    void *freeKeyArg,
    const ut_freeElementFunc freeValue,
    void *freeValueArg)
{
    struct ut_tableFreeHelper_arg arg;
    arg.freeKey = freeKey;
    arg.freeKeyArg = freeKeyArg;
    arg.freeValue = freeValue;
    arg.freeValueArg = freeValueArg;
    ut_avlCFreeArg (&table->td, &table->tree, ut_tableFreeHelper, &arg);
    os_free(table);
}
