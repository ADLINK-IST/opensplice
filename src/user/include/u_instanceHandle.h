/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef U_INSTANCEHANDLE_H
#define U_INSTANCEHANDLE_H


#if defined (__cplusplus)
extern "C" {
#endif

#include "u_handle.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_instanceHandle               u_handle
#define u_instanceHandleClaim(h,i)     u_handleClaim((h),(i))
#define u_instanceHandleRelease(h)     u_handleRelease((h))
#define u_instanceHandleIsEqual(h1,h2) u_handleIsEqual((h1),(h2))
#define u_instanceHandleIsNil(h)       u_handleIsNil(h)
#define u_instanceHandleSetNil(h)      u_handleSetNil(h)

OS_API c_long
u_instanceHandleSampleCount(
    u_instanceHandle _this);

OS_API v_state
u_instanceHandleState(
    u_instanceHandle _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
