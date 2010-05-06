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
#include "u_entity.h"
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

u_kernel
u_kernelNew (
    const c_char *uri);

u_kernel
u_kernelOpen (
    const c_char *uri,
    c_long timeout); /* timeout in seconds */

v_kernel
u_kernelSource (
    u_kernel _this);

u_result
u_kernelClose (
    u_kernel _this);

u_result
u_kernelFree (
    u_kernel _this);

os_sharedHandle
u_kernelSharedMemoryHandle (
    u_kernel kernel);

c_voidp
u_kernelGetCopy (
    u_kernel _this,
    u_entityCopy copy,
    void* copyArg);

c_address
u_kernelHandleServer(
    u_kernel _this);

c_voidp
u_kernelAddress(
    u_kernel _this);

#endif
