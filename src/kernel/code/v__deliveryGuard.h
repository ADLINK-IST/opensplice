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
#ifndef V_DELIVERYGUARD_H
#define V_DELIVERYGUARD_H

/** \file kernel/code/v__deliveryGuard.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "v__deliveryService.h"
#include "os_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* A Delivery Guard class implements 'wait for message delivery acknoledgement'
 * It provides v_writer objects a mechanism to wait for message delivery.
 */

#define v_deliveryGuard(o) (C_CAST(o,v_deliveryGuard))
#define v_deliveryPublisher(o) (C_CAST(o,v_deliveryPublisher))

/* This method is called by the delivery service to create a
 * guard for a specific DataWriter.
 * A Guard provides functionality to wait for delivery
 * acknoledgements from all associated synchronous DataReaders.
 */
v_deliveryGuard
v_deliveryGuardNew(
    v_deliveryService _this,
    v_writer writer);

/* This method is called by the delivery service to delete the
 * Guard object.
 */
v_result
v_deliveryGuardFree(
    v_deliveryGuard _this);

/* This method is called to pass a received delivery message to
 * the guard.
 * All the blocking threads are notified about this delivery message.
 * If all expected deliveries for a specific blocking thread have been
 * acknoledged the thread will be unblocked.
 */
v_result
v_deliveryGuardNotify(
    v_deliveryGuard _this,
    v_deliveryInfoTemplate msg);

/* This method is called to inform the guard to ignore waiting for a specific
 * systemId because the systemId has left the game.
 * All the blocking threads are notified about this ignore message.
 * If threads are blocking on the specified systemId they will be unblocked.
 */
v_result
v_deliveryGuardIgnore(
    v_deliveryGuard _this,
    v_gid readerGID);

#if defined (__cplusplus)
}
#endif
#endif

