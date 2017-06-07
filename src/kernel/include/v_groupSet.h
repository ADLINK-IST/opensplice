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
#ifndef V_GROUPSET_H
#define V_GROUPSET_H

/** \file kernel/include/v_groupSet.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_groupSet</code> cast method.
 *
 * This method casts an object to a <code>v_groupSet</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_groupSet</code> or
 * one of its subclasses.
 */
#define v_groupSet(_this) (C_CAST(_this,v_groupSet))

OS_API v_groupSet
v_groupSetNew (
    v_kernel kernel);

OS_API v_group
v_groupSetCreate (
    v_groupSet _this,
    v_partition partition,
    v_topic topic);

OS_API v_group
v_groupSetRemove (
    v_groupSet _this,
    v_group group);

OS_API c_iter
v_groupSetSelect (
    v_groupSet _this,
    c_char *expression,
    c_value params[]);

OS_API v_group
v_groupSetGet (
    v_groupSet _this,
    const c_char *partitionName,
    const c_char *topicName);

OS_API c_iter
v_groupSetLookup (
    v_groupSet _this,
    const c_char *partitionExpr,
    const c_char *topicExpr);

OS_API c_iter
v_groupSetSelectAll(
    v_groupSet _this);

OS_API c_bool
v_groupSetWalk (
    v_groupSet set,
    c_action action,
    c_voidp arg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

