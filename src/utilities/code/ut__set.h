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
#ifndef UT__SET_H
#define UT__SET_H

#include "ut_collection.h"
#include "ut__avltree.h"


#if defined (__cplusplus)
extern "C" {
#endif

#define ut_setNode(node) ((ut_setNode) (node))

OS_CLASS(ut_setNode);

OS_STRUCT(ut_setNode) {
    OS_EXTENDS(ut_avlNode); /* tableNode is a specialized avlNode */
    void *value; /* value of this node */
};

#if defined (__cplusplus)
}
#endif

#endif /* UT__SET_H */
