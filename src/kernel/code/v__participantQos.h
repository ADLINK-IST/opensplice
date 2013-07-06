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

#ifndef V__PARTICIPANTQOS_H
#define V__PARTICIPANTQOS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_qos.h"
#include "v_participantQos.h"

v_result
v_participantQosSet(
    v_participantQos _this,
    v_participantQos tmpl,
    v_qosChangeMask *changeMask);

#if defined (__cplusplus)
}
#endif

#endif /* V__PARTICIPANTQOS_H */
