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

#ifndef V__WRITERQOS_H
#define V__WRITERQOS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_qos.h"
#include "v_writerQos.h"

v_result
v_writerQosSet(
    v_writerQos _this,
    v_writerQos tmpl,
    c_bool enabled,
    v_qosChangeMask *changeMask);

#if defined (__cplusplus)
}
#endif

#endif /* V__WRITERQOS_H */
