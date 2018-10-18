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

#ifndef V__PUBLISHER_H
#define V__PUBLISHER_H

#include "v_publisher.h"
#include "v_entity.h"
#include "v_group.h"
#include "v_event.h"

#define v_publisherIsSuspended(_this) \
        (!OS_TIMEE_ISINFINITE((_this)->suspendTime))

#define v_publisherTransactionId(_this) \
        (v_publisher(_this)->transactionId)

void
v_publisherAssertLiveliness(
    v_publisher _this,
    v_event e);

c_bool
v_publisherConnectNewGroup(
    v_publisher _this,
    v_group g);

c_bool
v_publisherStartTransaction(
    v_publisher p,
    c_ulong *publisherId,
    c_ulong *transactionId);

#endif
