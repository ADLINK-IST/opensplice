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

/** @file c_avltree.h
    @brief The interface of the database AVL tree class.
*/

#ifndef C_AVLTREE_H
#define C_AVLTREE_H

#include "c_typebase.h"
#include "c_mmbase.h"

/** @def c_avlTree(tree)
    @brief The cast wrapper for the class c_avlTree.
*/
/** @def c_avlNode(node)
    @brief The cast wrapper for the class c_avlNode.
*/

#define c_avlTree(tree) ((c_avlTree)(tree))
#define c_avlNode(node) ((c_avlNode)(node))

/** @class c_avlNode
    @brief The class implementation.
*/
C_CLASS(c_avlNode);

C_STRUCT(c_avlNode) {
    c_avlNode left;    /* The element located at the left of the this tree element. */
    c_avlNode right;   /* The element located at the right of the this tree element. */
    c_short height;    /* The heigth of this element within the tree. */
};

/** @class c_avlTree
    @brief The class implementation.
*/
C_CLASS(c_avlTree);

C_STRUCT(c_avlTree) {
    c_avlNode root;    /* The root of the tree. */
    c_ulong offset;    /* The base offset of node information within tree elements. */
    c_ulong size;      /* The number of elements within the tree. */
    c_mm mm;           /* The memory management object associated to the tree. */
};

c_avlTree c_avlTreeNew   (c_mm mm, c_long offset);
void      c_avlTreeFree  (c_avlTree tree);
c_long    c_avlTreeCount (c_avlTree tree);
void     *c_avlTreeFirst (c_avlTree tree);
void     *c_avlTreeLast  (c_avlTree tree);
 
void     *c_avlTreeInsert (
               c_avlTree    tree,
               void        *node,
               c_equality (*compareFunction)(),
               void        *compareArgument);

void     *c_avlTreeReplace (
               c_avlTree    tree,
               void        *node,
               c_equality (*compareFunction)(),
               void        *compareArgument,
               c_bool     (*condition)(),
               void        *conditionArgument);

void     *c_avlTreeRemove (
               c_avlTree    tree,
               void        *templ,
               c_equality (*compareFunction)(),
               void        *compareArgument,
               c_bool     (*condition)(),
               void        *conditionArgument);

void     *c_avlTreeFind(
               c_avlTree    tree,
               void        *templ,
               c_equality (*compareFunction)(),
               void        *compareArgument);

void     *c_avlTreeNearest (
               c_avlTree    tree,
               void        *templ,
               c_equality (*compareFunction)(),
               void        *compareArgument,
               c_equality   specifier);

c_bool    c_avlTreeWalk(
               c_avlTree tree,
               c_bool  (*action) (),
               void     *actionArgument,
               c_fixType fix);

c_bool    c_avlTreeRangeWalk(
               c_avlTree    tree,
               void        *startTemplate,
               c_bool       startInclude,
               void        *endTemplate,
               c_bool       endInclude,
               c_equality (*compareFunction)(),
               void        *compareArgument,
               c_bool     (*action) (),
               void        *actionArgument,
               c_fixType    fix);

#endif /* C_AVLTREE_H */
