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

#include <org/opensplice/topic/qos/QosConverter.hpp>
#include <org/opensplice/core/policy/PolicyConverter.hpp>

using namespace org::opensplice::core::policy;

dds::topic::qos::TopicQos
org::opensplice::topic::qos::convertQos(const DDS::TopicQos& from)
{
    dds::topic::qos::TopicQos to;
    to = to << convertPolicy(from.durability) << convertPolicy(from.durability_service)
         << convertPolicy(from.deadline) << convertPolicy(from.latency_budget)
         << convertPolicy(from.liveliness) << convertPolicy(from.reliability)
         << convertPolicy(from.destination_order) << convertPolicy(from.history)
         << convertPolicy(from.resource_limits) << convertPolicy(from.transport_priority)
         << convertPolicy(from.lifespan) << convertPolicy(from.ownership);
    return to;
}

DDS::TopicQos
org::opensplice::topic::qos::convertQos(const dds::topic::qos::TopicQos& from)
{
    DDS::TopicQos to;

    DDS::DurabilityQosPolicy durability = convertPolicy(from.policy<dds::core::policy::Durability>());
    to.durability.kind = durability.kind;

    DDS::DurabilityServiceQosPolicy durability_service = convertPolicy(from.policy<dds::core::policy::DurabilityService>());
    to.durability_service.service_cleanup_delay = durability_service.service_cleanup_delay;
    to.durability_service.history_kind = durability_service.history_kind;
    to.durability_service.history_depth = durability_service.history_depth;
    to.durability_service.max_samples = durability_service.max_samples;
    to.durability_service.max_instances = durability_service.max_instances;
    to.durability_service.max_samples_per_instance = durability_service.max_samples_per_instance;

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

    DDS::OwnershipQosPolicy ownership = convertPolicy(from.policy<dds::core::policy::Ownership>());
    to.ownership.kind = ownership.kind;

    return to;
}
