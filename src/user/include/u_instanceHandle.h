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
#ifndef U_INSTANCEHANDLE_H
#define U_INSTANCEHANDLE_H


#if defined (__cplusplus)
extern "C" {
#endif

#include "v_public.h"
#include "v_collection.h"
#include "os_if.h"

typedef c_longlong u_instanceHandle;

#define U_INSTANCEHANDLE_NIL (0)

#include "u_user.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API u_instanceHandle
u_instanceHandleNew(
    v_public object);

OS_API u_result
u_instanceHandleClaim (
    u_instanceHandle _this,
    c_voidp instance);

OS_API u_result
u_instanceHandleRelease (
    u_instanceHandle _this);

OS_API u_instanceHandle
u_instanceHandleFromGID (
    v_gid gid);

OS_API v_gid
u_instanceHandleToGID (
    u_instanceHandle _this);

OS_API c_bool
u_instanceHandleIsNil (
    u_instanceHandle _this);

OS_API c_bool
u_instanceHandleIsEqual (
    u_instanceHandle h1,
    u_instanceHandle h2);

/* Depricated : only for GID publication_handle legacy. */

OS_API u_instanceHandle
u_instanceHandleFix(
    u_instanceHandle _this,
    v_collection reader);

/* Depricated : only for DLRL legacy. */

OS_API c_long
u_instanceHandleServerId(
    u_instanceHandle _this);

OS_API c_long
u_instanceHandleIndex(
    u_instanceHandle _this);

OS_API c_long
u_instanceHandleSerial(
    u_instanceHandle _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
