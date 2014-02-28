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

#ifndef ORG_OPENSPLICE_CORE_POLICY_POLICYCONVERTER_HPP_
#define ORG_OPENSPLICE_CORE_POLICY_POLICYCONVERTER_HPP_

#include <dds/core/types.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace core
{
namespace policy
{
/**
* @file
* @todo Have added imports to this file just for unit tests.
* Should we try and build dbts static ?
*/
// Destination Order Policy
dds::core::policy::DestinationOrder
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::DestinationOrderQosPolicy& from);

DDS::DestinationOrderQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::DestinationOrder& from);

// Deadline Policy
dds::core::policy::Deadline
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::DeadlineQosPolicy& from);

DDS::DeadlineQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::Deadline& from);

// Durability Policy
dds::core::policy::Durability
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::DurabilityQosPolicy& from);

DDS::DurabilityQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::Durability& from);

// Durability Service Policy
dds::core::policy::DurabilityService
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::DurabilityServiceQosPolicy& from);

DDS::DurabilityServiceQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::DurabilityService& from);

// Entity Factory Policy
dds::core::policy::EntityFactory
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::EntityFactoryQosPolicy& from);

DDS::EntityFactoryQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::EntityFactory& from);

//Group Data Policy
dds::core::policy::GroupData
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::GroupDataQosPolicy& from);

DDS::GroupDataQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::GroupData& from);

// History Policy
dds::core::policy::History
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::HistoryQosPolicy& h);

DDS::HistoryQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::History& h);

// Latency Budget Policy
dds::core::policy::LatencyBudget
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::LatencyBudgetQosPolicy& from);

DDS::LatencyBudgetQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::LatencyBudget& from);

// Lifespan Policy
dds::core::policy::Lifespan
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::LifespanQosPolicy& from);

DDS::LifespanQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::Lifespan& from);

// Liveliness Policy
dds::core::policy::Liveliness
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::LivelinessQosPolicy& from);

DDS::LivelinessQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::Liveliness& from);

#ifdef OMG_DDS_OWNERSHIP_SUPPORT
// Ownership Policy
dds::core::policy::Ownership
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::OwnershipQosPolicy& from);

DDS::OwnershipQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::Ownership& from);

// Ownership Strength
dds::core::policy::OwnershipStrength
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::OwnershipStrengthQosPolicy& from);

DDS::OwnershipStrengthQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::OwnershipStrength& from);
#endif  // OMG_DDS_OWNERSHIP_SUPPORT

// Partition Policy
dds::core::policy::Partition
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::PartitionQosPolicy& from);

DDS::PartitionQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::Partition& from);

// Presentation Policy
dds::core::policy::Presentation
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::PresentationQosPolicy& from);

DDS::PresentationQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::Presentation& from);

// Reader Data Lifecycle Policy
dds::core::policy::ReaderDataLifecycle
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::ReaderDataLifecycleQosPolicy& from);

DDS::ReaderDataLifecycleQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::ReaderDataLifecycle& from);

// Reliability Policy
dds::core::policy::Reliability
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::ReliabilityQosPolicy& from);

DDS::ReliabilityQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::Reliability& from);

// Resource Limits Policy
dds::core::policy::ResourceLimits
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::ResourceLimitsQosPolicy& from);

DDS::ResourceLimitsQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::ResourceLimits& from);

// Time Based Filter Policy
dds::core::policy::TimeBasedFilter
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::TimeBasedFilterQosPolicy& from);

DDS::TimeBasedFilterQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::TimeBasedFilter& from);

// Topic Data Policy
dds::core::policy::TopicData
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::TopicDataQosPolicy& from);

DDS::TopicDataQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::TopicData& from);

// Transport Priority Policy
dds::core::policy::TransportPriority
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::TransportPriorityQosPolicy& from);

DDS::TransportPriorityQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::TransportPriority& from);

// User Data Policy
dds::core::policy::UserData
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::UserDataQosPolicy& from);

DDS::UserDataQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::UserData& from);

// Writer Data Lifecycle Policy
dds::core::policy::WriterDataLifecycle
OSPL_ISOCPP_IMPL_API convertPolicy(const DDS::WriterDataLifecycleQosPolicy& from);

DDS::WriterDataLifecycleQosPolicy
OSPL_ISOCPP_IMPL_API convertPolicy(const dds::core::policy::WriterDataLifecycle& from);
}
}
}
}


#endif /* ORG_OPENSPLICE_CORE_POLICY_POLICYCONVERTER_HPP_ */
