#ifndef OMG_DDS_QOS_DETAIL_CORE_POLICY_HPP_
#define OMG_DDS_QOS_DETAIL_CORE_POLICY_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <foo/bar/core/policy/CorePolicy.hpp>
#include <dds/core/policy/TCorePolicy.hpp>


namespace dds { namespace core { namespace policy { namespace detail {
#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    typedef dds::core::policy::TDataRepresentation<foo::bar::core::policy::DataRepresentation>
    DataRepresentation;
#endif // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

    typedef dds::core::policy::TDeadline<foo::bar::core::policy::Deadline>
    Deadline;

    typedef dds::core::policy::TDestinationOrder<foo::bar::core::policy::DestinationOrder>
    DestinationOrder;

    typedef dds::core::policy::TDurability<foo::bar::core::policy::Durability>
    Durability;

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    typedef dds::core::policy::TDurabilityService<foo::bar::core::policy::DurabilityService>
    DurabilityService;
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

    typedef dds::core::policy::TEntityFactory<foo::bar::core::policy::EntityFactory>
    EntityFactory;

    typedef dds::core::policy::TGroupData<foo::bar::core::policy::GroupData>
    GroupData;

    typedef dds::core::policy::THistory<foo::bar::core::policy::History>
    History;

    typedef dds::core::policy::TLatencyBudget<foo::bar::core::policy::LatencyBudget>
    LatencyBudget;

    typedef dds::core::policy::TLifespan<foo::bar::core::policy::Lifespan>
    Lifespan;

    typedef dds::core::policy::TLiveliness<foo::bar::core::policy::Liveliness>
    Liveliness;

    typedef dds::core::policy::TOwnership<foo::bar::core::policy::Ownership>
    Ownership;

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
    typedef dds::core::policy::TOwnershipStrength<foo::bar::core::policy::OwnershipStrength>
    OwnershipStrength;
#endif  // OMG_DDS_OWNERSHIP_SUPPORT

    typedef dds::core::policy::TPartition<foo::bar::core::policy::Partition>
    Partition;

    typedef dds::core::policy::TPresentation<foo::bar::core::policy::Presentation>
    Presentation;

    typedef dds::core::policy::TReaderDataLifecycle<foo::bar::core::policy::ReaderDataLifecycle>
    ReaderDataLifecycle;

    typedef dds::core::policy::TReliability<foo::bar::core::policy::Reliability>
    Reliability;

    typedef dds::core::policy::TResourceLimits<foo::bar::core::policy::ResourceLimits>
    ResourceLimits;

    typedef dds::core::policy::TTimeBasedFilter<foo::bar::core::policy::TimeBasedFilter>
    TimeBasedFilter;

    typedef dds::core::policy::TTopicData<foo::bar::core::policy::TopicData>
    TopicData;

    typedef dds::core::policy::TTransportPriority<foo::bar::core::policy::TransportPriority>
    TransportPriority;

#ifdef OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT
    typedef dds::core::policy::TTypeConsistencyEnforcement<foo::bar::core::policy::TypeConsistencyEnforcement>
    TypeConsistencyEnforcement;
#endif // OMG_DDS_EXTENSIBLE_AND_DYNAMIC_TOPIC_TYPE_SUPPORT

    typedef dds::core::policy::TUserData<foo::bar::core::policy::UserData>
    UserData;

    typedef dds::core::policy::TWriterDataLifecycle<foo::bar::core::policy::WriterDataLifecycle>
    WriterDataLifecycle;
} } } } // namespace dds::core::policy::detail


#endif /* OMG_DDS_QOS_DETAIL_CORE_POLICY_HPP_ */
