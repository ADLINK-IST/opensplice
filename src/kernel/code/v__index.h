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
#ifndef V__INDEX_H
#define V__INDEX_H

#include "v_kernel.h"
#include "v_index.h"

#define v_indexKeyExpr(_this) \
        c_tableKeyExpr(v_index(_this)->objects)

v_index
v__indexNew(
    v_dataReader reader,
    q_expr _from,
    c_iter indexList,
    v_indexNewAction action,
    c_voidp arg);

void
v_indexInit(
     v_index _this,
     c_type instanceType,
     c_array sourceKeyList,
     v_reader reader);
     
#endif
