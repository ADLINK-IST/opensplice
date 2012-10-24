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
#ifndef U_HANDLE_H
#define U_HANDLE_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "v_public.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef c_longlong u_handle;

OS_API extern const u_handle U_HANDLE_NIL;

OS_API u_handle
u_handleNew(
    v_public object);

OS_API u_result
u_handleClaim (
    u_handle _this,
    c_voidp instance);

OS_API u_result
u_handleRelease (
    u_handle _this);

OS_API c_bool
u_handleIsNil (
    u_handle _this);

OS_API c_bool
u_handleIsEqual (
    u_handle h1,
    u_handle h2);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
