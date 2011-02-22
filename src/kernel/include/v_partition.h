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
#ifndef V_PARTITION_H
#define V_PARTITION_H

/** \file kernel/include/v_partition.h
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
 * \brief The <code>v_partition</code> cast method.
 *
 * This method casts an object to a <code>v_partition</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_partition</code> or
 * one of its subclasses.
 */
#define v_partition(_this) (C_CAST(_this,v_partition))

#define v_partitionName(_this) (v_entityName(v_partition(_this)))

OS_API v_partition
v_partitionNew (
    v_kernel kernel,
    const c_char *name,
    v_partitionQos qos);

OS_API void
v_partitionFree (
    v_partition _this);

OS_API void
v_partitionDeinit (
    v_partition _this);

OS_API c_iter
v_partitionLookupPublishers (
    v_partition _this);

OS_API c_iter
v_partitionLookupSubscribers (
    v_partition _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
