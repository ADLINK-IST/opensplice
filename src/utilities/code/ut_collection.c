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
#include <assert.h>

#include "os.h"
#include "ut_collection.h"
#include "ut__avltree.h"
#include "ut__table.h"

#define ut_tableWalkActionArg(a) ((ut_tableWalkActionArg)(a))

/* ***************************************************************************
 * DATA TYPES
 * ***************************************************************************/
OS_CLASS(ut_tableWalkActionArg);
OS_CLASS(ut_tableNodesCollection);

OS_STRUCT(ut_collection) {
    ut_collectionType type;
    ut_compareElementsFunc cmpFunc; /* pointer to user provided compare fuction */
    void *args;
};

OS_STRUCT(ut_table) {
    OS_EXTENDS(ut_collection);
    ut_avlTree tree; /* Internal collection that stores the table elements */
};

OS_STRUCT(ut_tableWalkActionArg) {
    ut_actionFunc tableActionFunc;
    void *actionArg;
    ut_collection collection;
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

/**
 * \brief Wrapper function for the comparison of two key values.
 * 
 * This operation gets the key values from both objects and passes them to the
 * user provided compare function which is retrieved from the collection object.
 * 
 * \param o1 Object to which the second object is compared.
 * \param o2 The second object that is compared.
 * \param c Collection on which this operation takes place. 
 * 
 * \return C_LT, C_EQ, or a C_GT as the first argument is less than, 
 *         equal to, or greater than the second.
 */
static os_equality 
ut_tableCmpWrapper(
    void *o1,
    void *o2,
    void *c)
{    
    os_equality result = OS_ER;
    
    result = ut_collection(c)->cmpFunc(ut_tableNode(o1)->key, ut_tableNode(o2)->key, ut_collection(c)->args);
    return result;
}

/**
 * \brief Wrapper function for a table action.
 * 
 * This function gets the value from the table node and passes it to the user
 * provided action function.
 * 
 * \param node tableNode on which the action will be performed.
 * \param argw User arguments wrapped in a . 
 * 
 */
static os_int32
ut_tableActionWrapper(
    void *node,
    void *argw)
{
    return ut_tableWalkActionArg(argw)->tableActionFunc(ut_tableNode(node)->value, (void *)(ut_tableWalkActionArg(argw)->actionArg));
}

/**
 * \brief Callback function used by the ut_FreeTable operation to collect all 
 *        ut_tableNodes of the current ut_table in an array.
 * 
 * \param node The current node processed by the walk operation.
 * \param allNodes A struct containing the array of 
 */
static os_int32
ut_tableCollectNodes(
    ut_tableNode node,
    ut_tableNodesCollection allNodes) 
{
    assert(node);
    assert(allNodes);
    
    allNodes->tableNodes[allNodes->nodeSeqNumber] = node;
    allNodes->nodeSeqNumber++;
    
    return 1;
}

/* ***************************************************************************
 * FUNCTION IMPLEMENTATION
 * ***************************************************************************/
#ifndef NDEBUG
ut_collection 
ut_listNew(
    const ut_compareElementsFunc cmpFunc,
    void *arg)
{
    fprintf(stderr, "ut_listNew: this function has not been implemented yet\n");
    assert(0);
    return NULL;
}

ut_collection 
ut_setNew(
    const ut_compareElementsFunc cmpFunc,
    void *arg)
{
    fprintf(stderr, "ut_setNew: this function has not been implemented yet\n");
    assert(0);
    return NULL;
}

ut_collection 
ut_bagNew (
    const ut_compareElementsFunc cmpFunc,
    void *arg)
{
    fprintf(stderr, "ut_bagNew: this function has not been implemented yet\n");
    assert(0); 
    return NULL;
}
#endif

ut_collection
ut_tableNew(
    const ut_compareElementsFunc cmpFunc,
    void *arg)
{
    ut_table table;
    ut_avlTree tree;
    
    table = NULL;
    tree = NULL;
    
    table = (ut_table)os_malloc((os_uint32)OS_SIZEOF(ut_table));
    ut_collection(table)->type = UT_TABLE;
    
    ut_collection(table)->cmpFunc = cmpFunc;
    ut_collection(table)->args = arg;
    
    tree = ut_avlTreeNew(0);
    
    table->tree = tree;
    
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
            ut_tableNode node;
            ut_tableNode foundNode;
            node = ut_newTableNode(o, NULL);
        
            foundNode = ut_avlTreeFind(ut_table(c)->tree,
                                       node,
                                       ut_tableCmpWrapper, 
                                       (void *)c);
            os_free(node);
            if (foundNode != NULL)
            {
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
            ut_tableNode node = NULL;
            ut_tableNode foundNode = NULL;
            node = ut_newTableNode(o, NULL);
            
            foundNode = ut_avlTreeRemove (ut_table(c)->tree,
                                          node,
                                          ut_tableCmpWrapper, 
                                          (void *)c,
                                          NULL, /* removal is allowed */
                                          NULL);
            os_free(node);
            if (foundNode) {
                result = foundNode->value;
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
            ut_tableNode node = NULL;
            ut_tableNode foundNode = NULL;
            node = ut_newTableNode(o, NULL);
            
            foundNode = ut_avlTreeFind(ut_table(c)->tree,
                                       node,
                                       ut_tableCmpWrapper, 
                                       (void *)c);
            
            if (foundNode != NULL) {                            
                if (OS_EQ == ut_tableCmpWrapper(foundNode, node, c)) {
                    contains = 1;
                }
            }
            os_free(node);            
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
        count = ut_avlTreeCount(ut_table(c)->tree);
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
    os_int32 succes;
    
    assert(c);
    assert(action);
    
    succes = 0;
    
    switch (c->type) {
    case UT_TABLE :
        {
        OS_STRUCT(ut_tableWalkActionArg) args;

        args.tableActionFunc = action;
        args.actionArg = arg;
        args.collection = c;
        succes = ut_avlTreeWalk(ut_table(c)->tree,
                                ut_tableActionWrapper,
                                &args,
                                UT_PREFIX);
        }
    break;
    default :
        fprintf(stderr, "ut_walk: This collection type is not yet supported\n");
        assert(0);
        break;
    }
    
    return succes;
}

os_int32
ut_tableInsert(
    ut_table t,
    void *key,
    void *value)
{
    os_int32 returnValue;
    void *treeValue;
    ut_tableNode node;
    
    returnValue = 0;
    treeValue = NULL;
    node = NULL;
    
    assert(ut_collection(t)->type == UT_TABLE);
    
    node = ut_newTableNode(key, value);
    
    treeValue = ut_avlTreeInsert(ut_table(t)->tree, 
                                 node, 
                                 ut_tableCmpWrapper, 
                                 (void *)t);
                                   
    if (treeValue == node) {
        returnValue = 1;
    } else {
        os_free(node);
    }
    
    return returnValue;    
}

void *
ut_tableNext(
    ut_table table,
    void *o)
{
    ut_tableNode node = NULL;
    ut_tableNode nextNode = NULL;
    
    assert(table);
    
    node = ut_newTableNode(o, NULL);
    
    nextNode = ut_avlTreeNearest(ut_table(table)->tree,
                                 node,
                                 ut_tableCmpWrapper, 
                                 (void *)table,
                                 OS_GT);
    os_free(node);
                                       
    return nextNode->value;
}

void
ut_tableFree(
    ut_table table, 
    const ut_freeElementFunc freeKey, 
    void *freeKeyArg,
    const ut_freeElementFunc freeValue, 
    void *freeValueArg)
{
    os_int32 succes = 0;
    OS_STRUCT(ut_tableNodesCollection) allNodes;
    ut_tableNode currentNode = NULL;
    os_uint32 i = 0;
        
    /* collect all tableNodes in a array */
    allNodes.numberOfNodes = ut_avlTreeCount(table->tree);
    allNodes.tableNodes = os_malloc(((os_uint32)sizeof(ut_tableNode)) * allNodes.numberOfNodes);
    allNodes.nodeSeqNumber = 0;
    
    succes = ut_avlTreeWalk(table->tree,
                            ut_tableCollectNodes,
                            &allNodes,
                            UT_PREFIX);
    assert(succes);
    
    /* call ut_tableFreeNode for all elements in the array */
    for (i = 0; i < allNodes.numberOfNodes; i++) {
        currentNode = allNodes.tableNodes[i];
        if (freeKey) {
            freeKey(currentNode->key, freeKeyArg);
        } 
        
        if (freeValue) {
            freeValue(currentNode->value, freeKeyArg);
        } 
    }
    
    os_free(allNodes.tableNodes);
    ut_avlTreeFree(table->tree);
    os_free(table);
}
