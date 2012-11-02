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
#ifndef UT__TABLE_H
#define UT__TABLE_H

#include "ut_collection.h"
#include "ut__avltree.h"


#if defined (__cplusplus)
extern "C" {
#endif

#define ut_tableNode(node) ((ut_tableNode) (node))

OS_CLASS(ut_tableNode);

OS_STRUCT(ut_tableNode) {
    OS_EXTENDS(ut_avlNode); /* tableNode is a specialized avlNode */
    void *key; /* identifier for this node */
    void *value; /* value of this node */
};

#if defined (__cplusplus)
}
#endif

#endif /* UT__TABLE_H */
