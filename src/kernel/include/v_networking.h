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
#ifndef V_NETWORKING_H
#define V_NETWORKING_H

#include "kernelModule.h"
#include "v_participantQos.h"

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
 * \brief The <code>v_networking</code> cast method.
 *
 * This method casts an object to a <code>v_networking</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_networking</code> or
 * one of its subclasses.
 */
#define v_networking(o) (C_CAST(o,v_networking))

OS_API v_networking
v_networkingNew(
    v_serviceManager manager,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos);

OS_API void
v_networkingFree(
    v_networking nw);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
