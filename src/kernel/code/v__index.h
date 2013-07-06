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
