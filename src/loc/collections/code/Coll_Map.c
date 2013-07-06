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
/*  C includes */
#include <assert.h>

/*  DCPS includes */
#include "os_heap.h"

/*  DLRL includes */
#include "DLRL_Report.h"
#include "Coll_Map.h"


struct Coll_MapNode_s {
    void         *_key;
    void         *_object;
    Coll_MapNode *_lchild;
    Coll_MapNode *_rchild;
};

typedef struct {
    void *_key;
    void *_new_object;
    void *_old_object;
    void *_original_key_pointer;
    int  _replace;
    int  (*_isLess)(void *left, void *right);
} Coll_MapHelper;

/*  local function prototypes */
static long
Coll_Map_insertInTree(
    Coll_MapNode **tree,
    Coll_MapHelper *helper);

static void
Coll_Map_deleteMinimum(
    Coll_MapNode **tree,
    Coll_MapNode **minimum);

static void *
Coll_Map_deleteFromTree(
    Coll_MapNode **tree,
    Coll_MapHelper *helper);

static void *
Coll_Map_findInTree(
    Coll_MapNode **tree,
    Coll_MapHelper *helper);

Coll_Map *
Coll_Map_new(
    int (* const isLess)(void *left, void *right))
{
    Coll_Map *_this;

    DLRL_INFO(INF_ENTER);

    assert(isLess);

    _this = os_malloc(sizeof(Coll_Map));

    if (_this){
        Coll_Map_init(_this, isLess);
    }
    DLRL_INFO(INF_EXIT);
    return _this;
}

void
Coll_Map_init(
    Coll_Map *_this,
    int (* const isLess)(void *left, void *right))
{
    DLRL_INFO(INF_ENTER);

    assert(_this);
    assert(isLess);

    _this->_nr_elements = 0;
    _this->_isLess = isLess;
    _this->_tree  = NULL;
    DLRL_INFO(INF_ENTITY, "created Map, address = %p", _this);

    DLRL_INFO(INF_EXIT);
}

long
Coll_Map_delete(
    Coll_Map *_this)
{
    long retValue = COLL_ERROR_NOT_EMPTY;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    /*  only allowed if the map is empty */
    if(_this->_nr_elements == 0){
        /*  free the list header */
        os_free(_this);
        retValue = COLL_OK;
        DLRL_INFO(INF_ENTITY, "deleting Map, address = %p", _this);
    }
    DLRL_INFO(INF_EXIT);
    return retValue;
}

long
Coll_Map_getNrOfElements(
    Coll_Map *_this)
{
    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);

    DLRL_INFO(INF_EXIT);
    return _this->_nr_elements;
}

long
Coll_Map_insertInTree(
    Coll_MapNode **tree,
    Coll_MapHelper *helper)
{
    long result;

    DLRL_INFO(INF_ENTER);

    assert(tree);
    assert(helper);

    if (*tree == NULL){
        /*  end of tree found => insert the new element here */
        *tree = os_malloc(sizeof(struct Coll_MapNode_s));
        if (tree){
            (*tree)->_key    = helper->_key;
            (*tree)->_object = helper->_new_object;
            (*tree)->_lchild = NULL;
            (*tree)->_rchild = NULL;
            result = COLL_OK;
        } else {
            result = COLL_ERROR_ALLOC;
        }
    } else {
        if (helper->_isLess(helper->_key, (*tree)->_key) ){
            /*  new key is smaller => insert to the left */
            result = Coll_Map_insertInTree( &(*tree)->_lchild, helper);
        } else if (helper->_isLess( (*tree)->_key, helper->_key) ){
            /*  key in tree is smaller => insert to the right */
            result = Coll_Map_insertInTree( &(*tree)->_rchild, helper);
        } else {
            /*  key is equal */
            if (helper->_replace){
                helper->_old_object = (*tree)->_object;
                (*tree)->_object = helper->_new_object;
                result = COLL_OK;
            } else {
                result = COLL_ERROR_ALREADY_EXISTS;
            }
        }
    }
    DLRL_INFO(INF_EXIT);
    return result;
}

void
Coll_Map_deleteMinimum(
    Coll_MapNode **tree,
    Coll_MapNode **minimum)
{
    DLRL_INFO(INF_ENTER);

    assert(tree);
    assert(minimum);

    if ( !(*tree)->_lchild){
        /*  minimum found */
        /* Coll_MapNode node_to_delete = *tree; */

        *minimum = *tree;
        *tree = (*tree)->_rchild;
    } else{
        /*  continue search */
        Coll_Map_deleteMinimum( &(*tree)->_lchild, minimum);
    }
    DLRL_INFO(INF_EXIT);
}

void *
Coll_Map_deleteFromTree(
    Coll_MapNode **tree,
    Coll_MapHelper *helper)
{
    void *removed_object = NULL;
    Coll_MapNode *minimum;
    Coll_MapNode *node_to_delete;

    DLRL_INFO(INF_ENTER);

    assert(tree);
    assert(helper);

    if (*tree){
        if (helper->_isLess(helper->_key, (*tree)->_key) ){
            /*  searched key is smaller => delete from the left subtree */
            removed_object = Coll_Map_deleteFromTree( &(*tree)->_lchild, helper);
        } else if (helper->_isLess( (*tree)->_key, helper->_key) ){
            /*  searched key is larger => delete from the right subtree */
            removed_object = Coll_Map_deleteFromTree( &(*tree)->_rchild, helper);
        } else {
            /*  searched key equals the current tree top key => remove the treetop */
            node_to_delete = *tree;

            if ( !(*tree)->_lchild){
                /*  replace the current treetop with the right subtreetop */
                (*tree) = (*tree)->_rchild;
            } else if ( !(*tree)->_rchild){
                /*  replace the current treetop with the left subtreetop */
                (*tree) = (*tree)->_lchild;
            } else {
                /*  two subtrees exist => replace the treetop with the minimum of the right subtree */
                /*  declare a pointer for the minimum in the right subtree */


                Coll_Map_deleteMinimum( &((*tree)->_rchild), &minimum);
                minimum->_lchild = (*tree)->_lchild;
                minimum->_rchild = (*tree)->_rchild;
                *tree = minimum;
            }
            /*  store the removed object pointer */
            removed_object = node_to_delete->_object;

            /*  store the removed key */
            helper->_original_key_pointer = node_to_delete->_key;

            /*  finally free the memory used by the node; */
            os_free(node_to_delete);
        }
    }
    DLRL_INFO(INF_EXIT);
    return removed_object;
}

void *
Coll_Map_findInTree(
    Coll_MapNode **tree,
    Coll_MapHelper *helper)
{
    void *retValue = NULL;

    DLRL_INFO(INF_ENTER);

    assert(tree);
    assert(helper);

    if (*tree){
        if (helper->_isLess(helper->_key, (*tree)->_key) ){
            /*  searched key is smaller => continue search in the left subtree */
            retValue = Coll_Map_findInTree( &(*tree)->_lchild, helper);
        } else if (helper->_isLess( (*tree)->_key, helper->_key) ){
            /*  searched key is larger => continue search in the right subtree */
            retValue = Coll_Map_findInTree( &(*tree)->_rchild, helper);
        } else {
            /*  searched key equals the current tree top key => return its object */
            retValue = (*tree)->_object;
        }
    }
    DLRL_INFO(INF_EXIT);
    return retValue;
}

long
Coll_Map_add(
    Coll_Map *_this,
    void *key,
    void *new_object,
    void **old_object)
{
    long result;
    Coll_MapHelper helper;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(key);
    /* new_object & old_object may be null */


    helper._key         = key;
    helper._new_object  = new_object;
    helper._old_object  = NULL;
    helper._isLess      = _this->_isLess;
    helper._replace     = (old_object) ? 1 : 0;

    result = Coll_Map_insertInTree( &(_this->_tree), &helper);

    if (result == COLL_OK) {
        if (helper._replace && helper._old_object){
            /*  object replacement */
            *old_object = helper._old_object;
        } else {
            /*  new object insertion */
            _this->_nr_elements++;
        }
    }
    DLRL_INFO(INF_EXIT);
    return result;
}

void *
Coll_Map_remove(
    Coll_Map *_this,
    void *key,
    void **stored_key)
{
    void *removed_object = NULL;
    Coll_MapHelper helper;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(key);
    /* stored key may be null */

    helper._key                  = key;
    helper._new_object           = NULL;
    helper._original_key_pointer = NULL;
    helper._isLess               = _this->_isLess;

    removed_object = Coll_Map_deleteFromTree( &(_this->_tree), &helper);
    if (removed_object){
        _this->_nr_elements--;
        if (stored_key){
            *stored_key = helper._original_key_pointer;
        }
    }
    DLRL_INFO(INF_EXIT);
    return removed_object;
}

void *
Coll_Map_get(
    Coll_Map *_this,
    void *key)
{
    void * retVal = NULL;
    Coll_MapHelper helper;

    DLRL_INFO_OBJECT(INF_ENTER);

    assert(_this);
    assert(key);

    helper._key         = key;
    helper._new_object  = NULL;
    helper._isLess     = _this->_isLess;

    retVal = Coll_Map_findInTree( &(_this->_tree), &helper);
    DLRL_INFO(INF_EXIT);
    return retVal;
}
