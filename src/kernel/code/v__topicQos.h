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

#ifndef V__TOPICQOS_H
#define V__TOPICQOS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_qos.h"
#include "v_topicQos.h"

v_result
v_topicQosSet(
    v_topicQos _this,
    v_topicQos tmpl,
    c_bool enabled,
    v_qosChangeMask *changeMask);

#if defined (__cplusplus)
}
#endif

#endif /* V__TOPICQOS_H */
