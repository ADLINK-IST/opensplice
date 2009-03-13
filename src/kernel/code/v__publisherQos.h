
#ifndef V__PUBLISHERQOS_H
#define V__PUBLISHERQOS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_qos.h"
#include "v_publisherQos.h"

v_result
v_publisherQosSet(
    v_publisherQos _this,
    v_publisherQos tmpl,
    c_bool enable,
    v_qosChangeMask *changeMask);

#if defined (__cplusplus)
}
#endif

#endif /* V__PUBLISHERQOS_H */
