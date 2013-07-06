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

v_result
v_subscriberSetQos(
    v_subscriber _this,
    v_subscriberQos qos);

void
v_subscriberNotify(
    v_subscriber _this,
    v_event event);

void
v_subscriberConnectNewGroup(
    v_subscriber s,
    v_group g);

v_reader
v_subscriberRemoveShare(
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

#if defined (__cplusplus)
}
#endif

#endif /* V__SUBSCRIBER_H */
