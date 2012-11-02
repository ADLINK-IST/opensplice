/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef V_POLICY_H
#define V_POLICY_H

#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define V_POLICY_BIT_USERDATA          (0x00000001U << V_USERDATAPOLICY_ID)
#define V_POLICY_BIT_DURABILITY        (0x00000001U << V_DURABILITYPOLICY_ID)
#define V_POLICY_BIT_DURABILITYSERVICE (0x00000001U << V_DURABILITYSERVICEPOLICY_ID)
#define V_POLICY_BIT_PRESENTATION      (0x00000001U << V_PRESENTATIONPOLICY_ID)
#define V_POLICY_BIT_DEADLINE          (0x00000001U << V_DEADLINEPOLICY_ID)
#define V_POLICY_BIT_LATENCY           (0x00000001U << V_LATENCYPOLICY_ID)
#define V_POLICY_BIT_OWNERSHIP         (0x00000001U << V_OWNERSHIPPOLICY_ID)
#define V_POLICY_BIT_STRENGTH          (0x00000001U << V_STRENGTHPOLICY_ID)
#define V_POLICY_BIT_LIVELINESS        (0x00000001U << V_LIVELINESSPOLICY_ID)
#define V_POLICY_BIT_PACING            (0x00000001U << V_PACINGPOLICY_ID)
#define V_POLICY_BIT_PARTITION         (0x00000001U << V_PARTITIONPOLICY_ID)
#define V_POLICY_BIT_RELIABILITY       (0x00000001U << V_RELIABILITYPOLICY_ID)
#define V_POLICY_BIT_ORDERBY           (0x00000001U << V_ORDERBYPOLICY_ID)
#define V_POLICY_BIT_HISTORY           (0x00000001U << V_HISTORYPOLICY_ID)
#define V_POLICY_BIT_RESOURCE          (0x00000001U << V_RESOURCEPOLICY_ID)
#define V_POLICY_BIT_ENTITYFACTORY     (0x00000001U << V_ENTITYFACTORYPOLICY_ID)
#define V_POLICY_BIT_WRITERLIFECYCLE   (0x00000001U << V_WRITERLIFECYCLEPOLICY_ID)
#define V_POLICY_BIT_READERLIFECYCLE   (0x00000001U << V_READERLIFECYCLEPOLICY_ID)
#define V_POLICY_BIT_TOPICDATA         (0x00000001U << V_TOPICDATAPOLICY_ID)
#define V_POLICY_BIT_GROUPDATA         (0x00000001U << V_GROUPDATAPOLICY_ID)
#define V_POLICY_BIT_TRANSPORT         (0x00000001U << V_TRANSPORTPOLICY_ID)
#define V_POLICY_BIT_LIFESPAN          (0x00000001U << V_LIFESPANPOLICY_ID)
#define V_POLICY_BIT_USERKEY           (0x00000001U << V_USERKEYPOLICY_ID)
#define V_POLICY_BIT_SHARE             (0x00000001U << V_SHAREPOLICY_ID)
#define V_POLICY_BIT_READERLIFESPAN    (0x00000001U << V_READERLIFESPANPOLICY_ID)
#define V_POLICY_BIT_SCHEDULING        (0x00000001U << V_SCHEDULINGPOLICY_ID)

/* policy compatibility */
#define v_reliabilityPolicyCompatible(offered,requested)\
    (offered.kind >= requested.kind)

#define v_reliabilityPolicyEqual(p1,p2)\
    ((p1.kind == p2.kind) &&\
     (p1.synchronous == p2.synchronous) && \
     ((p1.kind == V_RELIABILITY_BESTEFFORT) ||\
      (c_timeCompare(p1.max_blocking_time,p2.max_blocking_time) == C_EQ)))

#define v_reliabilityPolicyValid(p)\
    ((p.kind == V_RELIABILITY_BESTEFFORT) ||\
     ((p.kind == V_RELIABILITY_RELIABLE) && c_timeValid(p.max_blocking_time)))
/* The construction above only checks max_blocking_time when the kind
   is RELIABLE */

#define v_durabilityPolicyCompatible(offered,requested)\
    (offered.kind >= requested.kind)

#define v_durabilityPolicyEqual(p1,p2)\
    (p1.kind == p2.kind)

#define v_durabilityPolicyValid(p)\
    ((p.kind == V_DURABILITY_VOLATILE) ||\
     (p.kind == V_DURABILITY_TRANSIENT_LOCAL) ||\
     (p.kind == V_DURABILITY_TRANSIENT) ||\
     (p.kind == V_DURABILITY_PERSISTENT))

/* The construction used for v_durabilityPolicyValid only checks
   service_cleanup_delay when kind is not VOLATILE */
#define v_durabilityServicePolicyEqual(p1,p2)\
     ((c_timeCompare(p1.service_cleanup_delay, p2.service_cleanup_delay) == C_EQ) && \
      (p1.history_kind == p2.history_kind) &&\
      (p1.history_depth == p2.history_depth) &&\
      (p1.max_samples == p2.max_samples) &&\
      (p1.max_instances == p2.max_instances) &&\
      (p1.max_samples_per_instance == p2.max_samples_per_instance))

#define v_durabilityServicePolicyValid(p)\
    (((p.max_samples == V_LENGTH_UNLIMITED) ||\
      (p.max_samples > 0)) &&\
     ((p.max_instances == V_LENGTH_UNLIMITED) ||\
      (p.max_instances > 0)) &&\
     ((p.max_samples_per_instance == V_LENGTH_UNLIMITED) ||\
      (p.max_samples_per_instance > 0)) &&\
     ((p.history_kind == V_HISTORY_KEEPLAST) ||\
      (p.history_kind == V_HISTORY_KEEPALL)) &&\
     ((p.history_depth == V_LENGTH_UNLIMITED) ||\
      (p.history_depth > 0)) &&\
       c_timeValid(p.service_cleanup_delay))

#define v_presentationPolicyCompatible(offered,requested)\
    ((offered.access_scope >= requested.access_scope) && \
     ((requested.coherent_access == FALSE) || \
      (offered.coherent_access == requested.coherent_access)) &&\
     ((requested.ordered_access == FALSE) || \
      (offered.ordered_access == requested.ordered_access)))

#define v_presentationPolicyEqual(p1,p2)\
    ((p1.access_scope == p2.access_scope) &&\
     (p1.coherent_access == p2.coherent_access) &&\
     (p1.ordered_access == p2.ordered_access))

#define v_presentationPolicyValid(p)\
    (((p.access_scope == V_PRESENTATION_INSTANCE) ||\
      (p.access_scope == V_PRESENTATION_TOPIC) ||\
      (p.access_scope == V_PRESENTATION_GROUP)) &&\
     ((p.coherent_access == TRUE) || (p.coherent_access == FALSE)) &&\
     ((p.ordered_access == TRUE) || (p.ordered_access == FALSE)))

/* offered <= requested
 * Take account for INFINITE
 */
#define v_latencyPolicyCompatible(offered,requested)\
    ((c_timeCompare(requested.duration, C_TIME_INFINITE) == C_EQ) ||\
     (c_timeCompare(offered.duration,requested.duration) != C_GT)) /* <= */

#define v_latencyPolicyEqual(p1,p2)\
    (c_timeCompare(p1.duration,p2.duration) == C_EQ)

#define v_latencyPolicyValid(p)\
    (c_timeValid(p.duration))

#define v_orderbyPolicyCompatible(offered,requested)\
    (offered.kind >= requested.kind)

#define v_orderbyPolicyEqual(p1,p2)\
    (p1.kind == p2.kind)

#define v_orderbyPolicyValid(p)\
    ((p.kind == V_ORDERBY_RECEPTIONTIME) ||\
     (p.kind == V_ORDERBY_SOURCETIME))

/* offered <= requested
 * Take account for INFINITE
 */
#define v_deadlinePolicyCompatible(offered,requested)\
    ((c_timeCompare(requested.period, C_TIME_INFINITE) == C_EQ) ||\
     (c_timeCompare(offered.period, requested.period) != C_GT)) /* <= */

#define v_deadlinePolicyEqual(p1,p2)\
    (c_timeCompare(p1.period, p2.period) == C_EQ)

#define v_deadlinePolicyValid(p)\
    (c_timeValid(p.period))

/* offered <= requested
 * Take account for INFINITE
 */
#define v_livelinessPolicyCompatible(offered,requested)\
    ((offered.kind >= requested.kind) &&\
     ((c_timeCompare(requested.lease_duration, C_TIME_INFINITE) == C_EQ) ||\
      (c_timeCompare(offered.lease_duration,requested.lease_duration) != C_GT))) /* <= */

#define v_livelinessPolicyEqual(p1,p2)\
    ((p1.kind == p2.kind) &&\
     (c_timeCompare(p1.lease_duration,p2.lease_duration) == C_EQ))

#define v_livelinessPolicyValid(p)\
    (((p.kind == V_LIVELINESS_AUTOMATIC) ||\
      (p.kind == V_LIVELINESS_PARTICIPANT) ||\
      (p.kind == V_LIVELINESS_TOPIC)) &&\
     (c_timeValid(p.lease_duration)))

#define v_historyPolicyEqual(p1,p2)\
    ((p1.kind == p2.kind) &&\
     (p1.depth == p2.depth))

#define v_historyPolicyValid(p)\
    (((p.kind == V_HISTORY_KEEPLAST) ||\
      (p.kind == V_HISTORY_KEEPALL)) &&\
     ((p.depth == V_LENGTH_UNLIMITED) ||\
      (p.depth > 0)))

#define v_resourcePolicyEqual(p1,p2)\
    ((p1.max_samples == p2.max_samples) &&\
     (p1.max_instances == p2.max_instances) &&\
     (p1.max_samples_per_instance == p2.max_samples_per_instance))

#define v_resourcePolicyValid(p)\
    (((p.max_samples == V_LENGTH_UNLIMITED) ||\
      (p.max_samples > 0)) &&\
     ((p.max_instances == V_LENGTH_UNLIMITED) ||\
      (p.max_instances > 0)) &&\
     ((p.max_samples_per_instance == V_LENGTH_UNLIMITED) ||\
      (p.max_samples_per_instance > 0)))

#define v_resourcePolicyIsUnlimited(p)\
    ((p.max_samples == V_LENGTH_UNLIMITED) &&\
     (p.max_instances == V_LENGTH_UNLIMITED) &&\
     (p.max_samples_per_instance == V_LENGTH_UNLIMITED))

#define v_transportPolicyEqual(p1,p2)\
    (p1.value == p2.value)

#define v_transportPolicyValid(p)\
    (TRUE)

#define v_lifespanPolicyEqual(p1,p2)\
    (c_timeCompare(p1.duration,p2.duration) == C_EQ)

#define v_lifespanPolicyValid(p)\
    (c_timeValid(p.duration))

#define v_ownershipPolicyCompatible(p1,p2)\
    (p1.kind == p2.kind) /* this is not an error! see page 2-105 of OMG spec */

#define v_ownershipPolicyEqual(p1,p2)\
    (p1.kind == p2.kind)

#define v_ownershipPolicyValid(p)\
    ((p.kind == V_OWNERSHIP_SHARED) ||\
     (p.kind == V_OWNERSHIP_EXCLUSIVE))

#define v_strengthPolicyEqual(p1,p2)\
    (p1.value == p2.value)

#define v_strengthPolicyValid(p)\
    (TRUE)

#define v_writerLifecyclePolicyEqual(p1,p2)\
    ((p1.autodispose_unregistered_instances == p2.autodispose_unregistered_instances) &&\
     (c_timeCompare(p1.autopurge_suspended_samples_delay,p2.autopurge_suspended_samples_delay) == C_EQ) &&\
     (c_timeCompare(p1.autounregister_instance_delay,p2.autounregister_instance_delay) == C_EQ))

#define v_writerLifecyclePolicyValid(p)\
    (((p.autodispose_unregistered_instances == TRUE) ||\
      (p.autodispose_unregistered_instances == FALSE)) &&\
     (c_timeValid(p.autopurge_suspended_samples_delay)) &&\
     (c_timeValid(p.autounregister_instance_delay)))

#define v_entityFactoryPolicyEqual(p1,p2)\
    (p1.autoenable_created_entities == p2.autoenable_created_entities)

#define v_entityFactoryPolicyValid(p)\
    ((p.autoenable_created_entities == TRUE) ||\
     (p.autoenable_created_entities == FALSE))

#define v_readerLifecyclePolicyEqual(p1,p2)\
    ((c_timeCompare(p1.autopurge_nowriter_samples_delay,p2.autopurge_nowriter_samples_delay) == C_EQ) && \
     (c_timeCompare(p1.autopurge_disposed_samples_delay,p2.autopurge_disposed_samples_delay) == C_EQ) && \
     (p1.enable_invalid_samples == p2.enable_invalid_samples))

#define v_readerLifecyclePolicyValid(p)\
    (c_timeValid(p.autopurge_nowriter_samples_delay) && \
     c_timeValid(p.autopurge_disposed_samples_delay) && \
     ((p.enable_invalid_samples == TRUE) || \
      (p.enable_invalid_samples == FALSE)))

#define v_pacingPolicyEqual(p1,p2)\
    (c_timeCompare(p1.minSeperation,p2.minSeperation) == C_EQ)

#define v_pacingPolicyValid(p)\
    (c_timeValid(p.minSeperation))

#define v_userDataPolicyEqual(p1,p2)\
    ((p1.size == p2.size) && (memcmp(p1.value,p2.value,p1.size) == 0))

#define v_userDataPolicyValid(p)\
    (((p.size == 0) && (p.value == NULL)) || \
     ((p.size != 0) && (p.value != NULL)))

#define v_topicDataPolicyEqual(p1,p2)\
    ((p1.size == p2.size) && (memcmp(p1.value,p2.value,p1.size) == 0))

#define v_topicDataPolicyValid(p)\
    (((p.size == 0) && (p.value == NULL)) || \
     ((p.size != 0) && (p.value != NULL)))

#define v_groupDataPolicyEqual(p1,p2)\
    ((p1.size == p2.size) && (memcmp(p1.value,p2.value,p1.size) == 0))

#define v_groupDataPolicyValid(p)\
    (((p.size == 0) && (p.value == NULL)) || \
     ((p.size != 0) && (p.value != NULL)))

#define v_partitionPolicyEqual(p1,p2)\
    ((p1 != NULL) && (p2 != NULL) && (strcmp(p1,p2) == 0))

#define v_keyPolicyEqual(p1,p2)\
    ((p1.expression != NULL) && (p2.expression != NULL) && (strcmp(p1.expression,p2.expression) == 0))

#define v_readerLifespanPolicyCompatible(offered,requested)\
    ((!requested.used) ||\
     ((c_timeCompare(offered.duration, C_TIME_INFINITE) == C_EQ) &&\
      (c_timeCompare(requested.duration, C_TIME_INFINITE) == C_EQ)) ||\
     ((c_timeCompare(offered.duration, C_TIME_INFINITE) != C_EQ) &&\
      ((c_timeCompare(offered.duration,requested.duration) == C_EQ) ||\
       (c_timeCompare(offered.duration,requested.duration) == C_LT))))

#define v_readerLifespanPolicyEqual(p1,p2)\
    ((p1.used == p2.used) &&\
     ((p1.used && (c_timeCompare(p1.duration,p2.duration) == C_EQ)) || !p1.used))

#define v_readerLifespanPolicyValid(p)\
    (((p.used == FALSE) || (p.used == TRUE)) && c_timeValid(p.duration))

#define v_sharePolicyEqual(p1,p2)\
    ((p1.enable == p2.enable) && \
     ((p1.enable == FALSE) || ((p1.name != NULL) && (p2.name != NULL) && (strcmp(p1.name,p2.name) == 0))))

#define v_sharePolicyValid(p)\
    ((p.enable == FALSE) || ((p.enable == TRUE) && (p.name != NULL)))

#define v_userKeyPolicyEqual(p1,p2)\
    ((p1.enable == p2.enable) && \
     ((p1.enable == FALSE) || \
      ((p1.expression != NULL) && (p2.expression != NULL) && (strcmp(p1.expression,p2.expression) == 0))))

#define v_userKeyPolicyValid(p)\
    ((p.enable == FALSE) || ((p.enable == TRUE) && (p.expression != NULL)))

#define v_schedulingPolicyEqual(p1,p2)\
    ((p1.kind == p2.kind) && \
     (p1.priority == p2.priority) && \
     (p1.priorityKind == p2.priorityKind))

#define v_schedulingPolicyValid(p) \
   (((p.kind == V_SCHED_DEFAULT) || \
     (p.kind == V_SCHED_TIMESHARING) || \
     (p.kind == V_SCHED_REALTIME)) && \
    ((p.priorityKind == V_SCHED_PRIO_ABSOLUTE) || \
      p.priorityKind == V_SCHED_PRIO_RELATIVE))

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_POLICY_H */
