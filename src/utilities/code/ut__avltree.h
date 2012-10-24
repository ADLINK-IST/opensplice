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
#ifndef UT__AVLTREE_H
#define UT__AVLTREE_H

#include "ut_avltree.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define ut_avlNode(node) ((ut_avlNode)(node))

OS_CLASS(ut_avlNode);
OS_STRUCT(ut_avlNode) {
    ut_avlNode left;    /* The element located at the left of the this tree element. */
    ut_avlNode right;   /* The element located at the right of the this tree element. */
    os_uint32 height;    /* The heigth of this element within the tree. */
};

OS_STRUCT(ut_avlTree) {
    ut_avlNode root;    /* The root of the tree. */
    os_uint32 offset;    /* The base offset of node information within tree elements. */
    os_uint32 size;      /* The number of elements within the tree. */
};

#if defined (__cplusplus)
}
#endif

#endif /* UT__AVLTREE_H */
