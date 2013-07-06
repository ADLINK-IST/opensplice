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

