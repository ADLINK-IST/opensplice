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

#ifdef OSPL_BUILD_KERNEL
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

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

