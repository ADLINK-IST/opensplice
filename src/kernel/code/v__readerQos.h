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
#ifndef V__READERQOS_H
#define V__READERQOS_H

#include "v_readerQos.h"
#include "v_qos.h"

#if defined (__cplusplus)
extern "C" {
#endif

v_result
v_readerQosSet(
    v_readerQos _this,
    v_readerQos tmpl,
    c_bool enabled,
    v_qosChangeMask *changeMask);

#if defined (__cplusplus)
}
#endif

#endif /* V__READERQOS_H */
