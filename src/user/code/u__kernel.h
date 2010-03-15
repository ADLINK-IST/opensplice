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

#ifndef U__KERNEL_H
#define U__KERNEL_H

#include "u_participant.h"
#include "u_kernel.h"
#include "os.h"

u_result
u_kernelClaim (
    u_kernel _this,
    v_kernel *kernel);

u_result
u_kernelRelease (
    u_kernel _this);

u_result
u_kernelAdd (
    u_kernel _this,
    u_participant p);

u_result
u_kernelRemove (
    u_kernel _this,
    u_participant p);

u_result
u_kernelDetachParticipants (
    u_kernel _this);

c_bool
u_kernelCheckHandleServer (
    u_kernel _this,
    c_long serverId);

os_sharedHandle
u_kernelSharedMemoryHandle (
    u_kernel kernel);

#endif

