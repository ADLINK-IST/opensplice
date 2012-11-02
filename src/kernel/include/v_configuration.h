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
#ifndef V_CONFIGURATION_H
#define V_CONFIGURATION_H

/** \file kernel/include/v_configuration.h
 *  \brief This file defines the interface
 *
 */

#include "kernelModule.h"
#include "v_kernel.h"
#include "v_cfElement.h"
#include "v_cfNode.h"
#include "v_cfData.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_configuration</code> cast method.
 *
 * This method casts an object to a <code>v_configuration</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_configuration</code> or
 * one of its subclasses.
 */
#define v_configuration(o) (C_CAST(o,v_configuration))

OS_API v_configuration
v_configurationNew (
    v_kernel kernel);

OS_API void
v_configurationFree (
    v_configuration config);

OS_API void
v_configurationSetRoot (
    v_configuration config,
    v_cfElement root);

OS_API v_cfElement
v_configurationGetRoot (
    v_configuration config);

OS_API void
v_configurationSetUri (
    v_configuration config,
    const c_char *uri);

OS_API const c_char *
v_configurationGetUri (
    v_configuration config);

OS_API c_ulong
v_configurationIdNew (
    v_configuration config);


OS_API v_cfNode
v_configurationGetNode (
    v_configuration config,
    c_ulong id);

#undef OS_API

#endif /* V_CONFIGURATION_H */
