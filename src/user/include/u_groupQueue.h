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
#ifndef U_GROUPQUEUE_H
#define U_GROUPQUEUE_H

#include "u_types.h"
#include "u_reader.h"
#include "v_readerQos.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_groupQueue(o) \
        ((u_groupQueue)u_entityCheckType(u_entity(o), U_GROUPQUEUE))

OS_API u_groupQueue
u_groupQueueNew (
    u_subscriber s,
    const c_char *name,
    c_ulong queueSize,
    v_readerQos qos);
                                             
OS_API u_result
u_groupQueueInit (
    u_groupQueue _this,
    u_subscriber s);

OS_API u_result
u_groupQueueFree (
    u_groupQueue _this);

OS_API u_result
u_groupQueueDeinit (
    u_groupQueue _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /*U_GROUPQUEUE_H*/
