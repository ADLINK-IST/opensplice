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
#ifndef V_WRITERQOS_H
#define V_WRITERQOS_H

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
 * \brief The <code>v_writerQos</code> cast methods.
 *
 * This method casts an object to a <code>v_writerQos</code> object.
 * Before the cast is performed, if the NDEBUG flag is not set,
 * the type of the object is checked to be <code>v_writerQos</code> or
 * one of its subclasses.
 */
#define v_writerQos(o) (C_CAST(o,v_writerQos))

OS_API v_writerQos
v_writerQosNew (
    v_kernel kernel,
    v_writerQos value);

OS_API void
v_writerQosFree (
    v_writerQos _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
