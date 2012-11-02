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
#ifndef v_cfData_H
#define v_cfData_H

/**
 * \file kernel/include/v_cfData.h
 * \brief This file defines the interface of the kernel configuration data 
 *        node.
 *
 * 
 */

#if defined (__cplusplus)
extern "C" {
#endif

#define V_CFDATANAME "#text"

/**
 * \brief The <code>v_cfData</code> cast method.
 *
 * This method casts an object to a <code>v_cfData</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_cfData</code> or one of its subclasses.
 */
#define v_cfData(o) (C_CAST(o,v_cfData))

#include "kernelModule.h"
#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_cfData</code> constructor.
 *
 * This method calls <code>v_cfDataInit</code> to initialise the attributes
 * of this class.
 *
 * Supported value arguments:
 * <code>V_BOOLEAN</code>, <code>V_OCTET</code>, <code>V_SHORT</code>,
 * <code>V_LONG</code>, <code>V_LONGLONG</code>, <code>V_USHORT</code>,
 * <code>V_ULONG</code>, <code>V_ULONGLONG</code>, <code>V_FLOAT</code>,
 * <code>V_DOUBLE</code>, <code>V_CHAR</code> and <code>V_STRING</code>.
 * 
 * \param config the configuration it belongs to.
 * \param value  this argument contains the data
 *
 * \return <code>NULL</code> if memory allocation fails or the value argument
 *         is not supported, otherwise
 *         a reference to the newly instantiated configuration data node.
 */
OS_API v_cfData
v_cfDataNew (
    v_configuration config,
    c_value value);

/**
 * \brief The initialisation method to initialise the attributes of this class.
 *
 * The given value argument is copied into shared memory, so the given value
 * argument may be freed after this call.
 *
 * \param data   the configuration data object to operate on
 * \param config the configuration it belongs to.
 * \param value  this argument contains the data
 */
OS_API void
v_cfDataInit (
    v_cfData data,
    v_configuration config,
    c_value value);

/**
 * \brief Returns the data as a value object.
 *
 * The returned value object can contain references to shared memory and
 * may therefore never be altered! 
 * 
 * \param data the configuration data object to operate on
 *
 * \return the data.
 */
OS_API c_value
v_cfDataValue(
    v_cfData data);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* v_cfData_H */
