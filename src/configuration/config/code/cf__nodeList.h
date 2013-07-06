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

#ifndef CF__NODELIST_H
#define CF__NODELIST_H

#include "cf_node.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define cf_nodeList(o) ((cf_nodeList)(o))
C_CLASS(cf_nodeList);
C_STRUCT(cf_nodeList) {
    int maxNrNodes;
    int nrNodes;
    cf_node *theList;
};

typedef void *cf_nodeWalkActionArg;
typedef unsigned int (*cf_nodeWalkAction)(/*c_object, cf_nodeWalkActionArg*/);

cf_nodeList
cf_nodeListNew();
void
cf_nodeListFree(
    cf_nodeList list);

void
cf_nodeListClear(
    cf_nodeList list);
c_object
cf_nodeListInsert(
    cf_nodeList list,
    cf_node node);
c_bool
cf_nodeListWalk(
    cf_nodeList list,
    cf_nodeWalkAction action,
    cf_nodeWalkActionArg arg);

#if defined (__cplusplus)
}
#endif

#endif /* CF__NODELIST_H */
