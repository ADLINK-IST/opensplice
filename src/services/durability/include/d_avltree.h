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
/** @file d_avltree.h
    @brief The interface of the AVL tree class.
*/

#ifndef D_AVLTREE_H
#define D_AVLTREE_H

#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

/** @def d_avlNode(node)
    @brief The cast wrapper for the class d_avlNode.
*/

#define d_avlNode(node) ((d_avlNode)(node))

/** @class d_avlNode
    @brief The class implementation.
*/
C_CLASS(d_avlNode);


/** @fn d_avlTreeFree(
            d_avlNode rootNode,
            void (*   cleanAction)() )

    @brief The d_avlTree destructor method.
           This method expects an empty tree, the cleanAction can do the
           garbage collection. if not empty references can
           be lost. The user is responsible for the garbage collection.
*/

/** @fn d_avlTreeInsert(
            d_avlNode * rootNodePntr,
            c_voidp     data,
            int (*      compareFunction)() )

    @brief Insert a node in the tree (The implicit d_avlTree constructor method).
           If the item is in the tree
           Then the existing item is returned
           Else it is added and NULL is returned.
*/

/** @fn d_avlTreeRemove (
            d_avlNode * rootNodePntr,
            c_voidp     data,
            int (*      compareFunction)() )

    @brief Removes a node from the tree, returns data or zero (if not found).
*/

/** @fn d_avlTreeTake (
            d_avlNode * rootNodePntr )

    @brief Takes a node from the tree, returns data or zero (if there was none).
*/

/** @fn d_avlTreeFind (
            d_avlNode rootNode,
            c_voidp   data,
            int (*    compareFunction)() )

    @brief Finds a node in the tree, returns data or zero (if not found).
*/

/** @fn d_avlTreeFirst (
            d_avlNode rootNode )

    @brief Finds the first node in the tree, returns data or zero (if there is no node).
*/

/** @fn d_avlTreeWalk (
            d_avlNode * root,
            c_bool (*   action) (),
            c_voidp     actionArgument )

    @brief Executes the action for every item in the tree.
           The item and the actionArgument are the parameters for the action.
           The walk stops if the action returns FALSE.
*/

void    d_avlTreeFree   ( d_avlNode   rootNode, void (* cleanAction)() );
c_voidp d_avlTreeInsert ( d_avlNode * rootNodePntr, c_voidp data, int (* compareFunction)() );
c_voidp d_avlTreeRemove ( d_avlNode * rootNodePntr, c_voidp data, int (* compareFunction)() );
c_voidp d_avlTreeTake   ( d_avlNode * rootNodePntr );
c_voidp d_avlTreeFind   ( d_avlNode   rootNode,     c_voidp data, int (* compareFunction)() );
c_voidp d_avlTreeFirst  ( d_avlNode   rootNode );
c_bool  d_avlTreeWalk   ( d_avlNode * root, c_bool (* action) (), c_voidp actionArgument );

#if defined (__cplusplus)
}
#endif

#endif /* D_AVLTREE_H */

