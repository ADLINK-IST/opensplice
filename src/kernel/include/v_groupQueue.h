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
#ifndef V_GROUPQUEUE_H
#define V_GROUPQUEUE_H

/** \file kernel/include/v_groupQueue.h
 *  \brief This file defines the interface
 *
 */

#include "kernelModule.h"

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
 * \brief The <code>v_groupQueue</code> cast method.
 *
 * This method casts an object to a <code>v_groupQueue</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_groupQueue</code> or
 * one of its subclasses.
 */
#define v_groupQueue(_this) (C_CAST(_this,v_groupQueue))

OS_API v_groupQueue
v_groupQueueNew(
    v_subscriber subscriber,
    const c_char* name,
    c_ulong size,
    v_readerQos qos);

OS_API void
v_groupQueueFree(
    v_groupQueue _this);

OS_API v_groupAction
v_groupQueueRead(
    v_groupQueue _this);

OS_API v_groupAction
v_groupQueueTake(
    v_groupQueue _this);

OS_API c_ulong
v_groupQueueSize(
    v_groupQueue _this);

OS_API void
v_groupQueueSetMarker(
    v_groupQueue _this);

OS_API void
v_groupQueueResetMarker(
    v_groupQueue _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /*V_GROUPQUEUE_H*/
