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
#ifndef V__GROUPSTORE_H
#define V__GROUPSTORE_H

#include "v_group.h"
#include "v_groupStore.h"

#if defined (__cplusplus)
extern "C" {
#endif

v_groupStore
v_groupStoreNew(
    const v_group group,
    const c_char *keyExpr,
    const c_array messageKeyList);

void
v_groupStoreDeinit(
    const v_groupStore _this);

void
v_groupStoreDelete(
    const v_groupStore _this,
    const v_groupInstance instance);

void
v_groupStoreDispose(
    const v_groupStore _this,
    const v_groupInstance instance);

void
v_groupStoreWalk(
    const v_groupStore _this,
    const c_action action,
    const c_voidp arg);

c_iter
v_groupStoreSelect(
    const v_groupStore _this,
    const os_uint32 max);

/**
 * \brief Set the specified flags in the instanceStates
 * of all dataReader instances associated with the specified group.
 *
 * \param group The group for which all instanceStates of all DataReaders must set the flags.
 * \param flags The flags to set for all instanceStates of the DataReaders.
 *
 * \remark The function is thread-safe. During the execution of the function access
 * to the group is locked.
 */
void
v_groupStoreMarkGroupInstanceStates (
    const v_groupStore _this,
    c_ulong flags);

/**
 * \brief Reset the specified flags in the instanceStates
 * of all dataReader instances associated with the specified group.
 *
 * \param group The group for which all instanceStates of all DataReaders must reset the flags.
 * \param flags The flags to reset for all instanceStates of the DataReaders.
 *
 * \remark The function is thread-safe. During the execution of the function access
 * to the group is locked.
 */
void
v_groupStoreUnmarkGroupInstanceStates (
    const v_groupStore _this,
    c_ulong flags);

c_array
v_groupStoreKeyList(
    const v_groupStore _this);

c_query
v_groupStore_create_query(
    const v_groupStore _this,
    const q_expr progExpr,
    const c_value *params);

v_groupInstance
v_groupStoreLookupInstance(
    const v_groupStore _this,
    const v_message msg);

/**
 * \brief Lookup or create an instance for the message key value.
 *
 * \param _this The group store which shall hold a matching instance after this operation.
 * \param msg   The message providing the key value of the desired instance.
 */
v_groupInstance
v_groupStoreCreateInstance(
    const v_groupStore _this,
    const v_message msg);

#if defined (__cplusplus)
}
#endif

#endif
