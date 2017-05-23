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

namespace dds
{
namespace topic
{
template <typename D>
class TParticipantBuiltinTopicData;

template <typename D>
class TTopicBuiltinTopicData;

template <typename D>
class TPublicationBuiltinTopicData;

template <typename D>
class TSubscriptionBuiltinTopicData;

template <typename D>
class TCMParticipantBuiltinTopicData;

template <typename D>
class TCMPublisherBuiltinTopicData;

template <typename D>
class TCMSubscriberBuiltinTopicData;

template <typename D>
class TCMDataWriterBuiltinTopicData;

template <typename D>
class TCMDataReaderBuiltinTopicData;
}
}

/**
 * The DCPSParticipant topic communicates the existence of DomainParticipants
 * by means of the ParticipantBuiltinTopicData datatype. Each
 * ParticipantBuiltinTopicData sample in a Domain represents a DomainParticipant
 * that participates in that Domain: a new ParticipantBuiltinTopicData instance
 * is created when a newly-added DomainParticipant is enabled, and it is disposed
 * when that DomainParticipant is deleted. An updated ParticipantBuiltinTopicData
 * sample is written each time the DomainParticipant modifies its UserDataQosPolicy.
 */
template <typename D>
class dds::topic::TParticipantBuiltinTopicData : public ::dds::core::Value<D>
{
public:
    const dds::topic::BuiltinTopicKey& key() const;

    const ::dds::core::policy::UserData& user_data() const;
};

/**
 * The DCPSTopic topic communicates the existence of topics by means of the
 * TopicBuiltinTopicData datatype. Each TopicBuiltinTopicData sample in
 * a Domain represents a Topic in that Domain: a new TopicBuiltinTopicData
 * instance is created when a newly-added Topic is enabled. However, the instance is
 * not disposed when a Topic is deleted by its participant because a topic lifecycle
 * is tied to the lifecycle of a Domain, not to the lifecycle of an individual
 * participant. An updated TopicBuiltinTopicData sample is written each time a
 * Topic modifies one or more of its QosPolicy values.
 */
template <typename D>
class dds::topic::TTopicBuiltinTopicData : public ::dds::core::Value<D>
{
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

/**
 * The DCPSPublication topic communicates the existence of datawriters by means
 * of the PublicationBuiltinTopicData datatype. Each PublicationBuiltinTopicData
 * sample in a Domain represents a datawriter in that Domain: a new
 * PublicationBuiltinTopicData instance is created when a newly-added DataWriter
 * is enabled, and it is disposed when that DataWriter is deleted. An updated
 * PublicationBuiltinTopicData sample is written each time the DataWriter (or
 * the Publisher to which it belongs) modifies a QosPolicy that applies to the
 * entities connected to it. Also will it be updated when the writer looses or
 * regains its liveliness.
 */
template <typename D>
class dds::topic::TPublicationBuiltinTopicData  : public ::dds::core::Value<D>
{
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

/**
 * The DCPSSubscription topic communicates the existence of datareaders by
 * means of the SubscriptionBuiltinTopicData datatype. Each
 * SubscriptionBuiltinTopicData sample in a Domain represents a datareader
 * in that Domain: a new SubscriptionBuiltinTopicData instance is created
 * when a newly-added DataReader is enabled, and it is disposed when that
 * DataReader is deleted. An updated SubscriptionBuiltinTopicData sample is
 * written each time the DataReader (or the Subscriber to which it belongs)
 * modifies a QosPolicy that applies to the entities connected to it.
 */
template <typename D>
class dds::topic::TSubscriptionBuiltinTopicData  : public ::dds::core::Value<D>
{
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

/**
 * The CMParticipant topic...
 */
template <typename D>
class dds::topic::TCMParticipantBuiltinTopicData  : public ::dds::core::Value<D>
{
public:
    const dds::topic::BuiltinTopicKey&        key() const;
    const ::dds::core::policy::ProductData&   product() const;
};

/**
 * The CMPublisher topic...
 */
template <typename D>
class dds::topic::TCMPublisherBuiltinTopicData  : public ::dds::core::Value<D>
{
public:
    const dds::topic::BuiltinTopicKey&        key() const;
    const ::dds::core::policy::ProductData&   product() const;
    const dds::topic::BuiltinTopicKey&        participant_key() const;
    const std::string&                        name() const;
    const ::dds::core::policy::EntityFactory& entity_factory() const;
    const ::dds::core::policy::Partition&     partition() const;
};

/**
 * The CMSubscriber topic...
 */
template <typename D>
class dds::topic::TCMSubscriberBuiltinTopicData  : public ::dds::core::Value<D>
{
public:
    const dds::topic::BuiltinTopicKey&        key() const;
    const ::dds::core::policy::ProductData&   product() const;
    const dds::topic::BuiltinTopicKey&        participant_key() const;
    const std::string&                        name() const;
    const ::dds::core::policy::EntityFactory& entity_factory() const;
    const ::dds::core::policy::Partition&     partition() const;
    const ::dds::core::policy::Share&         share() const;
};

/**
 * The CMDataWriter topic...
 */
template <typename D>
class dds::topic::TCMDataWriterBuiltinTopicData  : public ::dds::core::Value<D>
{
public:
    const dds::topic::BuiltinTopicKey&              key() const;
    const ::dds::core::policy::ProductData&         product() const;
    const dds::topic::BuiltinTopicKey&              publisher_key() const;
    const std::string&                              name() const;
    const ::dds::core::policy::History&             history() const;
    const ::dds::core::policy::ResourceLimits&      resource_limits() const;
    const ::dds::core::policy::WriterDataLifecycle& writer_data_lifecycle() const;
};

/**
 * The CMDataReader topic...
 */
template <typename D>
class dds::topic::TCMDataReaderBuiltinTopicData  : public ::dds::core::Value<D>
{
public:
    const dds::topic::BuiltinTopicKey&              key() const;
    const ::dds::core::policy::ProductData&         product() const;
    const dds::topic::BuiltinTopicKey&              subscriber_key() const;
    const std::string&                              name() const;
    const ::dds::core::policy::History&             history() const;
    const ::dds::core::policy::ResourceLimits&      resource_limits() const;
    const ::dds::core::policy::ReaderDataLifecycle& reader_data_lifecycle() const;
    const ::dds::core::policy::SubscriptionKey&     subscription_keys() const;
    const ::dds::core::policy::Lifespan&            reader_lifespan() const;
    const ::dds::core::policy::Share&               share() const;
};

#endif /* OMG_TDDS_TOPIC_BUILT_IN_TOPIC_HPP_ */
