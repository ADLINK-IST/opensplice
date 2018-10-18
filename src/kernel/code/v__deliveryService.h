/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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

v_deliveryService
v_deliveryServiceNew(
    v_subscriber subscriber,
    const c_char *name);

void
v_deliveryServiceFree(
    v_deliveryService _this);

v_result
v__deliveryServiceEnable(
    _Inout_ v_deliveryService _this);

void
v_deliveryServiceDeinit(
    v_deliveryService _this);

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
