/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#ifndef V__PARTITIONADMIN_H
#define V__PARTITIONADMIN_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "kernelModuleI.h"
#include "v__partition.h"

#define v_partitionAdmin(o) (C_CAST(o,v_partitionAdmin))

v_partitionAdmin
v_partitionAdminNew(
    v_kernel kernel);

void
v_partitionAdminFree(
    v_partitionAdmin _this);

void
v_partitionAdminFill(
    v_partitionAdmin _this,
    const c_char *partitionExpr);

c_bool
v_partitionAdminAdd(
    v_partitionAdmin _this,
    v_partition d);

c_bool
v_partitionAdminUpdate(
    v_partitionAdmin _this,
    v_partitionPolicyI partitionExpressions,
    c_iter *addedPartitions,
    c_iter *removedPartitions);

c_iter
v_partitionAdminLookup(
    v_partitionAdmin _this,
    const c_char *partitionExpr);

c_bool
v_partitionAdminWalk(
    v_partitionAdmin _this,
    c_action action,
    c_voidp arg);

c_ulong
v_partitionAdminCount(
    v_partitionAdmin _this);

#if defined (__cplusplus)
}
#endif

#endif /* V__PARTITIONADMIN_H */
