/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
