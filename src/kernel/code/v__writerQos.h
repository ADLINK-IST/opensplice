
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
