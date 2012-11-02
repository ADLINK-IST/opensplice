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
#ifndef V_PUBLISHERQOS_H
#define V_PUBLISHERQOS_H

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
 * \brief The <code>v_publisherQos</code> cast method.
 *
 * This method casts an object to a <code>v_publisherQos</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_publisherQos</code> or
 * one of its subclasses.
 */
#define v_publisherQos(o) (C_CAST(o,v_publisherQos))

OS_API v_publisherQos
v_publisherQosNew(
    v_kernel kernel,
    v_publisherQos template);
    
OS_API void
v_publisherQosFree(
    v_publisherQos q);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
