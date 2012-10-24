/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef IN_MESSAGEQOS_H
#define IN_MESSAGEQOS_H

#include "v_kernel.h"
#include "in_connectivityPeerWriter.h"

/**
 * The message QoS contructor creates a new message QoS object from
 * the writer`s QoS policy value.
 */
v_messageQos
in_messageQos_new(
    in_connectivityPeerWriter writer, c_base base);

#endif
