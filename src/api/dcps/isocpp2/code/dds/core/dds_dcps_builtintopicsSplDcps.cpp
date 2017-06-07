/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
 */
#include "dds/core/detail/dds_dcps_builtintopics_DCPS.hpp"

v_copyin_result
__DDS_Time_t__copyIn(
    c_base base,
    const dds::core::Time *from,
    struct _DDS_Time_t *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;
    (void) base;

    to->sec = (c_long)from->sec();
    to->nanosec = (c_ulong)from->nanosec();
    return result;
}

v_copyin_result
__DDS_SchedulingClassQosPolicy__copyIn(
    c_base base,
    const DDS::SchedulingClassQosPolicy *from,
    struct _DDS_SchedulingClassQosPolicy *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;
    OS_UNUSED_ARG(base);

    if(((c_long)(*from) >= 0) && ((c_long)(*from) < 3) ){
        to->kind = (enum _DDS_SchedulingClassQosPolicyKind)(*from);
    } else {
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::SchedulingClassQosPolicy' of type 'org::opensplice::core::policy::SchedulingKind::Type' is out of range.");
        result = V_COPYIN_RESULT_INVALID;
    }
    return result;
}

v_copyin_result
__DDS_SchedulingPriorityQosPolicy__copyIn(
    c_base base,
    const DDS::SchedulingPriorityQosPolicy *from,
    struct _DDS_SchedulingPriorityQosPolicy *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;
    OS_UNUSED_ARG(base);

    if((c_long)(*from) >= 0 && (c_long)(*from) < 2 ){
        to->kind = (enum _DDS_SchedulingPriorityQosPolicyKind)(*from);
    } else {
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::SchedulingPriorityQosPolicy' of type 'org::opensplice::core::policy::SchedulingPriorityKind::Type' is out of range.");
        result = V_COPYIN_RESULT_INVALID;
    }
    return result;
}

v_copyin_result
__DDS_SchedulingQosPolicy__copyIn(
    c_base base,
    const DDS::SchedulingQosPolicy *from,
    struct _DDS_SchedulingQosPolicy *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;

    if(V_COPYIN_RESULT_IS_OK(result)){
        org::opensplice::core::policy::SchedulingKind::Type scheduling_kind = from->scheduling_kind();
        result = __DDS_SchedulingClassQosPolicy__copyIn(base, &scheduling_kind, &to->scheduling_class);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        org::opensplice::core::policy::SchedulingPriorityKind::Type priority_kind = from->scheduling_priority_kind();
        result = __DDS_SchedulingPriorityQosPolicy__copyIn(base, &priority_kind, &to->scheduling_priority_kind);
    }
    to->scheduling_priority = (c_long)from->scheduling_priority();
    return result;
}

v_copyin_result
__DDS_DomainParticipantQos__copyIn(
    c_base base,
    const dds::domain::qos::DomainParticipantQos *from,
    struct _DDS_DomainParticipantQos *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;

    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_UserDataQosPolicy__copyIn(base, &from->policy<dds::core::policy::UserData>(), &to->user_data);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_EntityFactoryQosPolicy__copyIn(base, &from->policy<dds::core::policy::EntityFactory>(), &to->entity_factory);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_SchedulingQosPolicy__copyIn(base, &(*from)->policy<org::opensplice::core::policy::WatchdogScheduling>(), &to->watchdog_scheduling);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_SchedulingQosPolicy__copyIn(base, &(*from)->policy<org::opensplice::core::policy::ListenerScheduling>(), &to->listener_scheduling);
    }
    return result;
}

v_copyin_result
__DDS_TopicQos__copyIn(
    c_base base,
    const dds::topic::qos::TopicQos *from,
    struct _DDS_TopicQos *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;

    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_TopicDataQosPolicy__copyIn(base, &from->policy<dds::core::policy::TopicData>(), &to->topic_data);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_DurabilityQosPolicy__copyIn(base, &from->policy<dds::core::policy::Durability>(), &to->durability);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_DurabilityServiceQosPolicy__copyIn(base, &from->policy<dds::core::policy::DurabilityService>(), &to->durability_service);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_DeadlineQosPolicy__copyIn(base, &from->policy<dds::core::policy::Deadline>(), &to->deadline);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_LatencyBudgetQosPolicy__copyIn(base, &from->policy<dds::core::policy::LatencyBudget>(), &to->latency_budget);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_LivelinessQosPolicy__copyIn(base, &from->policy<dds::core::policy::Liveliness>(), &to->liveliness);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_ReliabilityQosPolicy__copyIn(base, &from->policy<dds::core::policy::Reliability>(), &to->reliability);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_DestinationOrderQosPolicy__copyIn(base, &from->policy<dds::core::policy::DestinationOrder>(), &to->destination_order);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_HistoryQosPolicy__copyIn(base, &from->policy<dds::core::policy::History>(), &to->history);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_ResourceLimitsQosPolicy__copyIn(base, &from->policy<dds::core::policy::ResourceLimits>(), &to->resource_limits);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_TransportPriorityQosPolicy__copyIn(base, &from->policy<dds::core::policy::TransportPriority>(), &to->transport_priority);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_LifespanQosPolicy__copyIn(base, &from->policy<dds::core::policy::Lifespan>(), &to->lifespan);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_OwnershipQosPolicy__copyIn(base, &from->policy<dds::core::policy::Ownership>(), &to->ownership);
    }
    return result;
}

v_copyin_result
__DDS_DataWriterQos__copyIn(
    c_base base,
    const dds::pub::qos::DataWriterQos *from,
    struct _DDS_DataWriterQos *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;

    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_DurabilityQosPolicy__copyIn(base, &from->policy<dds::core::policy::Durability>(), &to->durability);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_DeadlineQosPolicy__copyIn(base, &from->policy<dds::core::policy::Deadline>(), &to->deadline);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_LatencyBudgetQosPolicy__copyIn(base, &from->policy<dds::core::policy::LatencyBudget>(), &to->latency_budget);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_LivelinessQosPolicy__copyIn(base, &from->policy<dds::core::policy::Liveliness>(), &to->liveliness);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_ReliabilityQosPolicy__copyIn(base, &from->policy<dds::core::policy::Reliability>(), &to->reliability);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_DestinationOrderQosPolicy__copyIn(base, &from->policy<dds::core::policy::DestinationOrder>(), &to->destination_order);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_HistoryQosPolicy__copyIn(base, &from->policy<dds::core::policy::History>(), &to->history);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_ResourceLimitsQosPolicy__copyIn(base, &from->policy<dds::core::policy::ResourceLimits>(), &to->resource_limits);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_TransportPriorityQosPolicy__copyIn(base, &from->policy<dds::core::policy::TransportPriority>(), &to->transport_priority);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_LifespanQosPolicy__copyIn(base, &from->policy<dds::core::policy::Lifespan>(), &to->lifespan);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_UserDataQosPolicy__copyIn(base, &from->policy<dds::core::policy::UserData>(), &to->user_data);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_OwnershipQosPolicy__copyIn(base, &from->policy<dds::core::policy::Ownership>(), &to->ownership);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_OwnershipStrengthQosPolicy__copyIn(base, &from->policy<dds::core::policy::OwnershipStrength>(), &to->ownership_strength);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_WriterDataLifecycleQosPolicy__copyIn(base, &from->policy<dds::core::policy::WriterDataLifecycle>(), &to->writer_data_lifecycle);
    }
    return result;
}

v_copyin_result
__DDS_PublisherQos__copyIn(
    c_base base,
    const dds::pub::qos::PublisherQos *from,
    struct _DDS_PublisherQos *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;

    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_PresentationQosPolicy__copyIn(base, &from->policy<dds::core::policy::Presentation>(), &to->presentation);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_PartitionQosPolicy__copyIn(base, &from->policy<dds::core::policy::Partition>(), &to->partition);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_GroupDataQosPolicy__copyIn(base, &from->policy<dds::core::policy::GroupData>(), &to->group_data);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_EntityFactoryQosPolicy__copyIn(base, &from->policy<dds::core::policy::EntityFactory>(), &to->entity_factory);
    }
    return result;
}

v_copyin_result
__DDS_DataReaderQos__copyIn(
    c_base base,
    const dds::sub::qos::DataReaderQos *from,
    struct _DDS_DataReaderQos *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;

    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_DurabilityQosPolicy__copyIn(base, &from->policy<dds::core::policy::Durability>(), &to->durability);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_DeadlineQosPolicy__copyIn(base, &from->policy<dds::core::policy::Deadline>(), &to->deadline);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_LatencyBudgetQosPolicy__copyIn(base, &from->policy<dds::core::policy::LatencyBudget>(), &to->latency_budget);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_LivelinessQosPolicy__copyIn(base, &from->policy<dds::core::policy::Liveliness>(), &to->liveliness);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_ReliabilityQosPolicy__copyIn(base, &from->policy<dds::core::policy::Reliability>(), &to->reliability);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_DestinationOrderQosPolicy__copyIn(base, &from->policy<dds::core::policy::DestinationOrder>(), &to->destination_order);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_HistoryQosPolicy__copyIn(base, &from->policy<dds::core::policy::History>(), &to->history);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_ResourceLimitsQosPolicy__copyIn(base, &from->policy<dds::core::policy::ResourceLimits>(), &to->resource_limits);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_UserDataQosPolicy__copyIn(base, &from->policy<dds::core::policy::UserData>(), &to->user_data);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_OwnershipQosPolicy__copyIn(base, &from->policy<dds::core::policy::Ownership>(), &to->ownership);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_TimeBasedFilterQosPolicy__copyIn(base, &from->policy<dds::core::policy::TimeBasedFilter>(), &to->time_based_filter);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_ReaderDataLifecycleQosPolicy__copyIn(base, &from->policy<dds::core::policy::ReaderDataLifecycle>(), &to->reader_data_lifecycle);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_SubscriptionKeyQosPolicy__copyIn(base, &from->policy<org::opensplice::core::policy::SubscriptionKey>(), &to->subscription_keys);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_ReaderLifespanQosPolicy__copyIn(base, &from->policy<org::opensplice::core::policy::ReaderLifespan>(), &to->reader_lifespan);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_ShareQosPolicy__copyIn(base, &from->policy<org::opensplice::core::policy::Share>(), &to->share);
    }
    return result;
}

v_copyin_result
__DDS_SubscriberQos__copyIn(
    c_base base,
    const dds::sub::qos::SubscriberQos *from,
    struct _DDS_SubscriberQos *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;

    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_PresentationQosPolicy__copyIn(base, &from->policy<dds::core::policy::Presentation>(), &to->presentation);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_PartitionQosPolicy__copyIn(base, &from->policy<dds::core::policy::Partition>(), &to->partition);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_GroupDataQosPolicy__copyIn(base, &from->policy<dds::core::policy::GroupData>(), &to->group_data);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_EntityFactoryQosPolicy__copyIn(base, &from->policy<dds::core::policy::EntityFactory>(), &to->entity_factory);
    }
    if(V_COPYIN_RESULT_IS_OK(result)){
        result = __DDS_ShareQosPolicy__copyIn(base, &from->policy<org::opensplice::core::policy::Share>(), &to->share);
    }
    return result;
}

void
__DDS_Time_t__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_Time_t *from = (const struct _DDS_Time_t *)_from;
    dds::core::Time *to = (dds::core::Time *)_to;
    to->sec((int64_t) from->sec);
    to->nanosec((uint32_t) from->nanosec);
}

void
__DDS_SchedulingClassQosPolicy__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_SchedulingClassQosPolicy *from = (const struct _DDS_SchedulingClassQosPolicy *)_from;
    DDS::SchedulingClassQosPolicy *to = (DDS::SchedulingClassQosPolicy *)_to;
    *to = (DDS::SchedulingClassQosPolicy)from->kind;
}

void
__DDS_SchedulingPriorityQosPolicy__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_SchedulingPriorityQosPolicy *from = (const struct _DDS_SchedulingPriorityQosPolicy *)_from;
    DDS::SchedulingPriorityQosPolicy *to = (DDS::SchedulingPriorityQosPolicy *)_to;
    *to = (DDS::SchedulingPriorityQosPolicy)from->kind;
}

void
__DDS_SchedulingQosPolicy__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_SchedulingQosPolicy *from = (const struct _DDS_SchedulingQosPolicy *)_from;
    DDS::SchedulingQosPolicy *to = (DDS::SchedulingQosPolicy *)_to;
    {
        DDS::SchedulingClassQosPolicy scheduling_class;
        __DDS_SchedulingClassQosPolicy__copyOut(&from->scheduling_class, &scheduling_class);
        to->scheduling_kind(scheduling_class);
    }
    {
        DDS::SchedulingPriorityQosPolicy scheduling_priority;
        __DDS_SchedulingPriorityQosPolicy__copyOut(&from->scheduling_priority_kind, &scheduling_priority);
        to->scheduling_priority_kind(scheduling_priority);
    }
    to->scheduling_priority((int32_t) from->scheduling_priority);
}

void
__DDS_DomainParticipantQos__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_DomainParticipantQos *from = (const struct _DDS_DomainParticipantQos *)_from;
    dds::domain::qos::DomainParticipantQos *to = (dds::domain::qos::DomainParticipantQos *)_to;
    {
        dds::core::policy::UserData user_data;
        __DDS_UserDataQosPolicy__copyOut(&from->user_data, &user_data);
        to->policy(user_data);
    }
    {
        dds::core::policy::EntityFactory entity_factory;
        __DDS_EntityFactoryQosPolicy__copyOut(&from->entity_factory, &entity_factory);
        to->policy(entity_factory);
    }
    {
        org::opensplice::core::policy::WatchdogScheduling watchdog_scheduling;
        __DDS_SchedulingQosPolicy__copyOut(&from->watchdog_scheduling, &watchdog_scheduling);
        to->policy(watchdog_scheduling);
    }
    {
        org::opensplice::core::policy::ListenerScheduling listener_scheduling;
        __DDS_SchedulingQosPolicy__copyOut(&from->listener_scheduling, &listener_scheduling);
        to->policy(listener_scheduling);
    }
}

void
__DDS_TopicQos__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_TopicQos *from = (const struct _DDS_TopicQos *)_from;
    dds::topic::qos::TopicQos *to = (dds::topic::qos::TopicQos *)_to;
    {
        dds::core::policy::TopicData topic_data;
        __DDS_TopicDataQosPolicy__copyOut(&from->topic_data, &topic_data);
        to->policy(topic_data);
    }
    {
        dds::core::policy::Durability durability;
        __DDS_DurabilityQosPolicy__copyOut(&from->durability, &durability);
        to->policy(durability);
    }
    {
        dds::core::policy::DurabilityService durability_service;
        __DDS_DurabilityServiceQosPolicy__copyOut(&from->durability_service, &durability_service);
        to->policy(durability_service);
    }
    {
        dds::core::policy::Deadline deadline;
        __DDS_DeadlineQosPolicy__copyOut(&from->deadline, &deadline);
        to->policy(deadline);
    }
    {
        dds::core::policy::LatencyBudget latency_budget;
        __DDS_LatencyBudgetQosPolicy__copyOut(&from->latency_budget, &latency_budget);
        to->policy(latency_budget);
    }
    {
        dds::core::policy::Liveliness liveliness;
        __DDS_LivelinessQosPolicy__copyOut(&from->liveliness, &liveliness);
        to->policy(liveliness);
    }
    {
        dds::core::policy::Reliability reliability;
        __DDS_ReliabilityQosPolicy__copyOut(&from->reliability, &reliability);
        to->policy(reliability);
    }
    {
        dds::core::policy::DestinationOrder destination_order;
        __DDS_DestinationOrderQosPolicy__copyOut(&from->destination_order, &destination_order);
        to->policy(destination_order);
    }
    {
        dds::core::policy::History history;
        __DDS_HistoryQosPolicy__copyOut(&from->history, &history);
        to->policy(history);
    }
    {
        dds::core::policy::ResourceLimits resource_limits;
        __DDS_ResourceLimitsQosPolicy__copyOut(&from->resource_limits, &resource_limits);
        to->policy(resource_limits);
    }
    {
        dds::core::policy::TransportPriority transport_priority;
        __DDS_TransportPriorityQosPolicy__copyOut(&from->transport_priority, &transport_priority);
        to->policy(transport_priority);
    }
    {
        dds::core::policy::Lifespan lifespan;
        __DDS_LifespanQosPolicy__copyOut(&from->lifespan, &lifespan);
        to->policy(lifespan);
    }
    {
        dds::core::policy::Ownership ownership;
        __DDS_OwnershipQosPolicy__copyOut(&from->ownership, &ownership);
        to->policy(ownership);
    }
}

void
__DDS_DataWriterQos__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_DataWriterQos *from = (const struct _DDS_DataWriterQos *)_from;
    dds::pub::qos::DataWriterQos *to = (dds::pub::qos::DataWriterQos *)_to;
    {
        dds::core::policy::Durability durability;
        __DDS_DurabilityQosPolicy__copyOut(&from->durability, &durability);
        to->policy(durability);
    }
    {
        dds::core::policy::Deadline deadline;
        __DDS_DeadlineQosPolicy__copyOut(&from->deadline, &deadline);
        to->policy(deadline);
    }
    {
        dds::core::policy::LatencyBudget latency_budget;
        __DDS_LatencyBudgetQosPolicy__copyOut(&from->latency_budget, &latency_budget);
        to->policy(latency_budget);
    }
    {
        dds::core::policy::Liveliness liveliness;
        __DDS_LivelinessQosPolicy__copyOut(&from->liveliness, &liveliness);
        to->policy(liveliness);
    }
    {
        dds::core::policy::Reliability reliability;
        __DDS_ReliabilityQosPolicy__copyOut(&from->reliability, &reliability);
        to->policy(reliability);
    }
    {
        dds::core::policy::DestinationOrder destination_order;
        __DDS_DestinationOrderQosPolicy__copyOut(&from->destination_order, &destination_order);
        to->policy(destination_order);
    }
    {
        dds::core::policy::History history;
        __DDS_HistoryQosPolicy__copyOut(&from->history, &history);
        to->policy(history);
    }
    {
        dds::core::policy::ResourceLimits resource_limits;
        __DDS_ResourceLimitsQosPolicy__copyOut(&from->resource_limits, &resource_limits);
        to->policy(resource_limits);
    }
    {
        dds::core::policy::TransportPriority transport_priority;
        __DDS_TransportPriorityQosPolicy__copyOut(&from->transport_priority, &transport_priority);
        to->policy(transport_priority);
    }
    {
        dds::core::policy::Lifespan lifespan;
        __DDS_LifespanQosPolicy__copyOut(&from->lifespan, &lifespan);
        to->policy(lifespan);
    }
    {
        dds::core::policy::UserData user_data;
        __DDS_UserDataQosPolicy__copyOut(&from->user_data, &user_data);
        to->policy(user_data);
    }
    {
        dds::core::policy::Ownership ownership;
        __DDS_OwnershipQosPolicy__copyOut(&from->ownership, &ownership);
        to->policy(ownership);
    }
    {
        dds::core::policy::OwnershipStrength ownership_strength;
        __DDS_OwnershipStrengthQosPolicy__copyOut(&from->ownership_strength, &ownership_strength);
        to->policy(ownership_strength);
    }
    {
        dds::core::policy::WriterDataLifecycle writer_data_lifecycle;
        __DDS_WriterDataLifecycleQosPolicy__copyOut(&from->writer_data_lifecycle, &writer_data_lifecycle);
        to->policy(writer_data_lifecycle);
    }
}

void
__DDS_PublisherQos__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_PublisherQos *from = (const struct _DDS_PublisherQos *)_from;
    dds::pub::qos::PublisherQos *to = (dds::pub::qos::PublisherQos *)_to;
    {
        dds::core::policy::Presentation presentation;
        __DDS_PresentationQosPolicy__copyOut(&from->presentation, &presentation);
        to->policy(presentation);
    }
    {
        dds::core::policy::Partition partition;
        __DDS_PartitionQosPolicy__copyOut(&from->partition, &partition);
        to->policy(partition);
    }
    {
        dds::core::policy::GroupData group_data;
        __DDS_GroupDataQosPolicy__copyOut(&from->group_data, &group_data);
        to->policy(group_data);
    }
    {
        dds::core::policy::EntityFactory entity_factory;
        __DDS_EntityFactoryQosPolicy__copyOut(&from->entity_factory, &entity_factory);
        to->policy(entity_factory);
    }
}

void
__DDS_DataReaderQos__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_DataReaderQos *from = (const struct _DDS_DataReaderQos *)_from;
    dds::sub::qos::DataReaderQos *to = (dds::sub::qos::DataReaderQos *)_to;
    {
        dds::core::policy::Durability durability;
        __DDS_DurabilityQosPolicy__copyOut(&from->durability, &durability);
        to->policy(durability);
    }
    {
        dds::core::policy::Deadline deadline;
        __DDS_DeadlineQosPolicy__copyOut(&from->deadline, &deadline);
        to->policy(deadline);
    }
    {
        dds::core::policy::LatencyBudget latency_budget;
        __DDS_LatencyBudgetQosPolicy__copyOut(&from->latency_budget, &latency_budget);
        to->policy(latency_budget);
    }
    {
        dds::core::policy::Liveliness liveliness;
        __DDS_LivelinessQosPolicy__copyOut(&from->liveliness, &liveliness);
        to->policy(liveliness);
    }
    {
        dds::core::policy::Reliability reliability;
        __DDS_ReliabilityQosPolicy__copyOut(&from->reliability, &reliability);
        to->policy(reliability);
    }
    {
        dds::core::policy::DestinationOrder destination_order;
        __DDS_DestinationOrderQosPolicy__copyOut(&from->destination_order, &destination_order);
        to->policy(destination_order);
    }
    {
        dds::core::policy::History history;
        __DDS_HistoryQosPolicy__copyOut(&from->history, &history);
        to->policy(history);
    }
    {
        dds::core::policy::ResourceLimits resource_limits;
        __DDS_ResourceLimitsQosPolicy__copyOut(&from->resource_limits, &resource_limits);
        to->policy(resource_limits);
    }
    {
        dds::core::policy::UserData user_data;
        __DDS_UserDataQosPolicy__copyOut(&from->user_data, &user_data);
        to->policy(user_data);
    }
    {
        dds::core::policy::Ownership ownership;
        __DDS_OwnershipQosPolicy__copyOut(&from->ownership, &ownership);
        to->policy(ownership);
    }
    {
        dds::core::policy::TimeBasedFilter time_based_filter;
        __DDS_TimeBasedFilterQosPolicy__copyOut(&from->time_based_filter, &time_based_filter);
        to->policy(time_based_filter);
    }
    {
        dds::core::policy::ReaderDataLifecycle reader_data_lifecycle;
        __DDS_ReaderDataLifecycleQosPolicy__copyOut(&from->reader_data_lifecycle, &reader_data_lifecycle);
        to->policy(reader_data_lifecycle);
    }
    {
        org::opensplice::core::policy::SubscriptionKey subscription_keys;
        __DDS_SubscriptionKeyQosPolicy__copyOut(&from->subscription_keys, &subscription_keys);
        to->policy(subscription_keys);
    }
    {
        org::opensplice::core::policy::ReaderLifespan reader_lifespan;
        __DDS_ReaderLifespanQosPolicy__copyOut(&from->reader_lifespan, &reader_lifespan);
        to->policy(reader_lifespan);
    }
    {
        org::opensplice::core::policy::Share share;
        __DDS_ShareQosPolicy__copyOut(&from->share, &share);
        to->policy(share);
    }
}

void
__DDS_SubscriberQos__copyOut(
    const void *_from,
    void *_to)
{
    const struct _DDS_SubscriberQos *from = (const struct _DDS_SubscriberQos *)_from;
    dds::sub::qos::SubscriberQos *to = (dds::sub::qos::SubscriberQos *)_to;
    {
        dds::core::policy::Presentation presentation;
        __DDS_PresentationQosPolicy__copyOut(&from->presentation, &presentation);
        to->policy(presentation);
    }
    {
        dds::core::policy::Partition partition;
        __DDS_PartitionQosPolicy__copyOut(&from->partition, &partition);
        to->policy(partition);
    }
    {
        dds::core::policy::GroupData group_data;
        __DDS_GroupDataQosPolicy__copyOut(&from->group_data, &group_data);
        to->policy(group_data);
    }
    {
        dds::core::policy::EntityFactory entity_factory;
        __DDS_EntityFactoryQosPolicy__copyOut(&from->entity_factory, &entity_factory);
        to->policy(entity_factory);
    }
    {
        org::opensplice::core::policy::Share share;
        __DDS_ShareQosPolicy__copyOut(&from->share, &share);
        to->policy(share);
    }
}

