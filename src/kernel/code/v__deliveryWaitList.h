/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef V_DELIVERYWAITLIST_H
#define V_DELIVERYWAITLIST_H

/** \file kernel/code/v__deliveryWaitList.h
 *  \brief This file defines the interface
 *
 */

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "v__deliveryGuard.h"
#include "os_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define v_deliveryWaitList(o) (C_CAST(o,v_deliveryWaitList))

/* This method is called by the delivery guard to create a
 * WaitList for a specific thread.
 * A WaitList provides functionality to wait for a list of expected delivery
 * acknoledgements.
 */
v_deliveryWaitList
v_deliveryWaitListNew(
    v_deliveryGuard guard,
    v_message msg);

/* This method is called by the delivery guard to delete the
 * WaitList object.
 */
v_result
v_deliveryWaitListFree(
    v_deliveryWaitList _this);

/* This method will wait until the delivery of the given message is
 * acknowledged by all synchronous DataReaders.
 * This method will return:
 * - OK when delivery to all DataReaders is acknowledged.
 * - TIMEOUT when delivery is not acknowledged within the timeout
 *   period specified by the DataWriter reliability policy attribute
 *   max_blocking_time.
 * - an error code when the operation fails.
 */
v_result
v_deliveryWaitListWait (
    v_deliveryWaitList _this,
    os_duration timeout);

/* This method is called to pass a received delivery message to
 * the WaitList.
 * If all expected deliveries are acknoledged the thread
 * will be unblocked.
 */
v_result
v_deliveryWaitListNotify (
    v_deliveryWaitList _this,
    v_deliveryInfoTemplate msg);

/* This method is called to inform the WaitList to ignore waiting for a specific
 * systemId because the systemId has left the game.
 * If blocking on the specified systemId the thread will be unblocked.
 */
v_result
v_deliveryWaitListIgnore (
    v_deliveryWaitList _this,
    v_gid readerGID);

#if defined (__cplusplus)
}
#endif
#endif

