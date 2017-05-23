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
#ifndef V__NETWORKQUEUE_H
#define V__NETWORKQUEUE_H

#include "v_networkQueue.h"

#define v_networkQueue(o) (C_CAST(o,v_networkQueue))

#define v_networkQueueResolution(_this) \
        v_networkQueue(_this)->resolution

#define v_networkQueuePriority(_this) \
        v_networkQueue(_this)->priority

#define v_networkQueueP2P(_this) \
        v_networkQueue(_this)->P2P

#define v_networkQueueReliable(_this) \
        v_networkQueue(_this)->reliable

typedef c_bool
(*v_networkQueueAction)(
    v_networkQueueSample sample,
    c_voidp arg);

c_bool
v_networkQueueTakeAction(
    v_networkQueue queue,
    v_networkQueueAction action,
    c_voidp arg);

#endif /* V__NETWORKQUEUE_H */

