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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_IMPL_HPP
#define ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_IMPL_HPP


#include <dds/core/detail/conformance.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include <dds/topic/BuiltinTopicKey.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{
class ParticipantBuiltinTopicDataImpl;
class TopicBuiltinTopicDataImpl;
class PublicationBuiltinTopicDataImpl;
class SubscriptionBuiltinTopicDataImpl;
class CMParticipantBuiltinTopicDataImpl;
class CMPublisherBuiltinTopicDataImpl;
class CMSubscriberBuiltinTopicDataImpl;
class CMDataWriterBuiltinTopicDataImpl;
class CMDataReaderBuiltinTopicDataImpl;
}
}
}

class org::opensplice::topic::ParticipantBuiltinTopicDataImpl
{
public:
    const dds::topic::BuiltinTopicKey& key()
    {
        return key_;
    }

    const ::dds::core::policy::UserData& user_data()
    {
        return user_data_;
    }

    bool operator ==(const ParticipantBuiltinTopicDataImpl& other) const
    {
        return other.key_ == key_ && other.user_data_ == user_data_;
    }

protected:
    dds::topic::BuiltinTopicKey key_;
    ::dds::core::policy::UserData user_data_;
};

class org::opensplice::topic::TopicBuiltinTopicDataImpl
{
public:
    const dds::topic::BuiltinTopicKey& key()
    {
        return key_;
    }
    const std::string&                  name() const
    {
        return name_;
    }
    const std::string&                  type_name() const
    {
        return type_name_;
    }
    const ::dds::core::policy::Durability&         durability() const
    {
        return durability_;
    }


#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

    const ::dds::core::policy::DurabilityService&  durability_service() const
    {
        return durability_service_;
    }

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


    const ::dds::core::policy::Deadline&           deadline() const
    {
        return deadline_;
    }
    const ::dds::core::policy::LatencyBudget&      latency_budget() const
    {
        return latency_budget_;
    }
    const ::dds::core::policy::Liveliness&         liveliness() const
    {
        return liveliness_;
    }
    const ::dds::core::policy::Reliability&        reliability() const
    {
        return reliability_;
    }
    const ::dds::core::policy::TransportPriority&  transport_priority() const
    {
        return transport_priority_;
    }
    const ::dds::core::policy::Lifespan&           lifespan() const
    {
        return lifespan_;
    }
    const ::dds::core::policy::DestinationOrder&   destination_order() const
    {
        return destination_order_;
    }
    const ::dds::core::policy::History&            history() const
    {
        return history_;
    }
    const ::dds::core::policy::ResourceLimits&     resource_limits() const
    {
        return resource_limits_;
    }
    const ::dds::core::policy::Ownership&          ownership() const
    {
        return ownership_;
    }
    const ::dds::core::policy::TopicData&          topic_data() const
    {
        return topic_data_;
    }

    bool operator ==(const TopicBuiltinTopicDataImpl& other) const
    {
        return other.key_ == key_
               && other.name_ == name_
               && other.type_name_ == type_name_
               && other.durability_ == durability_
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
               && other.durability_service_ == durability_service_
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
               && other.deadline_ == deadline_
               && other.latency_budget_ == latency_budget_
               && other.liveliness_ == liveliness_
               && other.reliability_ == reliability_
               && other.transport_priority_ == transport_priority_
               && other.lifespan_ == lifespan_
               && other.destination_order_ == destination_order_
               && other.history_ == history_
               && other.resource_limits_ == resource_limits_
               && other.ownership_ == ownership_
               && other.topic_data_ == topic_data_;
    }

protected:
    dds::topic::BuiltinTopicKey  key_;
    std::string                  name_;
    std::string                  type_name_;
    ::dds::core::policy::Durability         durability_;

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    ::dds::core::policy::DurabilityService  durability_service_;
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

    ::dds::core::policy::Deadline           deadline_;
    ::dds::core::policy::LatencyBudget      latency_budget_;
    ::dds::core::policy::Liveliness         liveliness_;
    ::dds::core::policy::Reliability        reliability_;
    ::dds::core::policy::TransportPriority  transport_priority_;
    ::dds::core::policy::Lifespan           lifespan_;
    ::dds::core::policy::DestinationOrder   destination_order_;
    ::dds::core::policy::History            history_;
    ::dds::core::policy::ResourceLimits     resource_limits_;
    ::dds::core::policy::Ownership          ownership_;
    ::dds::core::policy::TopicData          topic_data_;
};

//==============================================================================
//            PublicationBuiltinTopicDataImpl
//==============================================================================

class org::opensplice::topic::PublicationBuiltinTopicDataImpl
{
public:
    PublicationBuiltinTopicDataImpl() : ownership_strength_(0) { }

    const dds::topic::BuiltinTopicKey& key()
    {
        return key_;
    }

    const dds::topic::BuiltinTopicKey& participant_key()
    {
        return participant_key_;
    }

    const std::string&                  topic_name() const
    {
        return topic_name_;
    }
    const std::string&                  type_name() const
    {
        return type_name_;
    }
    const ::dds::core::policy::Durability&         durability() const
    {
        return durability_;
    }


#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

    const ::dds::core::policy::DurabilityService&  durability_service() const
    {
        return durability_service_;
    }

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


    const ::dds::core::policy::Deadline&           deadline() const
    {
        return deadline_;
    }
    const ::dds::core::policy::LatencyBudget&      latency_budget() const
    {
        return latency_budget_;
    }
    const ::dds::core::policy::Liveliness&         liveliness() const
    {
        return liveliness_;
    }
    const ::dds::core::policy::Reliability&        reliability() const
    {
        return reliability_;
    }
    //    const dds::core::policy::TransportPriority&  transport_priority() const {
    //        return transport_priority_;
    //    }
    const ::dds::core::policy::Lifespan&           lifespan() const
    {
        return lifespan_;
    }
    const ::dds::core::policy::DestinationOrder&   destination_order() const
    {
        return destination_order_;
    }

    const ::dds::core::policy::Ownership&          ownership() const
    {
        return ownership_;
    }


#ifdef  OMG_DDS_OWNERSHIP_SUPPORT

    const ::dds::core::policy::OwnershipStrength&  ownership_strength() const
    {
        return ownership_strength_;
    }

#endif  // OMG_DDS_OWNERSHIP_SUPPORT


    const ::dds::core::policy::Partition&          partition() const
    {
        return partition_;
    }
    const ::dds::core::policy::Presentation&       presentation() const
    {
        return presentation_;
    }

    const ::dds::core::policy::TopicData&          topic_data() const
    {
        return topic_data_;
    }

    const ::dds::core::policy::UserData&           user_data() const
    {
        return user_data_;
    }

    const ::dds::core::policy::GroupData&          group_data() const
    {
        return group_data_;
    }

    bool operator ==(const PublicationBuiltinTopicDataImpl& other) const
    {
        return other.key_ == key_
               && other.participant_key_ == participant_key_
               && other.topic_name_ == topic_name_
               && other.type_name_ == type_name_
               && other.durability_ == durability_
#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
               && other.durability_service_ == durability_service_
#endif  // OMG_DDS_PERSISTENCE_SUPPORT
               && other.deadline_ == deadline_
               && other.latency_budget_ == latency_budget_
               && other.liveliness_ == liveliness_
               && other.reliability_ == reliability_
               && other.lifespan_ == lifespan_
               && other.user_data_ == user_data_
               && other.ownership_ == ownership_
#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
               && other.ownership_strength_ == ownership_strength_
#endif  // OMG_DDS_OWNERSHIP_SUPPORT
               && other.destination_order_ == destination_order_
               && other.topic_data_ == topic_data_
               && other.group_data_ == group_data_;
    }

public:
    dds::topic::BuiltinTopicKey  key_;
    dds::topic::BuiltinTopicKey  participant_key_;
    std::string                  topic_name_;
    std::string                  type_name_;
    ::dds::core::policy::Durability         durability_;

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
    ::dds::core::policy::DurabilityService  durability_service_;
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

    ::dds::core::policy::Deadline           deadline_;
    ::dds::core::policy::LatencyBudget      latency_budget_;
    ::dds::core::policy::Liveliness         liveliness_;
    ::dds::core::policy::Reliability        reliability_;
    ::dds::core::policy::Lifespan           lifespan_;
    ::dds::core::policy::UserData           user_data_;
    ::dds::core::policy::Ownership          ownership_;

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
    ::dds::core::policy::OwnershipStrength  ownership_strength_;
#endif  // OMG_DDS_OWNERSHIP_SUPPORT

    ::dds::core::policy::DestinationOrder   destination_order_;
    ::dds::core::policy::Presentation       presentation_;
    ::dds::core::policy::Partition          partition_;
    ::dds::core::policy::TopicData          topic_data_;
    ::dds::core::policy::GroupData          group_data_;
};

//==============================================================================
//            SubscriptionBuiltinTopicDataImpl
//==============================================================================

class org::opensplice::topic::SubscriptionBuiltinTopicDataImpl
{
public:
    const dds::topic::BuiltinTopicKey& key()
    {
        return key_;
    }

    const dds::topic::BuiltinTopicKey& participant_key()
    {
        return participant_key_;
    }

    const std::string&                  topic_name() const
    {
        return topic_name_;
    }
    const std::string&                  type_name() const
    {
        return type_name_;
    }
    const ::dds::core::policy::Durability&         durability() const
    {
        return durability_;
    }

    const ::dds::core::policy::Deadline&           deadline() const
    {
        return deadline_;
    }
    const ::dds::core::policy::LatencyBudget&      latency_budget() const
    {
        return latency_budget_;
    }
    const ::dds::core::policy::Liveliness&         liveliness() const
    {
        return liveliness_;
    }
    const ::dds::core::policy::Reliability&        reliability() const
    {
        return reliability_;
    }

    const ::dds::core::policy::DestinationOrder&   destination_order() const
    {
        return destination_order_;
    }

    const ::dds::core::policy::TimeBasedFilter& time_based_filter()
    {
        return time_based_filter_;
    }

    const ::dds::core::policy::Ownership&          ownership() const
    {
        return ownership_;
    }
    const ::dds::core::policy::TopicData&          topic_data() const
    {
        return topic_data_;
    }

    const ::dds::core::policy::Partition&          partition() const
    {
        return partition_;
    }
    const ::dds::core::policy::Presentation&       presentation() const
    {
        return presentation_;
    }

    const ::dds::core::policy::UserData&           user_data() const
    {
        return user_data_;
    }

    const ::dds::core::policy::GroupData&          group_data() const
    {
        return group_data_;
    }

    bool operator ==(const SubscriptionBuiltinTopicDataImpl& other) const
    {
        return other.key_ == key_
               && other.participant_key_ == participant_key_
               && other.topic_name_ == topic_name_
               && other.type_name_ == type_name_
               && other.durability_ == durability_
               && other.deadline_ == deadline_
               && other.latency_budget_ == latency_budget_
               && other.liveliness_ == liveliness_
               && other.reliability_ == reliability_
               && other.ownership_ == ownership_
               && other.destination_order_ == destination_order_
               && other.user_data_ == user_data_
               && other.time_based_filter_ == time_based_filter_
               && other.presentation_ == presentation_
               && other.partition_ == partition_
               && other.topic_data_ == topic_data_
               && other.group_data_ == group_data_;
    }

public:
    dds::topic::BuiltinTopicKey  key_;
    dds::topic::BuiltinTopicKey  participant_key_;
    std::string                  topic_name_;
    std::string                  type_name_;
    ::dds::core::policy::Durability         durability_;
    ::dds::core::policy::Deadline           deadline_;
    ::dds::core::policy::LatencyBudget      latency_budget_;
    ::dds::core::policy::Liveliness         liveliness_;
    ::dds::core::policy::Reliability        reliability_;
    ::dds::core::policy::Ownership          ownership_;
    ::dds::core::policy::DestinationOrder   destination_order_;
    ::dds::core::policy::UserData           user_data_;
    ::dds::core::policy::TimeBasedFilter    time_based_filter_;
    ::dds::core::policy::Presentation       presentation_;
    ::dds::core::policy::Partition          partition_;
    ::dds::core::policy::TopicData          topic_data_;
    ::dds::core::policy::GroupData          group_data_;
};

//==============================================================================
//            CMParticipantBuiltinTopicData
//==============================================================================

class org::opensplice::topic::CMParticipantBuiltinTopicDataImpl
{
public:
    const dds::topic::BuiltinTopicKey&        key() const
    {
        return key_;
    }

    const ::dds::core::policy::ProductData&   product() const
    {
        return product_;
    }

    bool operator ==(const CMParticipantBuiltinTopicDataImpl& other) const
    {
        return other.key_ == key_
               && other.product_ == product_;
    }

protected:
    dds::topic::BuiltinTopicKey        key_;
    ::dds::core::policy::ProductData   product_;
};

//==============================================================================
//            CMPublisherBuiltinTopicData
//==============================================================================

class org::opensplice::topic::CMPublisherBuiltinTopicDataImpl
{
public:
    const dds::topic::BuiltinTopicKey&        key() const
    {
        return key_;
    }

    const ::dds::core::policy::ProductData&   product() const
    {
        return product_;
    }

    const dds::topic::BuiltinTopicKey&        participant_key() const
    {
        return participant_key_;
    }

    const std::string&                        name() const
    {
        return name_;
    }

    const ::dds::core::policy::EntityFactory& entity_factory() const
    {
        return entity_factory_;
    }

    const ::dds::core::policy::Partition&     partition() const
    {
        return partition_;
    }

    bool operator ==(const CMPublisherBuiltinTopicDataImpl& other) const
    {
        return other.key_ == key_
               && other.product_ == product_
               && other.participant_key_ == participant_key_
               && other.name_ == name_
               && other.entity_factory_ == entity_factory_
               && other.partition_ == partition_;
    }

protected:
    dds::topic::BuiltinTopicKey        key_;
    ::dds::core::policy::ProductData   product_;
    dds::topic::BuiltinTopicKey        participant_key_;
    std::string                        name_;
    ::dds::core::policy::EntityFactory entity_factory_;
    ::dds::core::policy::Partition     partition_;
};

//==============================================================================
//            CMSubscriberBuiltinTopicData
//==============================================================================

class org::opensplice::topic::CMSubscriberBuiltinTopicDataImpl
{
public:
    const dds::topic::BuiltinTopicKey&        key() const
    {
        return key_;
    }

    const ::dds::core::policy::ProductData&   product() const
    {
        return product_;
    }

    const dds::topic::BuiltinTopicKey&        participant_key() const
    {
        return participant_key_;
    }

    const std::string&                        name() const
    {
        return name_;
    }

    const ::dds::core::policy::EntityFactory& entity_factory() const
    {
        return entity_factory_;
    }

    const ::dds::core::policy::Partition&     partition() const
    {
        return partition_;
    }

    const ::dds::core::policy::Share&         share() const
    {
        return share_;
    }

    bool operator ==(const CMSubscriberBuiltinTopicDataImpl& other) const
    {
        return other.key_ == key_
               && other.product_ == product_
               && other.participant_key_ == participant_key_
               && other.name_ == name_
               && other.entity_factory_ == entity_factory_
               && other.partition_ == partition_
               && other.share_ == share_;
    }

protected:
    dds::topic::BuiltinTopicKey        key_;
    ::dds::core::policy::ProductData   product_;
    dds::topic::BuiltinTopicKey        participant_key_;
    std::string                        name_;
    ::dds::core::policy::EntityFactory entity_factory_;
    ::dds::core::policy::Partition     partition_;
    ::dds::core::policy::Share         share_;
};

//==============================================================================
//            CMDataWriterBuiltinTopicData
//==============================================================================

class org::opensplice::topic::CMDataWriterBuiltinTopicDataImpl
{
public:
    const dds::topic::BuiltinTopicKey&              key() const
    {
        return key_;
    }

    const ::dds::core::policy::ProductData&         product() const
    {
        return product_;
    }

    const dds::topic::BuiltinTopicKey&              publisher_key() const
    {
        return publisher_key_;
    }

    const std::string&                              name() const
    {
        return name_;
    }

    const ::dds::core::policy::History&             history() const
    {
        return history_;
    }

    const ::dds::core::policy::ResourceLimits&      resource_limits() const
    {
        return resource_limits_;
    }

    const ::dds::core::policy::WriterDataLifecycle& writer_data_lifecycle() const
    {
        return writer_data_lifecycle_;
    }

    bool operator ==(const CMDataWriterBuiltinTopicDataImpl& other) const
    {
        return other.key_ == key_
               && other.product_ == product_
               && other.publisher_key_ == publisher_key_
               && other.name_ == name_
               && other.history_ == history_
               && other.resource_limits_ == resource_limits_
               && other.writer_data_lifecycle_ == writer_data_lifecycle_;
    }

protected:
    dds::topic::BuiltinTopicKey              key_;
    ::dds::core::policy::ProductData         product_;
    dds::topic::BuiltinTopicKey              publisher_key_;
    std::string                              name_;
    ::dds::core::policy::History             history_;
    ::dds::core::policy::ResourceLimits      resource_limits_;
    ::dds::core::policy::WriterDataLifecycle writer_data_lifecycle_;
};

//==============================================================================
//            CMDataReaderBuiltinTopicData
//==============================================================================

class org::opensplice::topic::CMDataReaderBuiltinTopicDataImpl
{
public:
    const dds::topic::BuiltinTopicKey&              key() const
    {
        return key_;
    }

    const ::dds::core::policy::ProductData&         product() const
    {
        return product_;
    }

    const dds::topic::BuiltinTopicKey&              subscriber_key() const
    {
        return subscriber_key_;
    }

    const std::string&                              name() const
    {
        return name_;
    }

    const ::dds::core::policy::History&             history() const
    {
        return history_;
    }

    const ::dds::core::policy::ResourceLimits&      resource_limits() const
    {
        return resource_limits_;
    }

    const ::dds::core::policy::ReaderDataLifecycle& reader_data_lifecycle() const
    {
        return reader_data_lifecycle_;
    }

    const ::dds::core::policy::SubscriptionKey&     subscription_keys() const
    {
        return subscription_keys_;
    }

    const ::dds::core::policy::Lifespan&            reader_lifespan() const
    {
        return reader_lifespan_;
    }

    const ::dds::core::policy::Share&               share() const
    {
        return share_;
    }

    bool operator ==(const CMDataReaderBuiltinTopicDataImpl& other) const
    {
        return other.key_ == key_
               && other.product_ == product_
               && other.subscriber_key_ == subscriber_key_
               && other.name_ == name_
               && other.history_ == history_
               && other.resource_limits_ == resource_limits_
               && other.reader_data_lifecycle_ == reader_data_lifecycle_
               && other.subscription_keys_ == subscription_keys_
               && other.reader_lifespan_ == reader_lifespan_
               && other.share_ == share_;
    }

protected:
    dds::topic::BuiltinTopicKey              key_;
    ::dds::core::policy::ProductData         product_;
    dds::topic::BuiltinTopicKey              subscriber_key_;
    std::string                              name_;
    ::dds::core::policy::History             history_;
    ::dds::core::policy::ResourceLimits      resource_limits_;
    ::dds::core::policy::ReaderDataLifecycle reader_data_lifecycle_;
    ::dds::core::policy::SubscriptionKey     subscription_keys_;
    ::dds::core::policy::Lifespan            reader_lifespan_;
    ::dds::core::policy::Share               share_;
};


#endif /* ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_IMPL_HPP */
