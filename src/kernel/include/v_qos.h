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
#ifndef V_QOS_H
#define V_QOS_H

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
 * \brief The <code>v_qos</code> cast method.
 *
 * This method casts an object to a <code>qos</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>qos</code> or
 * one of its subclasses.
 */
#define v_qos(o) (C_CAST(o,v_qos))

typedef c_ulong v_qosChangeMask; /* mask indicating, which policies have changed */

OS_API v_qos
v_qosCreate (
    v_kernel kernel,
    v_qosKind kind);

OS_API void
v_qosFree (
    v_qos _this);

OS_API const c_char *
v_qosKindImage (
    v_qosKind kind);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_QOS_H */
