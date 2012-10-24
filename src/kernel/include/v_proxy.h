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
#ifndef V_PROXY_H
#define V_PROXY_H

#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_proxy</code> cast method.
 *
 * This method casts an object to a <code>v_proxy</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_proxy</code> or
 * one of its subclasses.
 */
#define v_proxy(_this) (C_CAST(_this,v_proxy))

OS_API v_proxy
v_proxyNew (
    v_kernel kernel,
    v_handle handle,
    c_voidp userData);

OS_API v_entity
v_proxyClaim (
    v_proxy _this);

OS_API void
v_proxyRelease (
    v_proxy _this);

#undef OS_API

#endif
