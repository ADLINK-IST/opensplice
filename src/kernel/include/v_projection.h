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
#ifndef V_PROJECTION_H
#define V_PROJECTION_H

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
 * \brief The <code>v_projection</code> cast method.
 *
 * This method casts an object to a <code>v_projection</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_projection</code> or
 * one of its subclasses.
 */
#define v_projection(_this) (C_CAST(_this,v_projection))

/**
 * \brief The <code>v_mapping</code> cast method.
 *
 * This method casts an object to a <code>v_mapping</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_mapping</code> or
 * one of its subclasses.
 */
#define v_mapping(_this) (C_CAST(_this,v_mapping))

OS_API v_projection
v_projectionNew (
    v_dataReader reader,
    q_expr projection);

OS_API c_type
v_projectionType (
    v_projection _this);

OS_API c_field
v_projectionSource (
    v_projection _this,
    const c_char *fieldName);

OS_API c_array
v_projectionRules (
    v_projection _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
