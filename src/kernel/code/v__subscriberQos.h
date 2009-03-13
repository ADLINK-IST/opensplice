
#ifndef V__SUBSCRIBERQOS_H
#define V__SUBSCRIBERQOS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_qos.h"
#include "v_subscriberQos.h"

v_result
v_subscriberQosSet(
    v_subscriberQos _this,
    v_subscriberQos tmpl,
    c_bool enable,
    v_qosChangeMask *changeMask);

#if defined (__cplusplus)
}
#endif

#endif /* V__SUBSCRIBERQOS_H */
