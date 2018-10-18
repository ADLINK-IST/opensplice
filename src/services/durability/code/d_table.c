/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include <stddef.h>
#include "ut_avl.h"
#include "d__misc.h"
#include "d__table.h"




/**************************************************************
 * Public functions
 **************************************************************/

c_ulong
d_tableSize(
    d_table table )
{
    c_ulong size;

    size = 0;
    if (table) {
        /* QAC EXPECT 3892; */
        assert(d_tableIsValid(table));
        size = (c_ulong) ut_avlCCount (&table->tree);
    }
    return size;
}

/***********************************************************************
 ***********************************************************************/

/** returns zero if the entry is added */
c_voidp
d_tableInsert(
    d_table table,
    c_voidp object )
{
    c_voidp existingItem;

    existingItem = object;
    if (table) {
        d_tableNode node;
        ut_avlIPath_t p;
        /* QAC EXPECT 3892; */
        assert(d_tableIsValid(table));
        if ((node = ut_avlCLookupIPath (&table->td, &table->tree, object, &p)) != NULL) {
            existingItem = node->object;
        } else if ((node = d_malloc (sizeof (*node), "d_tableNode")) != NULL) {
            node->object = object;
            ut_avlCInsertIPath (&table->td, &table->tree, node, &p);
            existingItem = NULL;
        }
    }
    return existingItem;
}

/***********************************************************************
 ***********************************************************************/

c_voidp
d_tableFind(
    d_table table,
    c_voidp arg )
{
    c_voidp   dataFound;

    dataFound = NULL;
    if (arg && table) {
        d_tableNode node;
        /* QAC EXPECT 3892; */
        assert(d_tableIsValid(table));
        if ((node = ut_avlCLookup (&table->td, &table->tree, arg)) != NULL) {
            dataFound = node->object;
        }
    }
    return dataFound;
}

/***********************************************************************
 ***********************************************************************/

c_voidp
d_tableFirst(
    d_table table )
{
    c_voidp   dataFound;

    dataFound = NULL;
    if (table) {
        d_tableNode node;
        /* QAC EXPECT 3892; */
        assert(d_tableIsValid(table));
        if ((node = ut_avlCFindMin (&table->td, &table->tree)) != NULL) {
            dataFound = node->object;
        }
    }
    return dataFound;
}

/***********************************************************************
 ***********************************************************************/

c_bool
d_tableWalk(
    d_table    table,
    c_bool ( * action ) (),
    c_voidp    userData )
{
    c_bool result;

    if (table) {
        ut_avlCIter_t it;
        d_tableNode n;
        /* QAC EXPECT 3892; */
        assert(d_tableIsValid(table));
        result = TRUE;
        for (n = ut_avlCIterFirst (&table->td, &table->tree, &it);
             n && result;
             n = ut_avlCIterNext (&it)) {
            result = action (n->object, userData);
        }
    } else {
        result = FALSE;
    }
    return result;
}

/***********************************************************************
 ***********************************************************************/

c_voidp
d_tableRemove(
    d_table  table,
    c_voidp  arg )
{
    c_voidp result;

    result = NULL;
    if (table) {
        d_tableNode node;
        ut_avlDPath_t p;
        /* QAC EXPECT 3892; */
        assert(d_tableIsValid(table));
        if ((node = ut_avlCLookupDPath (&table->td, &table->tree, arg, &p)) != NULL) {
            result = node->object;
            ut_avlCDeleteDPath (&table->td, &table->tree, node, &p);
            d_free (node);
        }
    }
    return result;
}

/***********************************************************************
 ***********************************************************************/

c_voidp
d_tableTake(
    d_table table)
{
    c_voidp result;

    result = NULL;
    if (table) {
        /* Delete an arbitrarily chosen node: the obvious candidates
           are: root, min and max.  */
        d_tableNode node;
        if ((node = ut_avlCRoot (&table->td, &table->tree)) != NULL) {
            result = node->object;
            ut_avlCDelete (&table->td, &table->tree, node);
            d_free(node);
        }
    }
    return result;
}

/***********************************************************************
 ***********************************************************************/

static void d_tableFreeHelper (void *vnode, void *varg)
{
    d_tableNode node = vnode;
    d_table table = varg;
    if (table->cleanAction) {
        table->cleanAction (node->object);
    }
    d_free (node);
}

void
d_tableDeinit(
    d_table table)
{
    assert(d_tableIsValid(table));

    if (table) {
       /* Clean up all stuff that was allocated */
        ut_avlCFreeArg (&table->td, &table->tree, d_tableFreeHelper, table);
        /* Call super-deinit */
        d_objectDeinit(d_object(table));
    }
}

void
d_tableFree(
    d_table table )
{
    assert(d_tableIsValid(table));

    if (table) {
        d_objectFree(d_object(table));
    }
}

/***********************************************************************
 ***********************************************************************/

d_table
d_tableNew(
    int ( *  compare )(),
    void ( * cleanAction )() )
{
    d_table table;

    assert(compare != 0);
    /* Allocate table object */
    table = (d_table)d_malloc(C_SIZEOF(d_table), "Table");
    if (table) {
        /* QAC EXPECT 3892; */
        /* Call super-init */
        d_objectInit(d_object(table), D_TABLE,
                     (d_objectDeinitFunc)d_tableDeinit);
        /* Initialize table object */
        ut_avlCTreedefInit (&table->td,
                            offsetof (C_STRUCT(d_tableNode), avlnode), offsetof (C_STRUCT(d_tableNode), object),
                            (int (*) (const void *, const void *)) compare, 0,
                            UT_AVL_TREEDEF_FLAG_INDKEY);
        ut_avlCInit (&table->td, &table->tree);
        table->cleanAction = cleanAction;
    }
    return table;
}


void *
d_tableIterFirst(
    d_table table,
    d_tableIter *iter)
{
    if (table == NULL) {
        return NULL;
    } else {
        d_tableNode n = ut_avlCIterFirst(&table->td, &table->tree, &iter->it);
        return n ? n->object : NULL;
    }
}

void *
d_tableIterNext(
    d_tableIter *iter)
{
    d_tableNode n = ut_avlCIterNext(&iter->it);
    return n ? n->object : NULL;
}
