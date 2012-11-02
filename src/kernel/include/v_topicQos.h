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
#ifndef V_TOPICQOS_H
#define V_TOPICQOS_H

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
 * \brief The <code>v_topicQos</code> cast methods.
 *
 * This method casts an object to a <code>v_topicQos</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_topicQos</code> or
 * one of its subclasses.
 */
#define v_topicQos(o) (C_CAST(o,v_topicQos))

OS_API v_topicQos
v_topicQosNew (
    v_kernel kernel,
    v_topicQos template);

OS_API void
v_topicQosFree (
    v_topicQos q);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
