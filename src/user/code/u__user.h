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
#ifndef U__USER_H
#define U__USER_H

#include "u_user.h"
#include "u_kernel.h"

#define OSRPT_CNTXT_USER "user layer"

typedef c_voidp u_kernelActionArg;
typedef void (*u_kernelAction)(u_kernel kernel, u_kernelActionArg arg);

#define u_resultFromKernel(r) ((u_result)r)

u_kernel
u_userKernelNew (
    const c_char *uri);

u_kernel
u_userKernelOpen (
    const c_char *uri,
    c_long timeout);

u_result
u_userKernelClose (
    u_kernel kernel);

u_result
u_userKernelFree (
    u_kernel kernel);

c_address
u_userServer (
    c_long id);

c_long
u_userServerId (
    v_public o);

c_long
u_userProtectCount ();

#endif

