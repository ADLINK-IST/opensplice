/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2012 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
*
*/


/**
 * @file
 */

#include <org/opensplice/sub/qos/QosConverter.hpp>
#include <org/opensplice/core/policy/PolicyConverter.hpp>

using namespace org::opensplice::core::policy;

dds::sub::qos::DataReaderQos
org::opensplice::sub::qos::convertQos(const DDS::DataReaderQos& from)
{
    dds::sub::qos::DataReaderQos to;
    to = to << convertPolicy(from.durability) << convertPolicy(from.deadline)
         << convertPolicy(from.latency_budget) << convertPolicy(from.liveliness)
         << convertPolicy(from.reliability) << convertPolicy(from.destination_order)
         << convertPolicy(from.history) << convertPolicy(from.resource_limits)
         << convertPolicy(from.user_data) << convertPolicy(from.ownership)
         << convertPolicy(from.time_based_filter) << convertPolicy(from.reader_data_lifecycle);
    return to;
}

DDS::DataReaderQos
org::opensplice::sub::qos::convertQos(const dds::sub::qos::DataReaderQos& from)
{
    DDS::DataReaderQos to;

    DDS::DurabilityQosPolicy durability = convertPolicy(from.policy<dds::core::policy::Durability>());
    to.durability.kind = durability.kind;

    DDS::DeadlineQosPolicy deadline = convertPolicy(from.policy<dds::core::policy::Deadline>());
    to.deadline.period = deadline.period;

    DDS::LatencyBudgetQosPolicy latency_budget = convertPolicy(from.policy<dds::core::policy::LatencyBudget>());
    to.latency_budget.duration = latency_budget.duration;

    DDS::LivelinessQosPolicy liveliness = convertPolicy(from.policy<dds::core::policy::Liveliness>());
    to.liveliness.kind = liveliness.kind;
    to.liveliness.lease_duration = liveliness.lease_duration;

    DDS::ReliabilityQosPolicy reliability = convertPolicy(from.policy<dds::core::policy::Reliability>());
    to.reliability.kind = reliability.kind;
    to.reliability.max_blocking_time = reliability.max_blocking_time;
    to.reliability.synchronous = reliability.synchronous;

    DDS::DestinationOrderQosPolicy destination_order = convertPolicy(from.policy<dds::core::policy::DestinationOrder>());
    to.destination_order.kind = destination_order.kind;

    DDS::HistoryQosPolicy history = convertPolicy(from.policy<dds::core::policy::History>());
    to.history.kind = history.kind;
    to.history.depth = history.depth;

    DDS::ResourceLimitsQosPolicy resource_limits = convertPolicy(from.policy<dds::core::policy::ResourceLimits>());
    to.resource_limits.max_samples = resource_limits.max_samples;
    to.resource_limits.max_instances = resource_limits.max_instances;
    to.resource_limits.max_samples_per_instance = resource_limits.max_samples_per_instance;

    DDS::UserDataQosPolicy user_data = convertPolicy(from.policy<dds::core::policy::UserData>());
    to.user_data.value = user_data.value;

    DDS::OwnershipQosPolicy ownership = convertPolicy(from.policy<dds::core::policy::Ownership>());
    to.ownership.kind = ownership.kind;

    DDS::TimeBasedFilterQosPolicy time_based_filter = convertPolicy(from.policy<dds::core::policy::TimeBasedFilter>());
    to.time_based_filter.minimum_separation = time_based_filter.minimum_separation;

    DDS::ReaderDataLifecycleQosPolicy reader_data_lifecycle = convertPolicy(from.policy<dds::core::policy::ReaderDataLifecycle>());
    to.reader_data_lifecycle.autopurge_nowriter_samples_delay = reader_data_lifecycle.autopurge_nowriter_samples_delay;
    to.reader_data_lifecycle.autopurge_disposed_samples_delay = reader_data_lifecycle.autopurge_disposed_samples_delay;
    to.reader_data_lifecycle.enable_invalid_samples = reader_data_lifecycle.enable_invalid_samples;
    to.reader_data_lifecycle.invalid_sample_visibility = reader_data_lifecycle.invalid_sample_visibility;

    to.subscription_keys.use_key_list = false;

    to.reader_lifespan.use_lifespan = false;
    to.reader_lifespan.duration = DDS::DURATION_INFINITE;

    to.share.enable = false;
    to.share.name = (const char*)0;

    return to;
}

dds::sub::qos::SubscriberQos
org::opensplice::sub::qos::convertQos(const DDS::SubscriberQos& from)
{
    dds::sub::qos::SubscriberQos to;
    to = to << convertPolicy(from.presentation) << convertPolicy(from.partition)
         << convertPolicy(from.group_data) << convertPolicy(from.entity_factory);
    return to;
}

DDS::SubscriberQos
org::opensplice::sub::qos::convertQos(const dds::sub::qos::SubscriberQos& from)
{
    DDS::SubscriberQos to;

    DDS::PresentationQosPolicy presentation = convertPolicy(from.policy<dds::core::policy::Presentation>());
    to.presentation.access_scope = presentation.access_scope;
    to.presentation.coherent_access = presentation.coherent_access;
    to.presentation.ordered_access = presentation.ordered_access;

    DDS::PartitionQosPolicy partition = convertPolicy(from.policy<dds::core::policy::Partition>());
    to.partition.name = partition.name;

    DDS::GroupDataQosPolicy group_data = convertPolicy(from.policy<dds::core::policy::GroupData>());
    to.group_data.value = group_data.value;

    DDS::EntityFactoryQosPolicy entity_factory = convertPolicy(from.policy<dds::core::policy::EntityFactory>());
    to.entity_factory.autoenable_created_entities = entity_factory.autoenable_created_entities;

    to.share.enable = false;
    to.share.name = (const char*)0;

    return to;
}
