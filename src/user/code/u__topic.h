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

#ifndef U__TOPIC_H
#define U__TOPIC_H

#include "u_topic.h"

u_result
u_topicInit(
    u_topic _this,
    const c_char *name,
    u_participant p);

u_result
u_topicDeinit (
    u_topic _this);

c_bool
u_topicIsBuiltin (
    u_topic _this);

#endif
