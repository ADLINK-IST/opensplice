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
#ifndef V__DELIVERYSERVICE_H
#define V__DELIVERYSERVICE_H

#include "v_kernel.h"

/** \file kernel/code/v__deliveryService.h
 *  \brief This file defines the interface of the Kernels Delivery Service.
 *
 * The Delivery Service implements a synchronous delivery protocol.
 * This Service implements a mechanism to send and receive delivery
 * acknoledgement.
 * When a synchronous DataReader receives a message from a synchronous
 * DataWriter it will acknowledge the message by calling the method
 * v_deliveryServiceAckMessage(). This method publishes a delivery message
 * via the builtin Delivery Topic DataWriter to acknowledeg the message.
 * The Delivery Service on the node of the DataWriter that has sent the message
 * will receive the delivery acknoledgement message and notify the DataWriter
 * that the sent message has been delivered.
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

/* the following v_deliveryServiceSubscribe and v_deliveryServiceUnSubscribe
 * methods are defined by the v_reader interface and are required to
 * establish connectivity to communication partitions.
 */
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

/* The v_deliveryServiceRegister method is called by the spliced to pass received
 * builtin subscription messages. The Delivery Service will use this information
 * to update the 'connected synchronous DataReader' list of the synchronous
 * DataWriters. Each synchronous DataWriter has a v_deliveryGuard known by the
 * Delivery Service that contains the list of all associated synchronous
 * DataReaders.
 */
void
v_deliveryServiceRegister(
    v_deliveryService _this,
    v_message msg);

/* The v_deliveryServiceUnregister method is called by the spliced to pass
 * received builtin subscription (dispose) messages.
 * The Delivery Service will use this information to update the
 * 'connected synchronous DataReader' list of the synchronous DataWriters.
 * Each synchronous DataWriter has a v_deliveryGuard known by the
 * Delivery Service that contains the list of all associated synchronous
 * DataReaders.
 */
void
v_deliveryServiceUnregister(
    v_deliveryService _this,
    v_message msg);

/* The v_deliveryServiceWrite method is called to deliver a delivery acknoledgemnt
 * message. The Service will find all waiting DataWriters (v_deliveryWaitLists)
 * and remove the DataReader identified by the message from the wait lists.
 * When a waitlist becomes empty (no DataReaders more to wait for) the DataWriter
 * will be unblocked.
 */
v_writeResult
v_deliveryServiceWrite(
    v_deliveryService _this,
    v_deliveryInfoTemplate msg);

/* The v_deliveryServiceAckMessage method is called by synchronous DataReaders
 * to acknowledge the delivery of a synchronous message.
 * This method will use the kernels Delivery Topic builtin DataWriter to
 * publish a Delivery message.
 */
v_writeResult
v_deliveryServiceAckMessage (
    v_deliveryService _this,
    v_message message,
    v_gid gid);

/* The v_deliveryServiceRemoveGuard method is called when a v_deliveryGuard
 * is freed: it removes itself from the v_deliveryService guard-list.
 * The guard-list is locked by this operation.
 */
v_result
v_deliveryServiceRemoveGuard(
    v_deliveryService _this,
    v_deliveryGuard guard);

/* The v_deliveryServiceAddGuard method is called when a v_deliveryGuard is
 * created: it adds itself to the v_deliveryService guard-list.
 * The guard-list is locked by this operation.
 */
v_result
v_deliveryServiceAddGuard(
    v_deliveryService _this,
    v_deliveryGuard guard);

/* The v_derliverServiceLookupGuard method can be called to lookup a guard in
 * the guard-list of the v_deliveryService. The lookup is done based on the
 * writerGID field of the supplied v_deliveryGuard 'template'.
 */
v_deliveryGuard
v_deliveryServiceLookupGuard(
    v_deliveryService _this,
    v_deliveryGuard template);

#endif
