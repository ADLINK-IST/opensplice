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
#include "d_misc.h"
#include "d_table.h"
#include "d_avltree.h"


#define TABLE_MAGIC     0x54736368      /* "Tsch" */

C_STRUCT(d_table) {
    C_EXTENDS(d_object);
    c_ulong    size;            /**< the number of entries                 */
    int ( *    compare) ();     /**< the user's compare function           */
    void ( *   cleanAction) (); /**< the user's cleanup action             */
    d_avlNode  tree;
};

/***********************************************************************
 ***********************************************************************/

static c_voidp
tableRemove(
    d_table table,
    c_voidp arg,
    int ( * compare )() )
{
    c_voidp item;

    assert(d_objectIsValid(d_object(table), D_TABLE) == TRUE);
    item = NULL;
    /* QAC EXPECT 3892; */
    if (table->tree) {
        item = d_avlTreeRemove(&table->tree, arg, compare);
        if (item) {
            --table->size;
            if (table->size == 0) {
                table->tree = NULL;
            }
        }
    }
    return item;
}

/***********************************************************************
 ***********************************************************************/

static c_voidp
tableTake(
    d_table table )
{
    c_voidp item;

    assert(d_objectIsValid(d_object(table), D_TABLE) == TRUE);
    item = NULL;
    /* QAC EXPECT 3892; */
    if (table->tree) {
        item = d_avlTreeTake(&table->tree);
        if (item) {
            --table->size;
            if (table->size == 0) {
                table->tree = NULL;
            }
        }
    }
    return item;
}

/***********************************************************************
 ***********************************************************************/
/*
static int
alwaysEqual(
    c_voidp one,
    c_voidp two )
{
    if (one) {
    }
    if (two) {
    }
    return 0;
}
*/
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
        assert(d_objectIsValid(d_object(table), D_TABLE) == TRUE);
        size = table->size;
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
        /* QAC EXPECT 3892; */
        assert(d_objectIsValid(d_object(table), D_TABLE) == TRUE);
        existingItem = d_avlTreeInsert(&table->tree, object, table->compare);
        if (!existingItem) {
            ++table->size;
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
        /* QAC EXPECT 3892; */
        assert(d_objectIsValid(d_object(table), D_TABLE) == TRUE);
        if (table->tree) {
            dataFound = d_avlTreeFind(table->tree, arg, table->compare);
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
        /* QAC EXPECT 3892; */
        assert(d_objectIsValid(d_object(table), D_TABLE) == TRUE);
        if (table->tree) {
            dataFound = d_avlTreeFirst(table->tree);
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
    c_bool result = FALSE;

    if (table) {
        /* QAC EXPECT 3892; */
        assert(d_objectIsValid(d_object(table), D_TABLE) == TRUE);
        if (table->tree) {
            result = d_avlTreeWalk(&table->tree, action, userData);
        } else {
            result = TRUE;
        }
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
        result = tableRemove(table, arg, table->compare);
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
        result = tableTake(table);
    }
    return result;
}

/***********************************************************************
 ***********************************************************************/

void
d_tableDeinit(
    d_object object)
{
    d_table table;

    assert(d_objectIsValid(object, D_TABLE) == TRUE);

    if(object){
        table = d_table(object);

        d_avlTreeFree(table->tree, table->cleanAction);
        table->size = 0;
        table->compare = NULL;
        table->tree = NULL;
    }
}

void
d_tableFree(
    d_table  table )
{
    if (table) {
        assert(d_objectIsValid(d_object(table), D_TABLE) == TRUE);
        d_objectFree(d_object(table), D_TABLE);
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

    assert(compare != (int(*)())NULL);
    table = (d_table)d_malloc((os_uint32)C_SIZEOF(d_table), "Table");
    if (table) {
        /* QAC EXPECT 3892; */
        d_objectInit(d_object(table), D_TABLE, d_tableDeinit);
        table->size = 0;
        table->compare = compare;
        table->cleanAction = cleanAction;
        table->tree = NULL;
    }
    return table;
}
