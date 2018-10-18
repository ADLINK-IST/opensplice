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

typedef dds::core::policy::TShare<org::opensplice::core::policy::Share>
Share;

typedef dds::core::policy::TProductData<org::opensplice::core::policy::ProductData>
ProductData;

typedef dds::core::policy::TSubscriptionKey<org::opensplice::core::policy::SubscriptionKey>
SubscriptionKey;
}
}
}
} // namespace dds::core::policy::detail


// End of implementation

#endif /* OSPL_DDS_CORE_POLICY_DETAIL_COREPOLICY_HPP_ */
