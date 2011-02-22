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
#ifndef U_PARTITION_H
#define U_PARTITION_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_partition(o) \
        ((u_partition)u_entityCheckType(u_entity(o), U_PARTITION))

/* A u_partition object is a user proxy to the kernel v_partition object.
 * The constructor will lookup or else create a kernel v_partition object and
 * create a u_partition object as user proxy.
 */
OS_API u_partition
u_partitionNew (
    u_participant p,
    const c_char *name,
    v_partitionQos qos);

OS_API u_result
u_partitionInit (
    u_partition _this);

OS_API u_result
u_partitionFree (
    u_partition _this);

OS_API u_result
u_partitionDeinit (
    u_partition _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
