/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include <dds_dcps.h>
#include <dds_dcps_private.h>
#include <dds.h>
#include <dds__time.h>


#define QOS_POLICY_USER_DATA              (0x0001U << 0)
#define QOS_POLICY_ENTITY_FACTORY         (0x0001U << 1)
#define QOS_POLICY_WATCHDOC_SCHEDULING    (0x0001U << 2)
#define QOS_POLICY_LISTENER_SCHEDULING    (0x0001U << 3)
#define QOS_POLICY_PRESENTATION           (0x0001U << 4)
#define QOS_POLICY_PARTITION              (0x0001U << 5)
#define QOS_POLICY_GROUP_DATA             (0x0001U << 6)
#define QOS_POLICY_TOPIC_DATA             (0x0001U << 7)
#define QOS_POLICY_DURABILITY             (0x0001U << 8)
#define QOS_POLICY_DURABILITY_SERVICE     (0x0001U << 10)
#define QOS_POLICY_DEADLINE               (0x0001U << 11)
#define QOS_POLICY_LATENCY_BUDGET         (0x0001U << 12)
#define QOS_POLICY_LIVELINESS             (0x0001U << 13)
#define QOS_POLICY_RELIABLE               (0x0001U << 14)
#define QOS_POLICY_DESTINATION_ORDER      (0x0001U << 15)
#define QOS_POLICY_HISTORY                (0x0001U << 16)
#define QOS_POLICY_RESOURCE_LIMITS        (0x0001U << 17)
#define QOS_POLICY_TRANSPORT_PRIORITY     (0x0001U << 18)
#define QOS_POLICY_LIFESPAN               (0x0001U << 19)
#define QOS_POLICY_OWNERSHIP              (0x0001U << 20)
#define QOS_POLICY_OWNERSHIP_STRENGTH     (0x0001U << 21)
#define QOS_POLICY_WRITER_DATA_LIFECYCLE  (0x0001U << 22)
#define QOS_POLICY_TIME_BASED_FILTER      (0x0001U << 23)
#define QOS_POLICY_READER_DATA_LIFECYCLE  (0x0001U << 24)
#define QOS_POLICY_SUBSCRIPTION_KEYS      (0x0001U << 25)
#define QOS_POLICY_READER_LIFESPAN        (0x0001U << 26)
#define QOS_POLICY_SHARE                  (0x0001U << 27)

#define QOS_POLICY_IS_SET(q,p) (((q)->mask & (p)) == (p))
#define QOS_POLICY_SET(q,p)    ((q)->mask |= (p))
#define QOS_POLICY_CLEAR(q,p)  ((q)->mask &= ~(p))


struct nn_xqos {
    os_uint32 mask;
    DDS_UserDataQosPolicy user_data;
    DDS_EntityFactoryQosPolicy entity_factory;
    DDS_SchedulingQosPolicy watchdog_scheduling;
    DDS_SchedulingQosPolicy listener_scheduling;
    DDS_PresentationQosPolicy presentation;
    DDS_PartitionQosPolicy partition;
    DDS_GroupDataQosPolicy group_data;
    DDS_TopicDataQosPolicy topic_data;
    DDS_DurabilityQosPolicy durability;
    DDS_DurabilityServiceQosPolicy durability_service;
    DDS_DeadlineQosPolicy deadline;
    DDS_LatencyBudgetQosPolicy latency_budget;
    DDS_LivelinessQosPolicy liveliness;
    DDS_ReliabilityQosPolicy reliability;
    DDS_DestinationOrderQosPolicy destination_order;
    DDS_HistoryQosPolicy history;
    DDS_ResourceLimitsQosPolicy resource_limits;
    DDS_TransportPriorityQosPolicy transport_priority;
    DDS_LifespanQosPolicy lifespan;
    DDS_OwnershipQosPolicy ownership;
    DDS_OwnershipStrengthQosPolicy ownership_strength;
    DDS_WriterDataLifecycleQosPolicy writer_data_lifecycle;
    DDS_TimeBasedFilterQosPolicy time_based_filter;
    DDS_ReaderDataLifecycleQosPolicy reader_data_lifecycle;
    DDS_SubscriptionKeyQosPolicy subscription_keys;
    DDS_ReaderLifespanQosPolicy reader_lifespan;
    DDS_ShareQosPolicy share;
};

static void
dds_qos_data_seq_free(
    DDS_octSeq * restrict seq)
{
    DDS_free(seq->_buffer);
    memset(seq, 0, sizeof(*seq));
}

static void
dds_qos_data_seq_copy(
    DDS_octSeq * restrict dst,
    const DDS_octSeq * restrict src)
{
    *dst = *src;
     if (src->_buffer) {
         if (src->_maximum > 0) {
             dst->_buffer = DDS_octSeq_allocbuf(src->_maximum);
             memcpy(dst->_buffer, src->_buffer, src->_length);
         } else {
             dst->_buffer = NULL;
         }
     }
}

static void
dds_get_data_seq_out (
    const DDS_octSeq * restrict seq,
    void ** value,
    size_t *sz)
{
    if (value && sz) {
        assert(seq->_buffer || (seq->_length == 0));
        if (seq->_buffer) {
            *value = DDS_sequence_allocbuf((DDS_deallocatorType) NULL,
                                  sizeof(DDS_octet*), seq->_length);
            memcpy(*value, seq->_buffer, seq->_length);
        } else {
            *value = NULL;
        }
        *sz = seq->_length;
    }
}

static void
dds_qos_data_seq_copy_in(
    DDS_octSeq * restrict seq,
    const void * restrict value,
    size_t sz)
{
    if (sz > 0) {
        seq->_buffer = DDS_octSeq_allocbuf(sz);
        memcpy(seq->_buffer, value, sz);

    }
    seq->_length = sz;
    seq->_maximum = sz;
    seq->_release = true;
}


static void
dds_qos_userdata_policy_free (
    DDS_UserDataQosPolicy * restrict policy)
{
    dds_qos_data_seq_free(&policy->value);
}

static void
dds_qos_userdata_policy_copy (
    DDS_UserDataQosPolicy * restrict dst,
    const DDS_UserDataQosPolicy * restrict src)
{
    dds_qos_data_seq_copy(&dst->value, &src->value);
}

static void
dds_qos_groupdata_policy_free (
    DDS_GroupDataQosPolicy * restrict policy)
{
    dds_qos_data_seq_free(&policy->value);
}

static void
dds_qos_groupdata_policy_copy (
    DDS_GroupDataQosPolicy * restrict dst,
    const DDS_GroupDataQosPolicy * restrict src)
{
    dds_qos_data_seq_copy(&dst->value, &src->value);
}

static void
dds_qos_topicdata_policy_free (
    DDS_TopicDataQosPolicy * restrict policy)
{
    dds_qos_data_seq_free(&policy->value);
}

static void
dds_qos_topicdata_policy_copy (
    DDS_TopicDataQosPolicy * restrict dst,
    const DDS_TopicDataQosPolicy * restrict src)
{
    dds_qos_data_seq_copy(&dst->value, &src->value);
}

static void
dds_qos_partition_policy_free (
    DDS_PartitionQosPolicy * restrict policy)
{
    DDS_free(policy->name._buffer);
    memset(policy, 0, sizeof(*policy));
}

static void
dds_qos_partition_policy_copy (
    DDS_PartitionQosPolicy * restrict to,
    const DDS_PartitionQosPolicy * restrict from)
{
    uint32_t i;
   *to = *from;
   if (from->name._buffer) {
       if (from->name._maximum) {
           to->name._buffer = DDS_StringSeq_allocbuf(from->name._maximum);
           for (i = 0; i < from->name._length; i++) {
               to->name._buffer[i] = DDS_string_dup(from->name._buffer[i]);
           }
       }
   }
}

static void
dds_qos_subscription_keys_policy_free (
    DDS_SubscriptionKeyQosPolicy * restrict policy)
{
    if (policy->key_list._buffer) {
        DDS_free(policy->key_list._buffer);
    }
    memset(policy, 0, sizeof(*policy));
}

static void
dds_qos_subscription_keys_policy_copy (
    DDS_SubscriptionKeyQosPolicy * restrict to,
    const DDS_SubscriptionKeyQosPolicy * restrict from)
{
    uint32_t i;

   *to = *from;
   if (from->use_key_list && from->key_list._buffer) {
       if (from->key_list._maximum) {
           to->key_list._buffer = DDS_StringSeq_allocbuf(from->key_list._maximum);
           for (i = 0; i < from->key_list._length; i++) {
               to->key_list._buffer[i] = DDS_string_dup(from->key_list._buffer[i]);
           }
       }
   }
}


static void
dds_qos_share_policy_free (
    DDS_ShareQosPolicy * restrict policy)
{
    DDS_free(policy->name);
    memset(policy, 0, sizeof(*policy));
}

static void
dds_qos_share_policy_copy (
    DDS_ShareQosPolicy * restrict to,
    const DDS_ShareQosPolicy * restrict from)
{
    if (from->enable) {
        to->enable = true;
        if (from->name) {
            to->name = DDS_string_dup(from->name);
        } else {
            to->name = NULL;
        }
    } else {
        to->enable = false;
    }
}

static void
dds_qos_init(
    dds_qos_t * restrict qos)
{
    assert(qos);

    memset(qos, 0, sizeof(*qos));

    qos->entity_factory.autoenable_created_entities = true;
    qos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    qos->deadline.period = dds_duration_to_sac(DDS_INFINITY);
    qos->durability_service.service_cleanup_delay = dds_duration_to_sac(0);
    qos->durability_service.history_kind = DDS_KEEP_LAST_HISTORY_QOS;
    qos->durability_service.history_depth = 1;
    qos->durability_service.max_samples = DDS_LENGTH_UNLIMITED;
    qos->durability_service.max_instances = DDS_LENGTH_UNLIMITED;
    qos->durability_service.max_samples_per_instance = DDS_LENGTH_UNLIMITED;
    qos->presentation.access_scope = DDS_INSTANCE_PRESENTATION_QOS;
    qos->latency_budget.duration = dds_duration_to_sac(0);
    qos->ownership.kind = DDS_SHARED_OWNERSHIP_QOS;
    qos->liveliness.kind = DDS_AUTOMATIC_LIVELINESS_QOS;
    qos->liveliness.lease_duration = dds_duration_to_sac(DDS_INFINITY);
    qos->time_based_filter.minimum_separation = dds_duration_to_sac(0);
    qos->reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
    qos->reliability.max_blocking_time = dds_duration_to_sac(DDS_MSECS(100));
    qos->reliability.synchronous = false;
    qos->lifespan.duration = dds_duration_to_sac(DDS_INFINITY);
    qos->destination_order.kind = DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    qos->history.kind = DDS_KEEP_ALL_HISTORY_QOS;
    qos->history.depth = 1;
    qos->resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
    qos->resource_limits.max_instances = DDS_LENGTH_UNLIMITED;
    qos->resource_limits.max_samples_per_instance = DDS_LENGTH_UNLIMITED;
    qos->writer_data_lifecycle.autodispose_unregistered_instances = true;
    qos->writer_data_lifecycle.autopurge_suspended_samples_delay = dds_duration_to_sac(DDS_INFINITY);
    qos->writer_data_lifecycle.autounregister_instance_delay = dds_duration_to_sac(DDS_INFINITY);
    qos->reader_data_lifecycle.autopurge_nowriter_samples_delay = dds_duration_to_sac(DDS_INFINITY);
    qos->reader_data_lifecycle.autopurge_disposed_samples_delay = dds_duration_to_sac(DDS_INFINITY);
    qos->reader_data_lifecycle.autopurge_dispose_all = false;
    qos->reader_data_lifecycle.enable_invalid_samples = true;
    qos->reader_data_lifecycle.invalid_sample_visibility.kind = DDS_MINIMUM_INVALID_SAMPLES;
}



dds_qos_t *
dds_qos_create (void)
{
    dds_qos_t *qos;

    qos = os_malloc(sizeof(struct nn_xqos));
    memset(qos, 0, sizeof(*qos));

    return qos;
}

void
dds_qos_delete (
    dds_qos_t * restrict qos)
{
    if (qos) {
        dds_qos_userdata_policy_free(&qos->user_data);
        dds_qos_groupdata_policy_free(&qos->group_data);
        dds_qos_topicdata_policy_free(&qos->topic_data);
        dds_qos_partition_policy_free(&qos->partition);
        dds_qos_subscription_keys_policy_free(&qos->subscription_keys);
        dds_qos_share_policy_free(&qos->share);
        os_free(qos);
    }
}

void
dds_qos_reset (
    dds_qos_t * restrict qos)
{
    if (qos) {
        dds_qos_userdata_policy_free(&qos->user_data);
        dds_qos_partition_policy_free(&qos->partition);
        dds_qos_groupdata_policy_free(&qos->group_data);
        dds_qos_topicdata_policy_free(&qos->topic_data);
        dds_qos_subscription_keys_policy_free(&qos->subscription_keys);
        dds_qos_share_policy_free(&qos->share);
        dds_qos_init(qos);
    }
}

void
dds_qos_copy (
    dds_qos_t * restrict dst,
    const dds_qos_t * restrict src)
{
    dst->mask = src->mask;
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_USER_DATA)) {
        dds_qos_userdata_policy_copy(&dst->user_data, &src->user_data);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_ENTITY_FACTORY)) {
        dst->entity_factory = src->entity_factory;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_WATCHDOC_SCHEDULING)) {
        dst->watchdog_scheduling = src->watchdog_scheduling;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LISTENER_SCHEDULING)) {
        dst->listener_scheduling = src->listener_scheduling;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_PRESENTATION)) {
        dst->presentation = src->presentation;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_PARTITION)) {
        dds_qos_partition_policy_copy(&dst->partition, &src->partition);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_GROUP_DATA)) {
        dds_qos_groupdata_policy_copy(&dst->group_data, &src->group_data);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_TOPIC_DATA)) {
        dds_qos_topicdata_policy_copy(&dst->topic_data, &src->topic_data);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DURABILITY)) {
        dst->durability = src->durability;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DURABILITY_SERVICE)) {
        dst->durability_service = src->durability_service;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DEADLINE)) {
        dst->deadline = src->deadline;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LATENCY_BUDGET)) {
        dst->latency_budget = src->latency_budget;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LIFESPAN)) {
        dst->liveliness = src->liveliness;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_RELIABLE)) {
        dst->reliability = src->reliability;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DESTINATION_ORDER)) {
        dst->destination_order = src->destination_order;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_HISTORY)) {
        dst->history = src->history;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_RESOURCE_LIMITS)) {
        dst->resource_limits = src->resource_limits;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_TRANSPORT_PRIORITY)) {
        dst->transport_priority = src->transport_priority;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LIFESPAN)) {
        dst->lifespan = src->lifespan;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_OWNERSHIP)) {
        dst->ownership = src->ownership;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_OWNERSHIP_STRENGTH)) {
        dst->ownership_strength = src->ownership_strength;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_WRITER_DATA_LIFECYCLE)) {
        dst->writer_data_lifecycle = src->writer_data_lifecycle;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_TIME_BASED_FILTER)) {
        dst->time_based_filter = src->time_based_filter;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_READER_DATA_LIFECYCLE)) {
        dst->reader_data_lifecycle = src->reader_data_lifecycle;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_SUBSCRIPTION_KEYS)) {
        dds_qos_subscription_keys_policy_copy(&dst->subscription_keys, &src->subscription_keys);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_READER_LIFESPAN)) {
        dst->reader_lifespan = src->reader_lifespan;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_SHARE)) {
        dds_qos_share_policy_copy(&dst->share, &src->share);
    }
}

void
dds_qos_merge (
    dds_qos_t * restrict dst,
    const dds_qos_t * restrict src)
{
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_USER_DATA) && QOS_POLICY_IS_SET(src, QOS_POLICY_USER_DATA)) {
        dds_qos_userdata_policy_copy(&dst->user_data, &src->user_data);
        QOS_POLICY_SET(dst, QOS_POLICY_USER_DATA);
    }
    if (!QOS_POLICY_IS_SET(dst,  QOS_POLICY_ENTITY_FACTORY) && (src->mask & QOS_POLICY_ENTITY_FACTORY)) {
        dst->entity_factory = src->entity_factory;
        QOS_POLICY_SET(dst, QOS_POLICY_ENTITY_FACTORY);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_WATCHDOC_SCHEDULING) && QOS_POLICY_IS_SET(src, QOS_POLICY_WATCHDOC_SCHEDULING)) {
        dst->watchdog_scheduling = src->watchdog_scheduling;
        QOS_POLICY_SET(dst, QOS_POLICY_WATCHDOC_SCHEDULING);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_LISTENER_SCHEDULING) && QOS_POLICY_IS_SET(src, QOS_POLICY_LISTENER_SCHEDULING)) {
        dst->listener_scheduling = src->listener_scheduling;
        QOS_POLICY_SET(dst, QOS_POLICY_LISTENER_SCHEDULING);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_PRESENTATION) && QOS_POLICY_IS_SET(src, QOS_POLICY_PRESENTATION)) {
        dst->presentation = src->presentation;
        QOS_POLICY_SET(dst, QOS_POLICY_PRESENTATION);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_PARTITION) && QOS_POLICY_IS_SET(src, QOS_POLICY_PARTITION)) {
        dds_qos_partition_policy_copy(&dst->partition, &src->partition);
        QOS_POLICY_SET(dst, QOS_POLICY_PARTITION);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_GROUP_DATA) && QOS_POLICY_IS_SET(src, QOS_POLICY_GROUP_DATA)) {
        dds_qos_groupdata_policy_copy(&dst->group_data, &src->group_data);
        QOS_POLICY_SET(dst, QOS_POLICY_GROUP_DATA);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_TOPIC_DATA) && QOS_POLICY_IS_SET(src, QOS_POLICY_TOPIC_DATA)) {
        dds_qos_topicdata_policy_copy(&dst->topic_data, &src->topic_data);
        QOS_POLICY_SET(dst, QOS_POLICY_TOPIC_DATA);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_DURABILITY) && QOS_POLICY_IS_SET(src, QOS_POLICY_DURABILITY)) {
        dst->durability = src->durability;
        QOS_POLICY_SET(dst, QOS_POLICY_DURABILITY);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_DURABILITY_SERVICE) && QOS_POLICY_IS_SET(src, QOS_POLICY_DURABILITY_SERVICE)) {
        dst->durability_service = src->durability_service;
        QOS_POLICY_SET(dst, QOS_POLICY_DURABILITY_SERVICE);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_DEADLINE) && QOS_POLICY_IS_SET(src, QOS_POLICY_DEADLINE)) {
        dst->deadline = src->deadline;
        QOS_POLICY_SET(dst, QOS_POLICY_DEADLINE);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_LATENCY_BUDGET) && QOS_POLICY_IS_SET(src, QOS_POLICY_LATENCY_BUDGET)) {
        dst->latency_budget = src->latency_budget;
        QOS_POLICY_SET(dst, QOS_POLICY_LATENCY_BUDGET);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_LIVELINESS) && QOS_POLICY_IS_SET(src, QOS_POLICY_LIVELINESS)) {
        dst->liveliness = src->liveliness;
        QOS_POLICY_SET(dst, QOS_POLICY_LIVELINESS);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_RELIABLE) && QOS_POLICY_IS_SET(src, QOS_POLICY_RELIABLE)) {
        dst->reliability = src->reliability;
        QOS_POLICY_SET(dst, QOS_POLICY_RELIABLE);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_DESTINATION_ORDER) && QOS_POLICY_IS_SET(src, QOS_POLICY_DESTINATION_ORDER)) {
        dst->destination_order = src->destination_order;
        QOS_POLICY_SET(dst, QOS_POLICY_DESTINATION_ORDER);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_HISTORY) && QOS_POLICY_IS_SET(src, QOS_POLICY_HISTORY)) {
        dst->history = src->history;
        QOS_POLICY_SET(dst, QOS_POLICY_HISTORY);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_RESOURCE_LIMITS) && QOS_POLICY_IS_SET(src, QOS_POLICY_RESOURCE_LIMITS)) {
        dst->resource_limits = src->resource_limits;
        QOS_POLICY_SET(dst, QOS_POLICY_RESOURCE_LIMITS);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_TRANSPORT_PRIORITY) && QOS_POLICY_IS_SET(src, QOS_POLICY_TRANSPORT_PRIORITY)) {
        dst->transport_priority = src->transport_priority;
        QOS_POLICY_SET(dst, QOS_POLICY_TRANSPORT_PRIORITY);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_LIFESPAN) && QOS_POLICY_IS_SET(src, QOS_POLICY_LIFESPAN)) {
        dst->lifespan = src->lifespan;
        QOS_POLICY_SET(dst, QOS_POLICY_LIFESPAN);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_OWNERSHIP) && QOS_POLICY_IS_SET(src, QOS_POLICY_OWNERSHIP)) {
        dst->ownership = src->ownership;
        QOS_POLICY_SET(dst, QOS_POLICY_OWNERSHIP);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_OWNERSHIP_STRENGTH) && QOS_POLICY_IS_SET(src, QOS_POLICY_OWNERSHIP_STRENGTH)) {
        dst->ownership_strength = src->ownership_strength;
        QOS_POLICY_SET(dst, QOS_POLICY_OWNERSHIP_STRENGTH);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_WRITER_DATA_LIFECYCLE) && QOS_POLICY_IS_SET(src, QOS_POLICY_WRITER_DATA_LIFECYCLE)) {
        dst->writer_data_lifecycle = src->writer_data_lifecycle;
        QOS_POLICY_SET(dst, QOS_POLICY_WRITER_DATA_LIFECYCLE);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_TIME_BASED_FILTER) && QOS_POLICY_IS_SET(src, QOS_POLICY_TIME_BASED_FILTER)) {
        dst->time_based_filter = src->time_based_filter;
        QOS_POLICY_SET(dst, QOS_POLICY_TIME_BASED_FILTER);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_READER_DATA_LIFECYCLE) && QOS_POLICY_IS_SET(src, QOS_POLICY_READER_DATA_LIFECYCLE)) {
        dst->reader_data_lifecycle = src->reader_data_lifecycle;
        QOS_POLICY_SET(dst, QOS_POLICY_READER_DATA_LIFECYCLE);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_SUBSCRIPTION_KEYS) && QOS_POLICY_IS_SET(src, QOS_POLICY_USER_DATA)) {
        dds_qos_subscription_keys_policy_copy(&dst->subscription_keys, &src->subscription_keys);
        QOS_POLICY_SET(dst, QOS_POLICY_SUBSCRIPTION_KEYS);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_READER_LIFESPAN) && QOS_POLICY_IS_SET(src, QOS_POLICY_READER_LIFESPAN)) {
        dst->reader_lifespan = src->reader_lifespan;
        QOS_POLICY_SET(dst, QOS_POLICY_READER_LIFESPAN);
    }
    if (!QOS_POLICY_IS_SET(dst, QOS_POLICY_SHARE) && QOS_POLICY_IS_SET(src, QOS_POLICY_SHARE)) {
        dds_qos_share_policy_copy(&dst->share, &src->share);
        QOS_POLICY_SET(dst, QOS_POLICY_SHARE);
    }
}

void
dds_get_default_participant_qos (
    dds_qos_t * restrict qos)
{
    const DDS_DomainParticipantQos *defqos = DDS_PARTICIPANT_QOS_DEFAULT;

    if (qos) {
        dds_qos_userdata_policy_copy(&qos->user_data, &defqos->user_data);
        QOS_POLICY_SET(qos, QOS_POLICY_USER_DATA);
        qos->entity_factory = defqos->entity_factory;
        QOS_POLICY_SET(qos, QOS_POLICY_ENTITY_FACTORY);
        qos->watchdog_scheduling = defqos->watchdog_scheduling;
        QOS_POLICY_SET(qos, QOS_POLICY_WATCHDOC_SCHEDULING);
        qos->listener_scheduling = defqos->listener_scheduling;
        QOS_POLICY_SET(qos, QOS_POLICY_LISTENER_SCHEDULING);
    }
}

void
dds_get_default_topic_qos (
    dds_qos_t * restrict qos)
{
    const DDS_TopicQos *defqos = DDS_TOPIC_QOS_DEFAULT;

    if (qos) {
        dds_qos_topicdata_policy_copy(&qos->topic_data, &defqos->topic_data);
        QOS_POLICY_SET(qos, QOS_POLICY_TOPIC_DATA);
        qos->durability = defqos->durability;
        QOS_POLICY_SET(qos, QOS_POLICY_DURABILITY);
        qos->durability_service = defqos->durability_service;
        QOS_POLICY_SET(qos, QOS_POLICY_DURABILITY_SERVICE);
        qos->deadline = defqos->deadline;
        QOS_POLICY_SET(qos, QOS_POLICY_DEADLINE);
        qos->latency_budget = defqos->latency_budget;
        QOS_POLICY_SET(qos, QOS_POLICY_LATENCY_BUDGET);
        qos->liveliness = defqos->liveliness;
        QOS_POLICY_SET(qos, QOS_POLICY_LIVELINESS);
        qos->reliability = defqos->reliability;
        QOS_POLICY_SET(qos, QOS_POLICY_RELIABLE);
        qos->destination_order = defqos->destination_order;
        QOS_POLICY_SET(qos, QOS_POLICY_DESTINATION_ORDER);
        qos->history = defqos->history;
        QOS_POLICY_SET(qos, QOS_POLICY_HISTORY);
        qos->resource_limits = defqos->resource_limits;
        QOS_POLICY_SET(qos, QOS_POLICY_RESOURCE_LIMITS);
        qos->transport_priority = defqos->transport_priority;
        QOS_POLICY_SET(qos, QOS_POLICY_TRANSPORT_PRIORITY);
        qos->lifespan = defqos->lifespan;
        QOS_POLICY_SET(qos, QOS_POLICY_LIFESPAN);
        qos->ownership = defqos->ownership;
        QOS_POLICY_SET(qos, QOS_POLICY_OWNERSHIP);
    }
}

void
dds_get_default_publisher_qos (
    dds_qos_t * restrict qos)
{
    const DDS_PublisherQos *defqos = DDS_PUBLISHER_QOS_DEFAULT;

    if (qos) {
        qos->presentation = defqos->presentation;
        QOS_POLICY_SET(qos, QOS_POLICY_PRESENTATION);
        dds_qos_partition_policy_copy(&qos->partition, &defqos->partition);
        QOS_POLICY_SET(qos, QOS_POLICY_PARTITION);
        dds_qos_groupdata_policy_copy(&qos->group_data, &defqos->group_data);
        QOS_POLICY_SET(qos, QOS_POLICY_GROUP_DATA);
        qos->entity_factory = defqos->entity_factory;
        QOS_POLICY_SET(qos, QOS_POLICY_ENTITY_FACTORY);
    }
}

void
dds_get_default_subscriber_qos (
    dds_qos_t * restrict qos)
{
    const DDS_SubscriberQos *defqos = DDS_SUBSCRIBER_QOS_DEFAULT;

    if (qos) {
        qos->presentation = defqos->presentation;
        QOS_POLICY_SET(qos, QOS_POLICY_PRESENTATION);
        dds_qos_partition_policy_copy(&qos->partition, &defqos->partition);
        QOS_POLICY_SET(qos, QOS_POLICY_PARTITION);
        dds_qos_groupdata_policy_copy(&qos->group_data, &defqos->group_data);
        QOS_POLICY_SET(qos, QOS_POLICY_GROUP_DATA);
        qos->entity_factory = defqos->entity_factory;
        QOS_POLICY_SET(qos, QOS_POLICY_ENTITY_FACTORY);
        dds_qos_share_policy_copy(&qos->share, &defqos->share);
        QOS_POLICY_SET(qos, QOS_POLICY_GROUP_DATA);
    }
}

void
dds_get_default_writer_qos (
    dds_qos_t * restrict qos)
{
    const DDS_DataWriterQos *defqos = DDS_DATAWRITER_QOS_DEFAULT;

    if (qos) {
        qos->durability = defqos->durability;
        QOS_POLICY_SET(qos, QOS_POLICY_DURABILITY);
        qos->deadline = defqos->deadline;
        QOS_POLICY_SET(qos, QOS_POLICY_DEADLINE);
        qos->latency_budget = defqos->latency_budget;
        QOS_POLICY_SET(qos, QOS_POLICY_LATENCY_BUDGET);
        qos->liveliness = defqos->liveliness;
        QOS_POLICY_SET(qos, QOS_POLICY_LIVELINESS);
        qos->reliability = defqos->reliability;
        QOS_POLICY_SET(qos, QOS_POLICY_RELIABLE);
        qos->destination_order = defqos->destination_order;
        QOS_POLICY_SET(qos, QOS_POLICY_DESTINATION_ORDER);
        qos->history = defqos->history;
        QOS_POLICY_SET(qos, QOS_POLICY_HISTORY);
        qos->resource_limits = defqos->resource_limits;
        QOS_POLICY_SET(qos, QOS_POLICY_RESOURCE_LIMITS);
        qos->transport_priority = defqos->transport_priority;
        QOS_POLICY_SET(qos, QOS_POLICY_TRANSPORT_PRIORITY);
        qos->lifespan = defqos->lifespan;
        QOS_POLICY_SET(qos, QOS_POLICY_LIFESPAN);
        dds_qos_userdata_policy_copy(&qos->user_data, &defqos->user_data);
        QOS_POLICY_SET(qos, QOS_POLICY_USER_DATA);
        qos->ownership = defqos->ownership;
        QOS_POLICY_SET(qos, QOS_POLICY_OWNERSHIP);
        qos->ownership_strength = defqos->ownership_strength;
        QOS_POLICY_SET(qos, QOS_POLICY_OWNERSHIP_STRENGTH);
        qos->writer_data_lifecycle = defqos->writer_data_lifecycle;
        QOS_POLICY_SET(qos, QOS_POLICY_WRITER_DATA_LIFECYCLE);
    }
}

void
dds_get_default_reader_qos (
    dds_qos_t * restrict qos)
{
    const DDS_DataReaderQos *defqos = DDS_DATAREADER_QOS_DEFAULT;

    if (qos) {
        qos->durability = defqos->durability;
        QOS_POLICY_SET(qos, QOS_POLICY_DURABILITY);
        qos->deadline = defqos->deadline;
        QOS_POLICY_SET(qos, QOS_POLICY_DEADLINE);
        qos->latency_budget = defqos->latency_budget;
        QOS_POLICY_SET(qos, QOS_POLICY_LATENCY_BUDGET);
        qos->liveliness = defqos->liveliness;
        QOS_POLICY_SET(qos, QOS_POLICY_LIVELINESS);
        qos->reliability = defqos->reliability;
        QOS_POLICY_SET(qos, QOS_POLICY_RELIABLE);
        qos->destination_order = defqos->destination_order;
        QOS_POLICY_SET(qos, QOS_POLICY_DESTINATION_ORDER);
        qos->history = defqos->history;
        QOS_POLICY_SET(qos, QOS_POLICY_HISTORY);
        qos->resource_limits = defqos->resource_limits;
        QOS_POLICY_SET(qos, QOS_POLICY_RESOURCE_LIMITS);
        dds_qos_userdata_policy_copy(&qos->user_data, &defqos->user_data);
        QOS_POLICY_SET(qos, QOS_POLICY_USER_DATA);
        qos->ownership = defqos->ownership;
        QOS_POLICY_SET(qos, QOS_POLICY_OWNERSHIP);
        qos->time_based_filter = defqos->time_based_filter;
        QOS_POLICY_SET(qos, QOS_POLICY_TIME_BASED_FILTER);
        qos->reader_data_lifecycle = defqos->reader_data_lifecycle;
        QOS_POLICY_SET(qos, QOS_POLICY_READER_DATA_LIFECYCLE);
        qos->reader_lifespan = defqos->reader_lifespan;
        QOS_POLICY_SET(qos, QOS_POLICY_READER_LIFESPAN);
        dds_qos_subscription_keys_policy_copy(&qos->subscription_keys, &defqos->subscription_keys);
        QOS_POLICY_SET(qos, QOS_POLICY_SUBSCRIPTION_KEYS);
        dds_qos_share_policy_copy(&qos->share, &defqos->share);
        QOS_POLICY_SET(qos, QOS_POLICY_SHARE);
    }
}

void
dds_qset_userdata (
    dds_qos_t * restrict qos,
    const void * restrict value,
    size_t sz)
{
    if (qos) {
        dds_qos_data_seq_copy_in(&qos->user_data.value, value, sz);
        QOS_POLICY_SET(qos, QOS_POLICY_USER_DATA);
    }
}

void
dds_qset_topicdata (
    dds_qos_t * restrict qos,
    const void * restrict value,
    size_t sz)
{
    if (qos) {
        dds_qos_data_seq_copy_in(&qos->topic_data.value, value, sz);
        QOS_POLICY_SET(qos, QOS_POLICY_TOPIC_DATA);
    }
}

void
dds_qset_groupdata (dds_qos_t * restrict qos, const void * restrict value, size_t sz)
{
    if (qos) {
        dds_qos_data_seq_copy_in(&qos->group_data.value, value, sz);
        QOS_POLICY_SET(qos, QOS_POLICY_GROUP_DATA);
    }
}

void
dds_qset_durability (dds_qos_t *qos, dds_durability_kind_t kind)
{
    if (qos) {
        qos->durability.kind = (DDS_DurabilityQosPolicyKind)kind;
        QOS_POLICY_SET(qos, QOS_POLICY_DURABILITY);
    }
}

void
dds_qset_history (dds_qos_t *qos, dds_history_kind_t kind, int32_t depth)
{
    if (qos) {
        qos->history.kind = (DDS_HistoryQosPolicyKind)kind;
        qos->history.depth = depth;
        QOS_POLICY_SET(qos, QOS_POLICY_HISTORY);
    }
}

void
dds_qset_resource_limits (dds_qos_t *qos, int32_t max_samples, int32_t max_instances, int32_t max_samples_per_instance)
{
    if (qos) {
        qos->resource_limits.max_samples = max_samples;
        qos->resource_limits.max_instances = max_instances;
        qos->resource_limits.max_samples_per_instance = max_samples_per_instance;
        QOS_POLICY_SET(qos, QOS_POLICY_RESOURCE_LIMITS);
    }
}

void
dds_qset_presentation (dds_qos_t *qos, dds_presentation_access_scope_kind_t access_scope, bool coherent_access, bool ordered_access)
{
    if (qos) {
        qos->presentation.access_scope = (DDS_PresentationQosPolicyAccessScopeKind)access_scope;
        qos->presentation.coherent_access = coherent_access;
        qos->presentation.ordered_access = ordered_access;
        QOS_POLICY_SET(qos, QOS_POLICY_PRESENTATION);
    }
}

void
dds_qset_lifespan (dds_qos_t *qos, dds_duration_t lifespan)
{
    if (qos) {
        qos->lifespan.duration = dds_duration_to_sac(lifespan);
        QOS_POLICY_SET(qos, QOS_POLICY_LIFESPAN);
    }
}

void
dds_qset_deadline (dds_qos_t *qos, dds_duration_t deadline)
{
    if (qos) {
        qos->deadline.period = dds_duration_to_sac(deadline);
        QOS_POLICY_SET(qos, QOS_POLICY_DEADLINE);
    }
}

void
dds_qset_latency_budget (dds_qos_t *qos, dds_duration_t duration)
{
    if (qos) {
        qos->latency_budget.duration = dds_duration_to_sac(duration);
        QOS_POLICY_SET(qos, QOS_POLICY_LATENCY_BUDGET);
    }
}

void
dds_qset_ownership (dds_qos_t *qos, dds_ownership_kind_t kind)
{
    if (qos) {
        qos->ownership.kind = (DDS_OwnershipQosPolicyKind)kind;
        QOS_POLICY_SET(qos, QOS_POLICY_OWNERSHIP);
    }
}

void
dds_qset_ownership_strength (dds_qos_t *qos, int32_t value)
{
    if (qos) {
        qos->ownership_strength.value = value;
        QOS_POLICY_SET(qos, QOS_POLICY_OWNERSHIP_STRENGTH);
    }
}

void
dds_qset_liveliness (dds_qos_t *qos, dds_liveliness_kind_t kind, dds_duration_t lease_duration)
{
    if (qos) {
        qos->liveliness.kind = (DDS_LivelinessQosPolicyKind)kind;
        qos->liveliness.lease_duration = dds_duration_to_sac(lease_duration);
        QOS_POLICY_SET(qos, QOS_POLICY_LIVELINESS);
    }
}

void
dds_qset_time_based_filter (dds_qos_t *qos, dds_duration_t minimum_separation)
{
    if (qos) {
        qos->time_based_filter.minimum_separation = dds_duration_to_sac(minimum_separation);
        QOS_POLICY_SET(qos, QOS_POLICY_TIME_BASED_FILTER);
    }
}

void
dds_qset_partition (dds_qos_t * restrict qos, uint32_t n, const char ** ps)
{
    uint32_t i;

    if (qos) {
        if (n > 0) {
            qos->partition.name._buffer = DDS_StringSeq_allocbuf(sizeof(DDS_string) * n);
            for (i = 0; i < n; i++) {
                if (ps[i]) {
                    qos->partition.name._buffer[i] = DDS_string_dup(ps[i]);
                } else {
                    qos->partition.name._buffer[i] = NULL;
                }
            }
        }
        qos->partition.name._length = n;
        qos->partition.name._maximum = n;
        qos->partition.name._release = true;
        QOS_POLICY_SET(qos, QOS_POLICY_PARTITION);
    }
}

void
dds_qset_reliability (dds_qos_t *qos, dds_reliability_kind_t kind, dds_duration_t max_blocking_time)
{
    if (qos) {
        qos->reliability.kind = (DDS_ReliabilityQosPolicyKind)kind;
        qos->reliability.max_blocking_time = dds_duration_to_sac(max_blocking_time);
        qos->reliability.synchronous = false;
        QOS_POLICY_SET(qos, QOS_POLICY_RELIABLE);
    }
}

void
dds_qset_transport_priority (dds_qos_t *qos, int32_t value)
{
    if (qos) {
        qos->transport_priority.value = value;
        QOS_POLICY_SET(qos, QOS_POLICY_TRANSPORT_PRIORITY);
    }
}

void
dds_qset_destination_order (dds_qos_t *qos, dds_destination_order_kind_t kind)
{
    if (qos) {
        qos->destination_order.kind = (DDS_DestinationOrderQosPolicyKind)kind;
        QOS_POLICY_SET(qos, QOS_POLICY_DESTINATION_ORDER);
    }
}

void
dds_qset_writer_data_lifecycle (dds_qos_t *qos, bool autodispose_unregistered_instances)
{
    if (qos) {
        qos->writer_data_lifecycle.autodispose_unregistered_instances = autodispose_unregistered_instances;
        qos->writer_data_lifecycle.autopurge_suspended_samples_delay = dds_duration_to_sac(DDS_INFINITY);
        qos->writer_data_lifecycle.autounregister_instance_delay = dds_duration_to_sac(DDS_INFINITY);
        QOS_POLICY_SET(qos, QOS_POLICY_WRITER_DATA_LIFECYCLE);
    }
}

void
dds_qset_reader_data_lifecycle (
    dds_qos_t *qos,
    dds_duration_t autopurge_nowriter_samples,
    dds_duration_t autopurge_disposed_samples_delay)
{
    if (qos) {
        qos->reader_data_lifecycle.autopurge_nowriter_samples_delay = dds_duration_to_sac(autopurge_nowriter_samples);
        qos->reader_data_lifecycle.autopurge_disposed_samples_delay = dds_duration_to_sac(autopurge_disposed_samples_delay);
        qos->reader_data_lifecycle.autopurge_dispose_all = false;
        qos->reader_data_lifecycle.enable_invalid_samples = true;
        qos->reader_data_lifecycle.invalid_sample_visibility.kind = DDS_MINIMUM_INVALID_SAMPLES;
        QOS_POLICY_SET(qos, QOS_POLICY_READER_DATA_LIFECYCLE);
    }
}

void
dds_qset_durability_service (
    dds_qos_t * qos,
    dds_duration_t service_cleanup_delay,
    dds_history_kind_t history_kind,
    int32_t history_depth,
    int32_t max_samples,
    int32_t max_instances,
    int32_t max_samples_per_instance)
{
    if (qos) {
        qos->durability_service.service_cleanup_delay = dds_duration_to_sac(service_cleanup_delay);
        qos->durability_service.history_kind = (DDS_HistoryQosPolicyKind)history_kind;
        qos->durability_service.history_depth = history_depth;
        qos->durability_service.max_samples = max_samples;
        qos->durability_service.max_instances = max_instances;
        qos->durability_service.max_samples_per_instance = max_samples_per_instance;
        QOS_POLICY_SET(qos, QOS_POLICY_DURABILITY_SERVICE);
    }
}

void
dds_qget_userdata (const dds_qos_t *qos, void ** value, size_t *sz)
{
    if (qos) {
        dds_get_data_seq_out(&qos->user_data.value, value, sz);
    }
}

void
dds_qget_topicdata (const dds_qos_t *qos, void ** value, size_t *sz)
{
    if (qos) {
        dds_get_data_seq_out(&qos->topic_data.value, value, sz);
    }
}

void
dds_qget_groupdata (const dds_qos_t *qos, void ** value, size_t *sz)
{
    if (qos) {
        dds_get_data_seq_out(&qos->group_data.value, value, sz);
    }
}

void
dds_qget_durability (const dds_qos_t *qos, dds_durability_kind_t *kind)
{
    if (qos) {
        if (kind) {
            *kind = (dds_durability_kind_t)qos->durability.kind;
        }
    }
}

void
dds_qget_history (const dds_qos_t *qos, dds_history_kind_t *kind, int32_t * depth)
{
    if (qos) {
        if (kind) {
            *kind = (dds_history_kind_t)qos->history.kind;
        }
        if (depth) {
            *depth = qos->history.depth;
        }
    }
}

void
dds_qget_resource_limits (const dds_qos_t *qos, int32_t *max_samples, int32_t *max_instances, int32_t *max_samples_per_instance)
{
    if (qos) {
        if (max_samples) {
            *max_samples = qos->resource_limits.max_samples;
        }
        if (max_instances) {
            *max_instances = qos->resource_limits.max_instances;
        }
        if (max_samples_per_instance) {
            *max_samples_per_instance = qos->resource_limits.max_samples_per_instance;
        }
    }
}

void
dds_qget_presentation (const dds_qos_t *qos, dds_presentation_access_scope_kind_t *access_scope, bool *coherent_access, bool *ordered_access)
{
    if (qos) {
        if (access_scope) {
            *access_scope = (dds_presentation_access_scope_kind_t)qos->presentation.access_scope;
        }
        if (coherent_access) {
            *coherent_access = qos->presentation.coherent_access;
        }
        if (ordered_access) {
            *ordered_access = qos->presentation.ordered_access;
        }
    }
}

void
dds_qget_lifespan (const dds_qos_t *qos, dds_duration_t *lifespan)
{
    if (qos) {
        if (lifespan) {
            *lifespan = dds_duration_from_sac(qos->lifespan.duration);
        }
    }
}

void
dds_qget_deadline (const dds_qos_t *qos, dds_duration_t *deadline)
{
    if (qos) {
        if (deadline) {
            *deadline = dds_duration_from_sac(qos->deadline.period);
        }
    }
}

void
dds_qget_latency_budget (const dds_qos_t *qos, dds_duration_t *duration)
{
    if (qos) {
        if (duration) {
            *duration = dds_duration_from_sac(qos->latency_budget.duration);
        }
    }
}

void
dds_qget_ownership (const dds_qos_t *qos, dds_ownership_kind_t *kind)
{
    if (qos) {
        if (kind) {
            *kind = (dds_ownership_kind_t)qos->ownership.kind;
        }
    }
}

void
dds_qget_ownership_strength (const dds_qos_t *qos, int32_t *value)
{
    if (qos) {
        if (value) {
            *value = qos->ownership_strength.value;
        }
    }
}

void
dds_qget_liveliness (const dds_qos_t *qos, dds_liveliness_kind_t *kind, dds_duration_t *lease_duration)
{
    if (qos) {
        if (kind) {
            *kind = (dds_liveliness_kind_t)qos->liveliness.kind;
        }
        if (lease_duration) {
            *lease_duration = dds_duration_from_sac(qos->liveliness.lease_duration);
        }

    }
}

void
dds_qget_time_based_filter (const dds_qos_t *qos, dds_duration_t *minimum_separation)
{
    if (qos) {
        if (minimum_separation) {
            *minimum_separation = dds_duration_from_sac(qos->time_based_filter.minimum_separation);
        }
    }
}

void
dds_qget_partition (const dds_qos_t *qos, uint32_t *n, char *** ps)
{
    uint32_t i;

    if (qos) {
        if (ps) {
            if (qos->partition.name._buffer) {
                *ps = DDS_sequence_allocbuf((DDS_deallocatorType) NULL,
                                      sizeof(DDS_string), qos->partition.name._length);
                for (i = 0; i < qos->partition.name._length; i++) {
                    (*ps)[i] = DDS_string_dup(qos->partition.name._buffer[i]);
                }
            } else {
                *ps = NULL;
            }
        }
        if (n) {
            *n = qos->partition.name._length;
        }
    }
}

void
dds_qget_reliability (const dds_qos_t *qos, dds_reliability_kind_t *kind, dds_duration_t *max_blocking_time)
{
    if (qos) {
        if (kind) {
            *kind = (dds_reliability_kind_t)qos->reliability.kind;
        }
        if (max_blocking_time) {
            *max_blocking_time = dds_duration_from_sac(qos->reliability.max_blocking_time);
        }
    }
}

void
dds_qget_transport_priority (const dds_qos_t *qos, int32_t *value)
{
    if (qos) {
        if (value) {
            *value = qos->transport_priority.value;
        }
    }
}

void
dds_qget_destination_order (const dds_qos_t *qos, dds_destination_order_kind_t *value)
{
    if (qos) {
        if (value) {
            *value = (dds_destination_order_kind_t)qos->destination_order.kind;
        }
    }
}

void
dds_qget_writer_data_lifecycle (const dds_qos_t *qos, bool * autodispose_unregistered_instances)
{
    if (qos) {
        if (autodispose_unregistered_instances) {
            *autodispose_unregistered_instances = qos->writer_data_lifecycle.autodispose_unregistered_instances;
        }
    }
}

void
dds_qget_reader_data_lifecycle (const dds_qos_t *qos, dds_duration_t *autopurge_nowriter_samples, dds_duration_t *autopurge_disposed_samples_delay)
{
    if (qos) {
        if (autopurge_nowriter_samples) {
            *autopurge_nowriter_samples = dds_duration_from_sac(qos->reader_data_lifecycle.autopurge_nowriter_samples_delay);
        }
        if (autopurge_disposed_samples_delay) {
            *autopurge_disposed_samples_delay = dds_duration_from_sac(qos->reader_data_lifecycle.autopurge_disposed_samples_delay);
        }
    }
}

void
dds_qget_durability_service (
    const dds_qos_t * qos,
    dds_duration_t * service_cleanup_delay,
    dds_history_kind_t * history_kind,
    int32_t * history_depth,
    int32_t * max_samples,
    int32_t * max_instances,
    int32_t * max_samples_per_instance)
{
    if (qos) {
        if (service_cleanup_delay) {
           *service_cleanup_delay = dds_duration_from_sac(qos->durability_service.service_cleanup_delay);
        }
        if (history_kind) {
            *history_kind = (dds_history_kind_t)qos->durability_service.history_kind;
        }
        if (history_depth) {
            *history_depth = qos->durability_service.history_depth;
        }
        if (max_samples) {
            *max_samples = qos->durability_service.max_samples;
        }
        if (max_instances) {
            *max_instances = qos->durability_service.max_instances;
        }
        if (max_samples_per_instance) {
            *max_samples_per_instance = qos->durability_service.max_samples_per_instance;
        }
    }
}

void
dds_qos_to_participant_qos(
    DDS_DomainParticipantQos * restrict dst,
    const dds_qos_t * restrict src)
{
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_ENTITY_FACTORY)) {
        dst->entity_factory = src->entity_factory;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LISTENER_SCHEDULING)) {
        dst->listener_scheduling = src->listener_scheduling;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_USER_DATA)) {
        dds_qos_userdata_policy_copy(&dst->user_data, &src->user_data);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_WATCHDOC_SCHEDULING)) {
        dst->watchdog_scheduling = src->watchdog_scheduling;
    }
}

void
dds_qos_from_participant_qos(
    dds_qos_t * restrict dst,
    const DDS_DomainParticipantQos * restrict src)
{
    dst->entity_factory = src->entity_factory;
    QOS_POLICY_SET(dst, QOS_POLICY_ENTITY_FACTORY);
    dst->listener_scheduling = src->listener_scheduling;
    QOS_POLICY_SET(dst, QOS_POLICY_LISTENER_SCHEDULING);
    dds_qos_userdata_policy_copy(&dst->user_data, &src->user_data);
    QOS_POLICY_SET(dst, QOS_POLICY_USER_DATA);
    dst->watchdog_scheduling = src->watchdog_scheduling;
    QOS_POLICY_SET(dst, QOS_POLICY_WATCHDOC_SCHEDULING);
}

void
dds_qos_to_topic_qos(
    DDS_TopicQos * restrict dst,
    const dds_qos_t * restrict src)
{
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DEADLINE)) {
        dst->deadline = src->deadline;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DESTINATION_ORDER)) {
        dst->destination_order = src->destination_order;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DURABILITY)) {
        dst->durability = src->durability;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DURABILITY_SERVICE)) {
        dst->durability_service = src->durability_service;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_HISTORY)) {
        dst->history.kind = src->history.kind;
        dst->history.depth = ((src->history.kind == DDS_KEEP_LAST_HISTORY_QOS) ? src->history.depth : DDS_LENGTH_UNLIMITED);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LATENCY_BUDGET)) {
        dst->latency_budget = src->latency_budget;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LIFESPAN)) {
        dst->lifespan = src->lifespan;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LIVELINESS)) {
        dst->liveliness = src->liveliness;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_OWNERSHIP)) {
        dst->ownership = src->ownership;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_RELIABLE)) {
        dst->reliability = src->reliability;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_RESOURCE_LIMITS)) {
        dst->resource_limits = src->resource_limits;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_TOPIC_DATA)) {
        dds_qos_topicdata_policy_copy(&dst->topic_data, &src->topic_data);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_TRANSPORT_PRIORITY)) {
        dst->transport_priority = src->transport_priority;
    }
}

void
dds_qos_from_topic_qos(
    dds_qos_t * restrict dst,
    const DDS_TopicQos * restrict src)
{
    dst->deadline = src->deadline;
    QOS_POLICY_SET(dst, QOS_POLICY_DEADLINE);
    dst->destination_order = src->destination_order;
    QOS_POLICY_SET(dst, QOS_POLICY_DESTINATION_ORDER);
    dst->durability = src->durability;
    QOS_POLICY_SET(dst, QOS_POLICY_DURABILITY);
    dst->durability_service = src->durability_service;
    QOS_POLICY_SET(dst, QOS_POLICY_DURABILITY_SERVICE);
    dst->history = src->history;
    QOS_POLICY_SET(dst, QOS_POLICY_HISTORY);
    dst->latency_budget = src->latency_budget;
    QOS_POLICY_SET(dst, QOS_POLICY_LATENCY_BUDGET);
    dst->lifespan = src->lifespan;
    QOS_POLICY_SET(dst, QOS_POLICY_LIFESPAN);
    dst->liveliness = src->liveliness;
    QOS_POLICY_SET(dst, QOS_POLICY_LIVELINESS);
    dst->ownership = src->ownership;
    QOS_POLICY_SET(dst, QOS_POLICY_OWNERSHIP);
    dst->reliability = src->reliability;
    QOS_POLICY_SET(dst, QOS_POLICY_RELIABLE);
    dst->resource_limits = src->resource_limits;
    QOS_POLICY_SET(dst, QOS_POLICY_RESOURCE_LIMITS);
    dds_qos_topicdata_policy_copy(&dst->topic_data, &src->topic_data);
    QOS_POLICY_SET(dst, QOS_POLICY_TOPIC_DATA);
    dst->transport_priority = src->transport_priority;
    QOS_POLICY_SET(dst, QOS_POLICY_TRANSPORT_PRIORITY);
}

void
dds_qos_to_publisher_qos(
    DDS_PublisherQos * restrict dst,
    const dds_qos_t * restrict src)
{
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_ENTITY_FACTORY)) {
        dst->entity_factory = src->entity_factory;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_PRESENTATION)) {
        dst->presentation = src->presentation;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_PARTITION)) {
        dds_qos_partition_policy_copy(&dst->partition, &src->partition);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_GROUP_DATA)) {
        dds_qos_groupdata_policy_copy(&dst->group_data, &src->group_data);
    }
}

void
dds_qos_from_publisher_qos(
    dds_qos_t * restrict dst,
    const DDS_PublisherQos * restrict src)
{
    dst->entity_factory = src->entity_factory;
    QOS_POLICY_SET(dst, QOS_POLICY_ENTITY_FACTORY);
    dst->presentation = src->presentation;
    QOS_POLICY_SET(dst, QOS_POLICY_PRESENTATION);
    dds_qos_partition_policy_copy(&dst->partition, &src->partition);
    QOS_POLICY_SET(dst, QOS_POLICY_PARTITION);
    dds_qos_groupdata_policy_copy(&dst->group_data, &src->group_data);
    QOS_POLICY_SET(dst, QOS_POLICY_GROUP_DATA);
}

void
dds_qos_to_subscriber_qos(
    DDS_SubscriberQos * restrict dst,
    const dds_qos_t * restrict src)
{
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_ENTITY_FACTORY)) {
        dst->entity_factory = src->entity_factory;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_PRESENTATION)) {
        dst->presentation = src->presentation;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_PARTITION)) {
        dds_qos_partition_policy_copy(&dst->partition, &src->partition);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_GROUP_DATA)) {
        dds_qos_groupdata_policy_copy(&dst->group_data, &src->group_data);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_SHARE)) {
        dds_qos_share_policy_copy(&dst->share, &src->share);
    }
}

void
dds_qos_from_subscriber_qos(
    dds_qos_t * restrict dst,
    const DDS_SubscriberQos * restrict src)
{
    dst->entity_factory = src->entity_factory;
    QOS_POLICY_SET(dst, QOS_POLICY_ENTITY_FACTORY);
    dst->presentation = src->presentation;
    QOS_POLICY_SET(dst, QOS_POLICY_PRESENTATION);
    dds_qos_partition_policy_copy(&dst->partition, &src->partition);
    QOS_POLICY_SET(dst, QOS_POLICY_PARTITION);
    dds_qos_groupdata_policy_copy(&dst->group_data, &src->group_data);
    QOS_POLICY_SET(dst, QOS_POLICY_GROUP_DATA);
    dds_qos_share_policy_copy(&dst->share, &src->share);
    QOS_POLICY_SET(dst, QOS_POLICY_SHARE);
}

void
dds_qos_to_writer_qos(
    DDS_DataWriterQos * restrict dst,
    const dds_qos_t * restrict src)
{
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DEADLINE)) {
        dst->deadline = src->deadline;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DESTINATION_ORDER)) {
        dst->destination_order = src->destination_order;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DURABILITY)) {
        dst->durability = src->durability;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_HISTORY)) {
        dst->history.kind = src->history.kind;
        dst->history.depth = ((src->history.kind == DDS_KEEP_LAST_HISTORY_QOS) ? src->history.depth : DDS_LENGTH_UNLIMITED);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LATENCY_BUDGET)) {
        dst->latency_budget = src->latency_budget;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LIFESPAN)) {
        dst->lifespan = src->lifespan;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LIVELINESS)) {
        dst->liveliness = src->liveliness;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_OWNERSHIP)) {
        dst->ownership = src->ownership;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_RELIABLE)) {
        dst->reliability = src->reliability;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_RESOURCE_LIMITS)) {
        dst->resource_limits = src->resource_limits;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_USER_DATA)) {
        dds_qos_userdata_policy_copy(&dst->user_data, &src->user_data);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_TRANSPORT_PRIORITY)) {
        dst->transport_priority = src->transport_priority;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_OWNERSHIP_STRENGTH)) {
        dst->ownership_strength = src->ownership_strength;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_WRITER_DATA_LIFECYCLE)) {
        dst->writer_data_lifecycle = src->writer_data_lifecycle;
    }
}

void
dds_qos_from_writer_qos(
    dds_qos_t * restrict dst,
    const DDS_DataWriterQos * restrict src)
{
    dst->deadline = src->deadline;
    QOS_POLICY_SET(dst, QOS_POLICY_DEADLINE);
    dst->destination_order = src->destination_order;
    QOS_POLICY_SET(dst, QOS_POLICY_DESTINATION_ORDER);
    dst->durability = src->durability;
    QOS_POLICY_SET(dst, QOS_POLICY_DURABILITY);
    dst->history = src->history;
    QOS_POLICY_SET(dst, QOS_POLICY_HISTORY);
    dst->latency_budget = src->latency_budget;
    QOS_POLICY_SET(dst, QOS_POLICY_LATENCY_BUDGET);
    dst->lifespan = src->lifespan;
    QOS_POLICY_SET(dst, QOS_POLICY_LIFESPAN);
    dst->liveliness = src->liveliness;
    QOS_POLICY_SET(dst, QOS_POLICY_LIVELINESS);
    dst->ownership = src->ownership;
    QOS_POLICY_SET(dst, QOS_POLICY_OWNERSHIP);
    dst->reliability = src->reliability;
    QOS_POLICY_SET(dst, QOS_POLICY_RELIABLE);
    dst->resource_limits = src->resource_limits;
    QOS_POLICY_SET(dst, QOS_POLICY_RESOURCE_LIMITS);
    dds_qos_userdata_policy_copy(&dst->user_data, &src->user_data);
    QOS_POLICY_SET(dst, QOS_POLICY_USER_DATA);
    dst->transport_priority = src->transport_priority;
    QOS_POLICY_SET(dst, QOS_POLICY_TRANSPORT_PRIORITY);
    dst->ownership_strength = src->ownership_strength;
    QOS_POLICY_SET(dst, QOS_POLICY_OWNERSHIP_STRENGTH);
    dst->writer_data_lifecycle = src->writer_data_lifecycle;
    QOS_POLICY_SET(dst, QOS_POLICY_WRITER_DATA_LIFECYCLE);
}

void
dds_qos_to_reader_qos(
    DDS_DataReaderQos * restrict dst,
    const dds_qos_t * restrict src)
{
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DEADLINE)) {
        dst->deadline = src->deadline;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DESTINATION_ORDER)) {
        dst->destination_order = src->destination_order;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_DURABILITY)) {
        dst->durability = src->durability;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_HISTORY)) {
        dst->history.kind = src->history.kind;
        dst->history.depth = ((src->history.kind == DDS_KEEP_LAST_HISTORY_QOS) ? src->history.depth : DDS_LENGTH_UNLIMITED);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LATENCY_BUDGET)) {
        dst->latency_budget = src->latency_budget;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_READER_LIFESPAN)) {
        dst->reader_lifespan = src->reader_lifespan;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_LIVELINESS)) {
        dst->liveliness = src->liveliness;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_OWNERSHIP)) {
        dst->ownership = src->ownership;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_RELIABLE)) {
        dst->reliability = src->reliability;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_RESOURCE_LIMITS)) {
        dst->resource_limits = src->resource_limits;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_USER_DATA)) {
        dds_qos_userdata_policy_copy(&dst->user_data, &src->user_data);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_TIME_BASED_FILTER)) {
        dst->time_based_filter = src->time_based_filter;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_READER_DATA_LIFECYCLE)) {
        dst->reader_data_lifecycle = src->reader_data_lifecycle;
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_SHARE)) {
        dds_qos_share_policy_copy(&dst->share , &src->share);
    }
    if (QOS_POLICY_IS_SET(src, QOS_POLICY_SUBSCRIPTION_KEYS)) {
        dds_qos_subscription_keys_policy_copy(&dst->subscription_keys, &src->subscription_keys);
    }
}

void
dds_qos_from_reader_qos(
    dds_qos_t * restrict dst,
    const DDS_DataReaderQos * restrict src)
{
    dst->deadline = src->deadline;
    QOS_POLICY_SET(dst, QOS_POLICY_DEADLINE);
    dst->destination_order = src->destination_order;
    QOS_POLICY_SET(dst, QOS_POLICY_DESTINATION_ORDER);
    dst->durability = src->durability;
    QOS_POLICY_SET(dst, QOS_POLICY_DURABILITY);
    dst->history = src->history;
    QOS_POLICY_SET(dst, QOS_POLICY_HISTORY);
    dst->latency_budget = src->latency_budget;
    QOS_POLICY_SET(dst, QOS_POLICY_LATENCY_BUDGET);
    dst->reader_lifespan = src->reader_lifespan;
    QOS_POLICY_SET(dst, QOS_POLICY_READER_LIFESPAN);
    dst->liveliness = src->liveliness;
    QOS_POLICY_SET(dst, QOS_POLICY_LIVELINESS);
    dst->ownership = src->ownership;
    QOS_POLICY_SET(dst, QOS_POLICY_OWNERSHIP);
    dst->reliability = src->reliability;
    QOS_POLICY_SET(dst, QOS_POLICY_RELIABLE);
    dst->resource_limits = src->resource_limits;
    QOS_POLICY_SET(dst, QOS_POLICY_RESOURCE_LIMITS);
    dds_qos_userdata_policy_copy(&dst->user_data, &src->user_data);
    QOS_POLICY_SET(dst, QOS_POLICY_USER_DATA);
    dst->time_based_filter = src->time_based_filter;
    QOS_POLICY_SET(dst, QOS_POLICY_TIME_BASED_FILTER);
    dst->reader_data_lifecycle = src->reader_data_lifecycle;
    QOS_POLICY_SET(dst, QOS_POLICY_READER_DATA_LIFECYCLE);
    dds_qos_share_policy_copy(&dst->share , &src->share);
    QOS_POLICY_SET(dst, QOS_POLICY_SHARE);
    dds_qos_subscription_keys_policy_copy(&dst->subscription_keys, &src->subscription_keys);
    QOS_POLICY_SET(dst, QOS_POLICY_SUBSCRIPTION_KEYS);
}
