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
