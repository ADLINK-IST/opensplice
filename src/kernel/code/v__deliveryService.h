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
#ifndef V__DELIVERYSERVICE_H
#define V__DELIVERYSERVICE_H

#include "v_kernel.h"

/** \file kernel/code/v__deliveryService.h
 *  \brief This file defines the interface of AckReader objects.
 *
 */

/**
 * \brief The <code>v_deliveryService</code> cast method.
 *
 * This method casts an object to a <code>v_deliveryService</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_deliveryService</code> or
 * one of its subclasses.
 */
#define v_deliveryService(o) (C_CAST(o,v_deliveryService))

#define v_deliveryServiceLock(_this) \
        v_observerLock(v_deliveryService(_this))

#define v_deliveryServiceUnLock(_this) \
        v_observerUnlock(v_deliveryService(_this))

v_deliveryService
v_deliveryServiceNew(
    v_subscriber subscriber,
    const c_char *name);

void
v_deliveryServiceFree(
    v_deliveryService _this);

v_result
v_deliveryServiceEnable(
    v_deliveryService _this);

void
v_deliveryServiceDeinit(
    v_deliveryService _this);

c_bool
v_deliveryServiceSubscribe(
    v_deliveryService _this,
    v_partition partition);

c_bool
v_deliveryServiceUnSubscribe(
    v_deliveryService _this,
    v_partition partition);

#define v_deliveryServiceAddEntry(_this,entry) \
        v_deliveryServiceEntry(v_readerAddEntry(v_reader(_this),v_entry(entry)))

void
v_deliveryServiceRegister(
    v_deliveryService _this,
    v_message msg);

void
v_deliveryServiceUnregister(
    v_deliveryService _this,
    v_message msg);

v_writeResult
v_deliveryServiceWrite(
    v_deliveryService _this,
    v_deliveryInfoTemplate msg);

#endif
