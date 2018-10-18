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

v_result
v__subscriberEnable(
    _Inout_ v_subscriber _this);

void
v_subscriberWalkReaders(
    v_subscriber _this,
    c_action action,
    c_voidp arg);

void
v_subscriberNotifyDataAvailable(
    v_subscriber _this,
    v_event e);

c_bool
v_subscriberConnectNewGroup(
    v_subscriber s,
    v_group g);

v_dataReader
v_subscriberAddShare(
    _Inout_ v_subscriber _this,
    _In_ v_dataReader reader);

c_ulong
v_subscriberRemoveShare(
    v_subscriber _this,
    v_dataReader reader);

void
v_subscriberGroupTransactionFlush(
    v_subscriber _this);

void
v_subscriberNotifyGroupCoherentPublication(
    v_subscriber _this,
    v_message msg);

void
v_subscriberNotify(
    v_subscriber _this,
    v_event event,
    c_voidp userData);
#if defined (__cplusplus)
}
#endif

#endif /* V__SUBSCRIBER_H */
