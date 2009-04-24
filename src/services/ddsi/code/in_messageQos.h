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
