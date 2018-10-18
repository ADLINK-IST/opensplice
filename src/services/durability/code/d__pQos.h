#ifndef D__PQOS_H
#define D__PQOS_H

#include "v_kernel.h"
#include "durabilityModule2.h"

#if defined (__cplusplus)
extern "C" {
#endif

d_persistentTopicQosV0 d_pQosFromTopicQos (const struct v_topicQos_s *qos);
v_topicQos d_topicQosFromPQos (c_base base, const struct d_persistentTopicQosV0_s *pqos);

#if defined (__cplusplus)
}
#endif

#endif
