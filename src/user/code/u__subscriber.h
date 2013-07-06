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

#ifndef U__SUBSCRIBER_H
#define U__SUBSCRIBER_H

#include "u_subscriber.h"

u_result
u_subscriberInit (
    u_subscriber _this,
    u_participant participant);

u_result
u_subscriberDeinit (
    u_subscriber _this);

#endif
