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
#ifndef DDS_DCPSSPLTYPES_H
#define DDS_DCPSSPLTYPES_H

#include "c_base.h"
#include "c_misc.h"
#include "c_sync.h"
#include "c_collection.h"
#include "c_field.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DCPSSAC
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef c_sequence _DDS_octSeq;
typedef c_long _DDS_BuiltinTopicKey_t[3];
typedef c_sequence _DDS_StringSeq;

enum _DDS_InvalidSampleVisibilityQosPolicyKind {
    _DDS_NO_INVALID_SAMPLES,
    _DDS_MINIMUM_INVALID_SAMPLES,
    _DDS_ALL_INVALID_SAMPLES
};

enum _DDS_SchedulingClassQosPolicyKind {
    _DDS_SCHEDULE_DEFAULT,
    _DDS_SCHEDULE_TIMESHARING,
    _DDS_SCHEDULE_REALTIME
};

enum _DDS_SchedulingPriorityQosPolicyKind {
    _DDS_PRIORITY_RELATIVE,
    _DDS_PRIORITY_ABSOLUTE
};

enum _DDS_DurabilityQosPolicyKind {
    _DDS_VOLATILE_DURABILITY_QOS,
    _DDS_TRANSIENT_LOCAL_DURABILITY_QOS,
    _DDS_TRANSIENT_DURABILITY_QOS,
    _DDS_PERSISTENT_DURABILITY_QOS
};

enum _DDS_PresentationQosPolicyAccessScopeKind {
    _DDS_INSTANCE_PRESENTATION_QOS,
    _DDS_TOPIC_PRESENTATION_QOS,
    _DDS_GROUP_PRESENTATION_QOS
};

enum _DDS_OwnershipQosPolicyKind {
    _DDS_SHARED_OWNERSHIP_QOS,
    _DDS_EXCLUSIVE_OWNERSHIP_QOS
};

enum _DDS_LivelinessQosPolicyKind {
    _DDS_AUTOMATIC_LIVELINESS_QOS,
    _DDS_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
    _DDS_MANUAL_BY_TOPIC_LIVELINESS_QOS
};

enum _DDS_ReliabilityQosPolicyKind {
    _DDS_BEST_EFFORT_RELIABILITY_QOS,
    _DDS_RELIABLE_RELIABILITY_QOS
};

enum _DDS_DestinationOrderQosPolicyKind {
    _DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
    _DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
};

enum _DDS_HistoryQosPolicyKind {
    _DDS_KEEP_LAST_HISTORY_QOS,
    _DDS_KEEP_ALL_HISTORY_QOS
};

struct _DDS_Duration_t {
    c_long sec;
    c_ulong nanosec;
};

struct _DDS_Time_t {
    c_long sec;
    c_ulong nanosec;
};

struct _DDS_EntityFactoryQosPolicy {
    c_bool autoenable_created_entities;
};

struct _DDS_WriterDataLifecycleQosPolicy {
    c_bool autodispose_unregistered_instances;
    struct _DDS_Duration_t autopurge_suspended_samples_delay;
    struct _DDS_Duration_t autounregister_instance_delay;
};

struct _DDS_InvalidSampleVisibilityQosPolicy {
    enum _DDS_InvalidSampleVisibilityQosPolicyKind kind;
};

struct _DDS_ReaderDataLifecycleQosPolicy {
    struct _DDS_Duration_t autopurge_nowriter_samples_delay;
    struct _DDS_Duration_t autopurge_disposed_samples_delay;
    c_bool enable_invalid_samples;
    struct _DDS_InvalidSampleVisibilityQosPolicy invalid_sample_visibility;
};

struct _DDS_SubscriptionKeyQosPolicy {
    c_bool use_key_list;
    _DDS_StringSeq key_list;
};

struct _DDS_ReaderLifespanQosPolicy {
    c_bool use_lifespan;
    struct _DDS_Duration_t duration;
};

struct _DDS_ShareQosPolicy {
    c_string name;
    c_bool enable;
};

struct _DDS_SchedulingClassQosPolicy {
    enum _DDS_SchedulingClassQosPolicyKind kind;
};

struct _DDS_SchedulingPriorityQosPolicy {
    enum _DDS_SchedulingPriorityQosPolicyKind kind;
};

struct _DDS_SchedulingQosPolicy {
    struct _DDS_SchedulingClassQosPolicy scheduling_class;
    struct _DDS_SchedulingPriorityQosPolicy scheduling_priority_kind;
    c_long scheduling_priority;
};

struct _DDS_ViewKeyQosPolicy {
    c_bool use_key_list;
    _DDS_StringSeq key_list;
};

struct _DDS_DataReaderViewQos {
    struct _DDS_ViewKeyQosPolicy view_keys;
};

struct _DDS_DomainParticipantFactoryQos {
    struct _DDS_EntityFactoryQosPolicy entity_factory;
};

struct _DDS_UserDataQosPolicy {
    _DDS_octSeq value;
};

struct _DDS_TopicDataQosPolicy {
    _DDS_octSeq value;
};

struct _DDS_GroupDataQosPolicy {
    _DDS_octSeq value;
};

struct _DDS_TransportPriorityQosPolicy {
    c_long value;
};

struct _DDS_LifespanQosPolicy {
    struct _DDS_Duration_t duration;
};

struct _DDS_DurabilityQosPolicy {
    enum _DDS_DurabilityQosPolicyKind kind;
};

struct _DDS_PresentationQosPolicy {
    enum _DDS_PresentationQosPolicyAccessScopeKind access_scope;
    c_bool coherent_access;
    c_bool ordered_access;
};

struct _DDS_DeadlineQosPolicy {
    struct _DDS_Duration_t period;
};

struct _DDS_LatencyBudgetQosPolicy {
    struct _DDS_Duration_t duration;
};

struct _DDS_OwnershipQosPolicy {
    enum _DDS_OwnershipQosPolicyKind kind;
};

struct _DDS_OwnershipStrengthQosPolicy {
    c_long value;
};

struct _DDS_LivelinessQosPolicy {
    enum _DDS_LivelinessQosPolicyKind kind;
    struct _DDS_Duration_t lease_duration;
};

struct _DDS_TimeBasedFilterQosPolicy {
    struct _DDS_Duration_t minimum_separation;
};

struct _DDS_PartitionQosPolicy {
    _DDS_StringSeq name;
};

struct _DDS_ReliabilityQosPolicy {
    enum _DDS_ReliabilityQosPolicyKind kind;
    struct _DDS_Duration_t max_blocking_time;
    c_bool synchronous;
};

struct _DDS_DestinationOrderQosPolicy {
    enum _DDS_DestinationOrderQosPolicyKind kind;
};

struct _DDS_HistoryQosPolicy {
    enum _DDS_HistoryQosPolicyKind kind;
    c_long depth;
};

struct _DDS_ResourceLimitsQosPolicy {
    c_long max_samples;
    c_long max_instances;
    c_long max_samples_per_instance;
};

struct _DDS_DurabilityServiceQosPolicy {
    struct _DDS_Duration_t service_cleanup_delay;
    enum _DDS_HistoryQosPolicyKind history_kind;
    c_long history_depth;
    c_long max_samples;
    c_long max_instances;
    c_long max_samples_per_instance;
};

struct _DDS_ParticipantBuiltinTopicData {
    _DDS_BuiltinTopicKey_t key;
    struct _DDS_UserDataQosPolicy user_data;
};

struct _DDS_DomainParticipantQos {
    struct _DDS_UserDataQosPolicy user_data;
    struct _DDS_EntityFactoryQosPolicy entity_factory;
    struct _DDS_SchedulingQosPolicy watchdog_scheduling;
    struct _DDS_SchedulingQosPolicy listener_scheduling;
};
struct _DDS_TopicQos {
    struct _DDS_TopicDataQosPolicy topic_data;
    struct _DDS_DurabilityQosPolicy durability;
    struct _DDS_DurabilityServiceQosPolicy durability_service;
    struct _DDS_DeadlineQosPolicy deadline;
    struct _DDS_LatencyBudgetQosPolicy latency_budget;
    struct _DDS_LivelinessQosPolicy liveliness;
    struct _DDS_ReliabilityQosPolicy reliability;
    struct _DDS_DestinationOrderQosPolicy destination_order;
    struct _DDS_HistoryQosPolicy history;
    struct _DDS_ResourceLimitsQosPolicy resource_limits;
    struct _DDS_TransportPriorityQosPolicy transport_priority;
    struct _DDS_LifespanQosPolicy lifespan;
    struct _DDS_OwnershipQosPolicy ownership;
};

struct _DDS_DataWriterQos {
    struct _DDS_DurabilityQosPolicy durability;
    struct _DDS_DeadlineQosPolicy deadline;
    struct _DDS_LatencyBudgetQosPolicy latency_budget;
    struct _DDS_LivelinessQosPolicy liveliness;
    struct _DDS_ReliabilityQosPolicy reliability;
    struct _DDS_DestinationOrderQosPolicy destination_order;
    struct _DDS_HistoryQosPolicy history;
    struct _DDS_ResourceLimitsQosPolicy resource_limits;
    struct _DDS_TransportPriorityQosPolicy transport_priority;
    struct _DDS_LifespanQosPolicy lifespan;
    struct _DDS_UserDataQosPolicy user_data;
    struct _DDS_OwnershipQosPolicy ownership;
    struct _DDS_OwnershipStrengthQosPolicy ownership_strength;
    struct _DDS_WriterDataLifecycleQosPolicy writer_data_lifecycle;
};

struct _DDS_PublisherQos {
    struct _DDS_PresentationQosPolicy presentation;
    struct _DDS_PartitionQosPolicy partition;
    struct _DDS_GroupDataQosPolicy group_data;
    struct _DDS_EntityFactoryQosPolicy entity_factory;
};

struct _DDS_DataReaderQos {
    struct _DDS_DurabilityQosPolicy durability;
    struct _DDS_DeadlineQosPolicy deadline;
    struct _DDS_LatencyBudgetQosPolicy latency_budget;
    struct _DDS_LivelinessQosPolicy liveliness;
    struct _DDS_ReliabilityQosPolicy reliability;
    struct _DDS_DestinationOrderQosPolicy destination_order;
    struct _DDS_HistoryQosPolicy history;
    struct _DDS_ResourceLimitsQosPolicy resource_limits;
    struct _DDS_UserDataQosPolicy user_data;
    struct _DDS_OwnershipQosPolicy ownership;
    struct _DDS_TimeBasedFilterQosPolicy time_based_filter;
    struct _DDS_ReaderDataLifecycleQosPolicy reader_data_lifecycle;
    struct _DDS_SubscriptionKeyQosPolicy subscription_keys;
    struct _DDS_ReaderLifespanQosPolicy reader_lifespan;
    struct _DDS_ShareQosPolicy share;
};

struct _DDS_SubscriberQos {
    struct _DDS_PresentationQosPolicy presentation;
    struct _DDS_PartitionQosPolicy partition;
    struct _DDS_GroupDataQosPolicy group_data;
    struct _DDS_EntityFactoryQosPolicy entity_factory;
    struct _DDS_ShareQosPolicy share;
};

OS_API c_bool
__DDS_Duration_t__copyIn(
    c_base base,
    void *_from,
    void *_to);

OS_API c_bool
__DDS_Time_t__copyIn(
    c_base base,
    void *_from,
    void *_to);

OS_API c_bool
__DDS_EntityFactoryQosPolicy__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_WriterDataLifecycleQosPolicy__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_InvalidSampleVisibilityQosPolicy__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_ReaderDataLifecycleQosPolicy__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_SubscriptionKeyQosPolicy__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_ReaderLifespanQosPolicy__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_ShareQosPolicy__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_SchedulingClassQosPolicy__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_SchedulingPriorityQosPolicy__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_SchedulingQosPolicy__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_ViewKeyQosPolicy__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool __DDS_UserDataQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_TopicDataQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_GroupDataQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_TransportPriorityQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_LifespanQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_DurabilityQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_PresentationQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_DeadlineQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_LatencyBudgetQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_OwnershipQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_OwnershipStrengthQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_LivelinessQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_TimeBasedFilterQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_PartitionQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_ReliabilityQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_DestinationOrderQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_HistoryQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_ResourceLimitsQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_DurabilityServiceQosPolicy__copyIn(c_base base, void *from, void *to);
OS_API c_bool __DDS_ParticipantBuiltinTopicData__copyIn(c_base base, void *from, void *to);

OS_API c_bool
__DDS_DataReaderViewQos__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_DomainParticipantFactoryQos__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_DomainParticipantQos__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_TopicQos__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_DataWriterQos__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_PublisherQos__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_DataReaderQos__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API c_bool
__DDS_SubscriberQos__copyIn(
    c_base base,
    void *from,
    void *to);

OS_API void
__DDS_Duration_t__copyOut(
    void *_from,
    void *_to);

OS_API void
__DDS_Time_t__copyOut(
    void *_from,
    void *_to);

OS_API void
__DDS_EntityFactoryQosPolicy__copyOut(
    void *_from,
    void *_to);

OS_API void __DDS_WriterDataLifecycleQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_InvalidSampleVisibilityQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_ReaderDataLifecycleQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_SubscriptionKeyQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_ReaderLifespanQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_ShareQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_SchedulingClassQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_SchedulingPriorityQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_SchedulingQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_ViewKeyQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_UserDataQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_TopicDataQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_GroupDataQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_TransportPriorityQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_LifespanQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_DurabilityQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_PresentationQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_DeadlineQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_LatencyBudgetQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_OwnershipQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_OwnershipStrengthQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_LivelinessQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_TimeBasedFilterQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_PartitionQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_ReliabilityQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_DestinationOrderQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_HistoryQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_ResourceLimitsQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_DurabilityServiceQosPolicy__copyOut(void *_from, void *_to);
OS_API void __DDS_ParticipantBuiltinTopicData__copyOut(void *_from, void *_to);
OS_API void __DDS_DomainParticipantFactoryQos__copyOut(void *_from, void *_to);
OS_API void __DDS_DataReaderViewQos__copyOut(void *_from, void *_to);
OS_API void __DDS_DomainParticipantQos__copyOut(void *_from, void *_to);
OS_API void __DDS_TopicQos__copyOut(void *_from, void *_to);
OS_API void __DDS_DataWriterQos__copyOut(void *_from, void *_to);
OS_API void __DDS_PublisherQos__copyOut(void *_from, void *_to);
OS_API void __DDS_DataReaderQos__copyOut(void *_from, void *_to);
OS_API void __DDS_SubscriberQos__copyOut(void *_from, void *_to);

struct _DDS_NamedDomainParticipantQos {
    c_string name;
    struct _DDS_DomainParticipantQos domainparticipant_qos;
};

struct _DDS_NamedPublisherQos {
    c_string name;
    struct _DDS_PublisherQos publisher_qos;
};

struct _DDS_NamedSubscriberQos {
    c_string name;
    struct _DDS_SubscriberQos subscriber_qos;
};

struct _DDS_NamedTopicQos {
    c_string name;
    struct _DDS_TopicQos topic_qos;
};

struct _DDS_NamedDataWriterQos {
    c_string name;
    struct _DDS_DataWriterQos datawriter_qos;
};

struct _DDS_NamedDataReaderQos {
    c_string name;
    struct _DDS_DataReaderQos datareader_qos;
};

OS_API void
__DDS_NamedDomainParticipantQos__copyOut(
    void *_from,
    void *_to);

OS_API void
__DDS_NamedPublisherQos__copyOut(
    void *_from,
    void *_to);

OS_API void
__DDS_NamedSubscriberQos__copyOut(
    void *_from,
    void *_to);

OS_API void
__DDS_NamedTopicQos__copyOut(
    void *_from,
    void *_to);

OS_API void
__DDS_NamedDataWriterQos__copyOut(
    void *_from,
    void *_to);

OS_API void
__DDS_NamedDataReaderQos__copyOut(
    void *_from,
    void *_to);

#undef OS_API

#endif /* DDS_DCPSSPLTYPES_H */
