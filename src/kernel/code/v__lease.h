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

#ifndef V__LEASE_H
#define V__LEASE_H

#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define v_lease(o) (C_CAST(o,v_lease))

OS_API v_lease
v_leaseNew (
    v_leaseManager leaseManager,
    v_public p,
    v_duration leaseDuration,
    v_leaseActionId actionId,
    c_long repeatCount);

OS_API void
v_leaseDeinit (
    v_lease _this);

OS_API void
v_leaseRenew (
    v_lease _this,
    v_duration leaseDuration);

OS_API void
v_leaseUpdate (
    v_lease _this);

OS_API c_time
v_leaseExpiryTime (
    v_lease _this);

OS_API void
v_leaseAction(
    v_lease _this,
    c_time now);

OS_API v_gid
v_leaseGid (
    v_lease _this);

OS_API c_long
v_leaseDecRepeatCount(
    v_lease _this);

OS_API c_long
v_leaseRepeatCount(
    v_lease _this);

#undef OS_API
   
#if defined (__cplusplus)
}
#endif

#endif /* V__LEASE_H */
