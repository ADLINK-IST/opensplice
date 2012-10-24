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
#ifndef V_CFATTRIBUTE_H
#define V_CFATTRIBUTE_H

/** \file kernel/include/v_cfAttribute.h
 *  \brief This file defines the interface
 *
 */

#if defined (__cplusplus)
extern "C" {
#endif

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
 * \brief The <code>v_cfAttribute</code> cast method.
 *
 * This method casts an object to a <code>v_cfAttribute</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_cfAttribute</code> or one of its subclasses.
 */
#define v_cfAttribute(o) (C_CAST(o,v_cfAttribute))

/**
 * \brief The <code>v_cfAttribute</code> constructor.
 *
 * Attributes are uniquely identified by their name.
 *
 * \param config the configuration it belongs too.
 * \param name   the name of the attribute
 * \param value  the attribute value
 * 
 * \return <code>NULL</code> if memory allocation fails, otherwise
 *         a reference to a newly instantiated configuration element.         
 */
OS_API v_cfAttribute
v_cfAttributeNew (
    v_configuration config,
    const c_char *name,
    c_value value);

/**
 * \brief The initialisation method to initialise the attributes of this class.
 *
 * \param attribute a reference to the configuration attribute object
 * \param config the configuration it belongs too.
 * \param name      the attribute name
 * \param value     the attribute value
 */
OS_API void
v_cfAttributeInit (
    v_cfAttribute attribute,
    v_configuration config,
    const c_char *name,
    c_value value);

/**
 * \brief Returns the attribute value.
 *
 * Every attribute has a value. With this method the value property
 * of an attribute can be requested.
 *
 * \param attribute a reference to the configuration attribute object
 * 
 * \return the attribute value.
 */
OS_API c_value
v_cfAttributeValue(
    v_cfAttribute attribute);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_CFATTRIBUTE_H */
