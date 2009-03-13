
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
