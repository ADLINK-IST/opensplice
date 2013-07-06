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

#ifndef U__PUBLISHER_H
#define U__PUBLISHER_H

#include "u_publisher.h"

u_result
u_publisherAddWriter (
    u_publisher _this,
    u_writer writer);

u_result
u_publisherRemoveWriter (
    u_publisher _this,
    u_writer writer);

u_result
u_publisherInit (
    u_publisher _this,
    u_participant participant);

u_result
u_publisherDeinit (
    u_publisher _this);

#endif
