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
#ifndef V_PARTICIPANTQOS_H
#define V_PARTICIPANTQOS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_participantQos</code> cast method.
 *
 * This method casts an object to a <code>v_participantQos</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_participantQos</code> or
 * one of its subclasses.
 */
#define v_participantQos(o) (C_CAST(o,v_participantQos))

OS_API v_participantQos
v_participantQosNew(
    v_kernel kernel,
    v_participantQos template);
    
OS_API void
v_participantQosFree(
    v_participantQos q);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_PARTICIPANTQOS_H */
