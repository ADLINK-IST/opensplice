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
#ifndef V__KERNEL_H
#define V__KERNEL_H

#include "v_kernel.h"

#define v_kernelGetQos(_this) \
        (c_keep(_this->qos))

void
v_lockShares (
    v_kernel _this);

void
v_unlockShares (
    v_kernel _this);

v_entity
v_addShareUnsafe (
    v_kernel _this,
    v_entity e);

v_entity
v_removeShare (
    v_kernel _this,
    v_entity e);

c_iter
v_resolveShare (
    v_kernel _this,
    const c_char *name);

c_ulong
v_kernelGetTransactionId (
    v_kernel _this);

void
v_checkMaxSamplesPerInstanceWarningLevel(
    v_kernel _this,
    c_ulong count);

void
v_checkMaxSamplesWarningLevel(
    v_kernel _this,
    c_ulong count);

void
v_checkMaxInstancesWarningLevel(
    v_kernel _this,
    c_ulong count);

#endif /* V__KERNEL_H */
