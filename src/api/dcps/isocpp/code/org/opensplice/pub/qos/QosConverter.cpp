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

#include <org/opensplice/pub/qos/QosConverter.hpp>
#include <org/opensplice/core/policy/PolicyConverter.hpp>

using namespace org::opensplice::core::policy;

dds::pub::qos::DataWriterQos
org::opensplice::pub::qos::convertQos(const DDS::DataWriterQos& from)
{
    dds::pub::qos::DataWriterQos to;
    to = to << convertPolicy(from.durability) << convertPolicy(from.deadline)
         << convertPolicy(from.latency_budget) << convertPolicy(from.liveliness)
         << convertPolicy(from.reliability) << convertPolicy(from.destination_order)
         << convertPolicy(from.history) << convertPolicy(from.resource_limits)
         << convertPolicy(from.transport_priority) << convertPolicy(from.lifespan)
         << convertPolicy(from.user_data) << convertPolicy(from.ownership)
         << convertPolicy(from.ownership_strength) << convertPolicy(from.writer_data_lifecycle);
    return to;
}

DDS::DataWriterQos
org::opensplice::pub::qos::convertQos(const dds::pub::qos::DataWriterQos& from)
{
    DDS::DataWriterQos to;

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

    DDS::TransportPriorityQosPolicy transport_priority = convertPolicy(from.policy<dds::core::policy::TransportPriority>());
    to.transport_priority.value = transport_priority.value;

    DDS::LifespanQosPolicy lifespan = convertPolicy(from.policy<dds::core::policy::Lifespan>());
    to.lifespan.duration = lifespan.duration;

    DDS::UserDataQosPolicy user_data = convertPolicy(from.policy<dds::core::policy::UserData>());
    to.user_data.value = user_data.value;

    DDS::OwnershipQosPolicy ownership = convertPolicy(from.policy<dds::core::policy::Ownership>());
    to.ownership.kind = ownership.kind;

    DDS::OwnershipStrengthQosPolicy ownership_strength = convertPolicy(from.policy<dds::core::policy::OwnershipStrength>());
    to.ownership_strength.value = ownership_strength.value;

    DDS::WriterDataLifecycleQosPolicy writer_data_lifecycle = convertPolicy(from.policy<dds::core::policy::WriterDataLifecycle>());
    to.writer_data_lifecycle.autodispose_unregistered_instances = writer_data_lifecycle.autodispose_unregistered_instances;
    to.writer_data_lifecycle.autopurge_suspended_samples_delay = writer_data_lifecycle.autopurge_suspended_samples_delay;
    to.writer_data_lifecycle.autounregister_instance_delay = writer_data_lifecycle.autounregister_instance_delay;

    return to;
}

dds::pub::qos::PublisherQos
org::opensplice::pub::qos::convertQos(const DDS::PublisherQos& from)
{
    dds::pub::qos::PublisherQos to;
    to = to << convertPolicy(from.presentation) << convertPolicy(from.partition)
         << convertPolicy(from.group_data) << convertPolicy(from.entity_factory);
    return to;
}

DDS::PublisherQos
org::opensplice::pub::qos::convertQos(const dds::pub::qos::PublisherQos& from)
{
    DDS::PublisherQos to;

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

    return to;
}
