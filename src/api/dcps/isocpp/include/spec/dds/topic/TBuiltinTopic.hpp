#ifndef OMG_TDDS_TOPIC_BUILT_IN_TOPIC_HPP_
#define OMG_TDDS_TOPIC_BUILT_IN_TOPIC_HPP_

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

#include <dds/core/detail/conformance.hpp>
#include <dds/core/Value.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include <dds/topic/BuiltinTopicKey.hpp>

namespace dds { namespace topic {
  template <typename D>
  class TParticipantBuiltinTopicData;

  template <typename D>
  class TTopicBuiltinTopicData;

  template <typename D>
  class TPublicationBuiltinTopicData;

  template <typename D>
  class TSubscriptionBuiltinTopicData;
} }

template <typename D>
class dds::topic::TParticipantBuiltinTopicData : public ::dds::core::Value<D> {
public:
  const dds::topic::BuiltinTopicKey& key() const;

  const ::dds::core::policy::UserData& user_data() const;
};

template <typename D>
class dds::topic::TTopicBuiltinTopicData : public ::dds::core::Value<D>  {
public:
  const dds::topic::BuiltinTopicKey& key();

  const std::string&                  name() const;
  const std::string&                  type_name() const;

  const ::dds::core::policy::Durability&         durability() const;


#ifdef OMG_DDS_PERSISTENCE_SUPPORT

  const ::dds::core::policy::DurabilityService&  durability_service() const;

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


  const ::dds::core::policy::Deadline&           deadline() const;
  const ::dds::core::policy::LatencyBudget&      latency_budget() const;
  const ::dds::core::policy::Liveliness&         liveliness() const;
  const ::dds::core::policy::Reliability&        reliability() const;
  const ::dds::core::policy::TransportPriority&  transport_priority() const;
  const ::dds::core::policy::Lifespan&           lifespan() const;
  const ::dds::core::policy::DestinationOrder&   destination_order() const;
  const ::dds::core::policy::History&            history() const;
  const ::dds::core::policy::ResourceLimits&     resource_limits() const;
  const ::dds::core::policy::Ownership&          ownership() const;
  const ::dds::core::policy::TopicData&          topic_data() const;
};

template <typename D>
class dds::topic::TPublicationBuiltinTopicData  : public ::dds::core::Value<D> {
public:
  const dds::topic::BuiltinTopicKey& key() const;
  const dds::topic::BuiltinTopicKey& participant_key() const;
  const std::string&                  topic_name() const;
  const std::string&                  type_name() const;
  const ::dds::core::policy::Durability&         durability() const;

#ifdef OMG_DDS_PERSISTENCE_SUPPORT

  const ::dds::core::policy::DurabilityService&  durability_service() const;

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


  const ::dds::core::policy::Deadline&           deadline() const;
  const ::dds::core::policy::LatencyBudget&      latency_budget() const;
  const ::dds::core::policy::Liveliness&         liveliness() const;
  const ::dds::core::policy::Reliability&        reliability() const;
  const ::dds::core::policy::Lifespan&           lifespan() const;
  const ::dds::core::policy::UserData&           user_data() const;
  const ::dds::core::policy::Ownership&          ownership() const;


#ifdef OMG_DDS_OWNERSHIP_SUPPORT

const ::dds::core::policy::OwnershipStrength&  ownership_strength() const;

#endif  // OMG_DDS_OWNERSHIP_SUPPORT


const ::dds::core::policy::DestinationOrder&   destination_order() const;

const ::dds::core::policy::Presentation&       presentation() const;

const ::dds::core::policy::Partition&          partition() const;

const ::dds::core::policy::TopicData&          topic_data() const;

const ::dds::core::policy::GroupData&          group_data() const;

};

template <typename D>
class dds::topic::TSubscriptionBuiltinTopicData  : public ::dds::core::Value<D> {
public:
  const dds::topic::BuiltinTopicKey& key() const;
  const dds::topic::BuiltinTopicKey& participant_key() const;
  const std::string&                  topic_name() const;
  const std::string&                  type_name() const;
  const ::dds::core::policy::Durability&         durability() const;
  const ::dds::core::policy::Deadline&           deadline() const;
  const ::dds::core::policy::LatencyBudget&      latency_budget() const;
  const ::dds::core::policy::Liveliness&         liveliness() const;
  const ::dds::core::policy::Reliability&        reliability() const;
  const ::dds::core::policy::Ownership&          ownership() const;
  const ::dds::core::policy::DestinationOrder&   destination_order() const;
  const ::dds::core::policy::UserData&           user_data() const;
  const ::dds::core::policy::TimeBasedFilter&    time_based_filter() const;
  const ::dds::core::policy::Presentation&       presentation() const;
  const ::dds::core::policy::Partition&          partition() const;
  const ::dds::core::policy::TopicData&          topic_data() const;
  const ::dds::core::policy::GroupData&          group_data() const;

};

#endif /* OMG_TDDS_TOPIC_BUILT_IN_TOPIC_HPP_ */
