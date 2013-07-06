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

#ifndef V__PARTITIONADMIN_H
#define V__PARTITIONADMIN_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "kernelModule.h"
#include "v__partition.h"

#define v_partitionAdmin(o) (C_CAST(o,v_partitionAdmin))

v_partitionAdmin
v_partitionAdminNew(
    v_kernel kernel);

void
v_partitionAdminFree(
    v_partitionAdmin _this);

c_bool
v_partitionAdminFitsInterest(
    v_partitionAdmin _this,
    v_partition d);

c_iter
v_partitionAdminAdd(
    v_partitionAdmin _this,
    const c_char *partitionExpr);

c_iter
v_partitionAdminRemove(
    v_partitionAdmin _this,
    const c_char *partitionExpr);

c_bool
v_partitionAdminSet(
    v_partitionAdmin _this,
    v_partitionPolicy partitionExpressions,
    c_iter *addedPartitions,
    c_iter *removedPartitions);

c_bool
v_partitionAdminExists(
    v_partitionAdmin _this,
    const c_char *partitionName);

c_iter
v_partitionAdminLookup(
    v_partitionAdmin _this,
    const c_char *partitionExpr);

c_bool
v_partitionAdminWalk(
    v_partitionAdmin _this,
    c_action action,
    c_voidp arg);


#if defined (__cplusplus)
}
#endif

#endif /* V__PARTITIONADMIN_H */
