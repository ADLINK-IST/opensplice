/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef V__SERVICEMANAGER_H
#define V__SERVICEMANAGER_H

#include "v_kernel.h"
#include "v_serviceManager.h"
#include "v_event.h"
/* This is only needed since cmxml component uses this protected header file! */

#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

void
v_serviceManagerNotify(
    v_serviceManager _this,
    v_event event,
    c_voidp userData);

v_serviceState
v_serviceManagerRegister(
    v_serviceManager _this,
    v_service service,
    const c_char *extStateName);

OS_API v_serviceState
v_serviceManagerGetServiceState(
    v_serviceManager _this,
    const c_char *serviceName);

#undef OS_API
 
#endif
