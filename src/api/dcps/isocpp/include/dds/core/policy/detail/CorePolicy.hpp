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
#ifndef OSPL_DDS_CORE_POLICY_DETAIL_COREPOLICY_HPP_
#define OSPL_DDS_CORE_POLICY_DETAIL_COREPOLICY_HPP_

/**
 * @file
 */

// Implementation

#include <org/opensplice/core/policy/CorePolicy.hpp>
#include <dds/core/policy/TCorePolicy.hpp>


namespace dds
{
namespace core
{
namespace policy
{
namespace detail
{
#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
typedef dds::core::policy::TDataRepresentation<org::opensplice::core::policy::DataRepresentation>
DataRepresentation;
#endif // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

typedef dds::core::policy::TDeadline<org::opensplice::core::policy::Deadline>
Deadline;

typedef dds::core::policy::TDestinationOrder<org::opensplice::core::policy::DestinationOrder>
DestinationOrder;

typedef dds::core::policy::TDurability<org::opensplice::core::policy::Durability>
Durability;

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
typedef dds::core::policy::TDurabilityService<org::opensplice::core::policy::DurabilityService>
DurabilityService;
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

typedef dds::core::policy::TEntityFactory<org::opensplice::core::policy::EntityFactory>
EntityFactory;

typedef dds::core::policy::TGroupData<org::opensplice::core::policy::GroupData>
GroupData;

typedef dds::core::policy::THistory<org::opensplice::core::policy::History>
History;

typedef dds::core::policy::TLatencyBudget<org::opensplice::core::policy::LatencyBudget>
LatencyBudget;

typedef dds::core::policy::TLifespan<org::opensplice::core::policy::Lifespan>
Lifespan;

typedef dds::core::policy::TLiveliness<org::opensplice::core::policy::Liveliness>
Liveliness;

typedef dds::core::policy::TOwnership<org::opensplice::core::policy::Ownership>
Ownership;

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
typedef dds::core::policy::TOwnershipStrength<org::opensplice::core::policy::OwnershipStrength>
OwnershipStrength;
#endif  // OMG_DDS_OWNERSHIP_SUPPORT

typedef dds::core::policy::TPartition<org::opensplice::core::policy::Partition>
Partition;

typedef dds::core::policy::TPresentation<org::opensplice::core::policy::Presentation>
Presentation;

typedef dds::core::policy::TReaderDataLifecycle<org::opensplice::core::policy::ReaderDataLifecycle>
ReaderDataLifecycle;

typedef dds::core::policy::TReliability<org::opensplice::core::policy::Reliability>
Reliability;

typedef dds::core::policy::TResourceLimits<org::opensplice::core::policy::ResourceLimits>
ResourceLimits;

typedef dds::core::policy::TTimeBasedFilter<org::opensplice::core::policy::TimeBasedFilter>
TimeBasedFilter;

typedef dds::core::policy::TTopicData<org::opensplice::core::policy::TopicData>
TopicData;

typedef dds::core::policy::TTransportPriority<org::opensplice::core::policy::TransportPriority>
TransportPriority;

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
typedef dds::core::policy::TTypeConsistencyEnforcement<org::opensplice::core::policy::TypeConsistencyEnforcement>
TypeConsistencyEnforcement;
#endif // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

typedef dds::core::policy::TUserData<org::opensplice::core::policy::UserData>
UserData;

typedef dds::core::policy::TWriterDataLifecycle<org::opensplice::core::policy::WriterDataLifecycle>
WriterDataLifecycle;
}
}
}
} // namespace dds::core::policy::detail


// End of implementation

#endif /* OSPL_DDS_CORE_POLICY_DETAIL_COREPOLICY_HPP_ */
