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

#ifndef V__SUBSCRIBER_H
#define V__SUBSCRIBER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_subscriber.h"
#include "v_subscriberQos.h"
#include "v_reader.h"
#include "v_entity.h"
#include "v_group.h"
#include "v_event.h"
#include "v__transactionGroup.h"
#include "v__partitionAdmin.h"

#define v_subscriberPartitionCount(_this) v_partitionAdminCount(v_subscriber(_this)->partitions)

#define v_subscriberLock(_this) \
        c_mutexLock(&_this->mutex)

#define v_subscriberUnlock(_this) \
        c_mutexUnlock(&_this->mutex)

void
v_subscriberWalkReaders(
    v_subscriber _this,
    c_bool (*action)(v_reader reader, c_voidp arg),
    c_voidp arg);

void
v_subscriberNotifyDataAvailable(
    v_subscriber _this,
    v_event e);

c_bool
v_subscriberConnectNewGroup(
    v_subscriber s,
    v_group g);

v_reader
v_subscriberRemoveShareUnsafe(
    v_subscriber _this,
    v_reader reader);

v_reader
v_subscriberAddShareUnsafe(
    v_subscriber _this,
    v_reader r);

v_reader
v_subscriberRemoveShareUnsafe(
    v_subscriber _this,
    v_reader r);

void
v_subscriberLockShares(
    v_subscriber _this);

void
v_subscriberUnlockShares(
    v_subscriber _this);

/* This operation will return the subscriber's transactionGroup admin.
 * It will lazy create a new admin in case it didn't exist already.
 */
v_transactionGroupAdmin
v_subscriberLookupTransactionGroupAdmin(
    v_subscriber _this);

c_bool
v__subscriberRequireAccessLockCoherent(
    v_subscriber _this);

c_bool
v__subscriberRequireAccessLockOrdered(
    v_subscriber _this);

v_result
v_subscriberTestBeginAccess(
    v_subscriber _this);

void
v_subscriberLockAccess(
    v_subscriber _this);

c_bool
v_subscriberTryLockAccess(
    v_subscriber _this);

void
v_subscriberUnlockAccess(
    v_subscriber _this);

void
v_subscriberTriggerGroupCoherent(
    v_subscriber _this,
    v_reader owner);

void
v_subscriberNotifyGroupCoherentPublication(
    v_subscriber _this,
    v_message msg);

#if defined (__cplusplus)
}
#endif

#endif /* V__SUBSCRIBER_H */
