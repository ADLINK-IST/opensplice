/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef OSPL_DDS_TOPIC_TBUILTINTOPIC_HPP_
#define OSPL_DDS_TOPIC_TBUILTINTOPIC_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/TBuiltinTopic.hpp>

// Implementation
namespace dds
{
namespace topic
{

//TParticipantBuiltinTopicData
template <typename D>
const dds::topic::BuiltinTopicKey& TParticipantBuiltinTopicData<D>::key() const
{
    return ((D)this->delegate()).key();
}

template <typename D>
const ::dds::core::policy::UserData& TParticipantBuiltinTopicData<D>::user_data() const
{
    return ((D)this->delegate()).user_data();
}

//TTopicBuiltinTopicData
template <typename D>
const dds::topic::BuiltinTopicKey& TTopicBuiltinTopicData<D>::key()
{
    return this->delegate().key();
}

template <typename D>
const std::string& TTopicBuiltinTopicData<D>::name() const
{
    return ((D)this->delegate()).name();
}

template <typename D>
const std::string& TTopicBuiltinTopicData<D>::type_name() const
{
    return ((D)this->delegate()).type_name();
}

template <typename D>
const ::dds::core::policy::Durability& TTopicBuiltinTopicData<D>::durability() const
{
    return ((D)this->delegate()).durability();
}

#ifdef OMG_DDS_PERSISTENCE_SUPPORT
template <typename D>
const ::dds::core::policy::DurabilityService& TTopicBuiltinTopicData<D>::durability_service() const
{
    return ((D)this->delegate()).durability_service();
}

#endif  // OMG_DDS_PERSISTENCE_SUPPORT

template <typename D>
const ::dds::core::policy::Deadline& TTopicBuiltinTopicData<D>::deadline() const
{
    return ((D)this->delegate()).deadline();
}

template <typename D>
const ::dds::core::policy::LatencyBudget& TTopicBuiltinTopicData<D>::latency_budget() const
{
    return ((D)this->delegate()).latency_budget();
}

template <typename D>
const ::dds::core::policy::Liveliness& TTopicBuiltinTopicData<D>::liveliness() const
{
    return ((D)this->delegate()).liveliness();
}

template <typename D>
const ::dds::core::policy::Reliability& TTopicBuiltinTopicData<D>::reliability() const
{
    return ((D)this->delegate()).reliability();
}

template <typename D>
const ::dds::core::policy::TransportPriority& TTopicBuiltinTopicData<D>::transport_priority() const
{
    return ((D)this->delegate()).transport_priority();
}

template <typename D>
const ::dds::core::policy::Lifespan& TTopicBuiltinTopicData<D>::lifespan() const
{
    return ((D)this->delegate()).lifespan();
}

template <typename D>
const ::dds::core::policy::DestinationOrder& TTopicBuiltinTopicData<D>::destination_order() const
{
    return ((D)this->delegate()).destination_order();
}

template <typename D>
const ::dds::core::policy::History& TTopicBuiltinTopicData<D>::history() const
{
    return ((D)this->delegate()).history();
}

template <typename D>
const ::dds::core::policy::ResourceLimits& TTopicBuiltinTopicData<D>::resource_limits() const
{
    return ((D)this->delegate()).resource_limits();
}

template <typename D>
const ::dds::core::policy::Ownership& TTopicBuiltinTopicData<D>::ownership() const
{
    return ((D)this->delegate()).ownership();
}

template <typename D>
const ::dds::core::policy::TopicData& TTopicBuiltinTopicData<D>::topic_data() const
{
    return ((D)this->delegate()).topic_data();
}

//TPublicationBuiltinTopicData

template <typename D>
const dds::topic::BuiltinTopicKey&  TPublicationBuiltinTopicData<D>::key() const
{
    return ((D)this->delegate()).key();
}

template <typename D>
const dds::topic::BuiltinTopicKey& TPublicationBuiltinTopicData<D>::participant_key() const
{
    return ((D)this->delegate()).key();
}

template <typename D>
const std::string& TPublicationBuiltinTopicData<D>::topic_name() const
{
    return ((D)this->delegate()).topic_name();
}

template <typename D>
const std::string& TPublicationBuiltinTopicData<D>::type_name() const
{
    return ((D)this->delegate()).type_name();
}

template <typename D>
const ::dds::core::policy::Durability& TPublicationBuiltinTopicData<D>::durability() const
{
    return ((D)this->delegate()).durability();
}

#ifdef OMG_DDS_PERSISTENCE_SUPPORT

template <typename D>
const ::dds::core::policy::DurabilityService& TPublicationBuiltinTopicData<D>::durability_service() const
{
    return ((D)this->delegate()).durability_service();
}
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

template <typename D>
const ::dds::core::policy::Deadline& TPublicationBuiltinTopicData<D>::deadline() const
{
    return ((D)this->delegate()).deadline();
}

template <typename D>
const ::dds::core::policy::LatencyBudget& TPublicationBuiltinTopicData<D>::latency_budget() const
{
    return ((D)this->delegate()).latency_budget();
}

template <typename D>
const ::dds::core::policy::Liveliness& TPublicationBuiltinTopicData<D>::liveliness() const
{
    return ((D)this->delegate()).liveliness();
}

template <typename D>
const ::dds::core::policy::Reliability& TPublicationBuiltinTopicData<D>::reliability() const
{
    return ((D)this->delegate()).reliability();
}


template <typename D>
const ::dds::core::policy::Lifespan& TPublicationBuiltinTopicData<D>::lifespan() const
{
    return ((D)this->delegate()).lifespan();
}

template <typename D>
const ::dds::core::policy::UserData& TPublicationBuiltinTopicData<D>::user_data() const
{
    return ((D)this->delegate()).user_data();
}

template <typename D>
const ::dds::core::policy::Ownership& TPublicationBuiltinTopicData<D>::ownership() const
{
    return ((D)this->delegate()).ownership();
}

#ifdef OMG_DDS_OWNERSHIP_SUPPORT

template <typename D>
const ::dds::core::policy::OwnershipStrength& TPublicationBuiltinTopicData<D>::ownership_strength() const
{
    return ((D)this->delegate()).ownership_strength();
}
#endif  // OMG_DDS_OWNERSHIP_SUPPORT


template <typename D>
const ::dds::core::policy::DestinationOrder& TPublicationBuiltinTopicData<D>::destination_order() const
{
    return ((D)this->delegate()).destination_order();
}

template <typename D>
const ::dds::core::policy::Presentation& TPublicationBuiltinTopicData<D>::presentation() const
{
    return ((D)this->delegate()).presentation();
}

template <typename D>
const ::dds::core::policy::Partition& TPublicationBuiltinTopicData<D>::partition() const
{
    return ((D)this->delegate()).partition();
}

template <typename D>
const ::dds::core::policy::TopicData& TPublicationBuiltinTopicData<D>::topic_data() const
{
    return ((D)this->delegate()).topic_data();
}

template <typename D>
const ::dds::core::policy::GroupData& TPublicationBuiltinTopicData<D>::group_data() const
{
    return ((D)this->delegate()).group_data();
}



template <typename D>
const dds::topic::BuiltinTopicKey& TSubscriptionBuiltinTopicData<D>::key() const
{
    return ((D)this->delegate()).key();
}

template <typename D>
const dds::topic::BuiltinTopicKey& TSubscriptionBuiltinTopicData<D>::participant_key() const
{
    return ((D)this->delegate()).key();
}

template <typename D>
const std::string& TSubscriptionBuiltinTopicData<D>::topic_name() const
{
    return ((D)this->delegate()).topic_name();
}

template <typename D>
const std::string& TSubscriptionBuiltinTopicData<D>::type_name() const
{
    return ((D)this->delegate()).type_name();
}

template <typename D>
const ::dds::core::policy::Durability& TSubscriptionBuiltinTopicData<D>::durability() const
{
    return ((D)this->delegate()).durability();
}

template <typename D>
const ::dds::core::policy::Deadline& TSubscriptionBuiltinTopicData<D>::deadline() const
{
    return ((D)this->delegate()).deadline();
}

template <typename D>
const ::dds::core::policy::LatencyBudget& TSubscriptionBuiltinTopicData<D>::latency_budget() const
{
    return ((D)this->delegate()).latency_budget();
}

template <typename D>
const ::dds::core::policy::Liveliness& TSubscriptionBuiltinTopicData<D>::liveliness() const
{
    return ((D)this->delegate()).liveliness();
}

template <typename D>
const ::dds::core::policy::Reliability& TSubscriptionBuiltinTopicData<D>::reliability() const
{
    return ((D)this->delegate()).reliability();
}

template <typename D>
const ::dds::core::policy::Ownership& TSubscriptionBuiltinTopicData<D>::ownership() const
{
    return ((D)this->delegate()).ownership();
}

template <typename D>
const ::dds::core::policy::DestinationOrder& TSubscriptionBuiltinTopicData<D>::destination_order() const
{
    return ((D)this->delegate()).destination_order();
}

template <typename D>
const ::dds::core::policy::UserData& TSubscriptionBuiltinTopicData<D>::user_data() const
{
    return ((D)this->delegate()).user_data();
}

template <typename D>
const ::dds::core::policy::TimeBasedFilter& TSubscriptionBuiltinTopicData<D>::time_based_filter() const
{
    return ((D)this->delegate()).time_based_filter();
}

template <typename D>
const ::dds::core::policy::Presentation& TSubscriptionBuiltinTopicData<D>::presentation() const
{
    return ((D)this->delegate()).presentation();
}

template <typename D>
const ::dds::core::policy::Partition& TSubscriptionBuiltinTopicData<D>::partition() const
{
    return ((D)this->delegate()).partition();
}

template <typename D>
const ::dds::core::policy::TopicData& TSubscriptionBuiltinTopicData<D>::topic_data() const
{
    return ((D)this->delegate()).topic_data();
}

template <typename D>
const ::dds::core::policy::GroupData& TSubscriptionBuiltinTopicData<D>::group_data() const
{
    return ((D)this->delegate()).group_data();
}

//TCMParticipantBuiltinTopicData

template <typename D>
const dds::topic::BuiltinTopicKey& TCMParticipantBuiltinTopicData<D>::key() const
{
    return ((D)this->delegate()).key();
}

template <typename D>
const ::dds::core::policy::ProductData& TCMParticipantBuiltinTopicData<D>::product() const
{
    return ((D)this->delegate()).product();
}

//TCMPublisherBuiltinTopicData

template <typename D>
const dds::topic::BuiltinTopicKey& TCMPublisherBuiltinTopicData<D>::key() const
{
    return ((D)this->delegate()).key();
}

template <typename D>
const ::dds::core::policy::ProductData& TCMPublisherBuiltinTopicData<D>::product() const
{
    return ((D)this->delegate()).product();
}

template <typename D>
const dds::topic::BuiltinTopicKey& TCMPublisherBuiltinTopicData<D>::participant_key() const
{
    return ((D)this->delegate()).participant_key();
}

template <typename D>
const std::string& TCMPublisherBuiltinTopicData<D>::name() const
{
    return ((D)this->delegate()).name();
}

template <typename D>
const ::dds::core::policy::EntityFactory& TCMPublisherBuiltinTopicData<D>::entity_factory() const
{
    return ((D)this->delegate()).entity_factory();
}

template <typename D>
const ::dds::core::policy::Partition& TCMPublisherBuiltinTopicData<D>::partition() const
{
    return ((D)this->delegate()).partition();
}

//TCMSubscriberBuiltinTopicData

template <typename D>
const dds::topic::BuiltinTopicKey& TCMSubscriberBuiltinTopicData<D>::key() const
{
    return ((D)this->delegate()).key();
}

template <typename D>
const ::dds::core::policy::ProductData& TCMSubscriberBuiltinTopicData<D>::product() const
{
    return ((D)this->delegate()).product();
}

template <typename D>
const dds::topic::BuiltinTopicKey& TCMSubscriberBuiltinTopicData<D>::participant_key() const
{
    return ((D)this->delegate()).participant_key();
}

template <typename D>
const std::string& TCMSubscriberBuiltinTopicData<D>::name() const
{
    return ((D)this->delegate()).name();
}

template <typename D>
const ::dds::core::policy::EntityFactory& TCMSubscriberBuiltinTopicData<D>::entity_factory() const
{
    return ((D)this->delegate()).entity_factory();
}

template <typename D>
const ::dds::core::policy::Partition& TCMSubscriberBuiltinTopicData<D>::partition() const
{
    return ((D)this->delegate()).partition();
}

template <typename D>
const ::dds::core::policy::Share& TCMSubscriberBuiltinTopicData<D>::share() const
{
    return ((D)this->delegate()).share();
}

//TCMDataWriterBuiltinTopicData

template <typename D>
const dds::topic::BuiltinTopicKey& TCMDataWriterBuiltinTopicData<D>::key() const
{
    return ((D)this->delegate()).key();
}

template <typename D>
const ::dds::core::policy::ProductData& TCMDataWriterBuiltinTopicData<D>::product() const
{
    return ((D)this->delegate()).product();
}

template <typename D>
const dds::topic::BuiltinTopicKey& TCMDataWriterBuiltinTopicData<D>::publisher_key() const
{
    return ((D)this->delegate()).publisher_key();
}

template <typename D>
const std::string& TCMDataWriterBuiltinTopicData<D>::name() const
{
    return ((D)this->delegate()).name();
}

template <typename D>
const ::dds::core::policy::History& TCMDataWriterBuiltinTopicData<D>::history() const
{
    return ((D)this->delegate()).history();
}

template <typename D>
const ::dds::core::policy::ResourceLimits& TCMDataWriterBuiltinTopicData<D>::resource_limits() const
{
    return ((D)this->delegate()).resource_limits();
}

template <typename D>
const ::dds::core::policy::WriterDataLifecycle& TCMDataWriterBuiltinTopicData<D>::writer_data_lifecycle() const
{
    return ((D)this->delegate()).writer_data_lifecycle();
}

//TCMDataReaderBuiltinTopicData

template <typename D>
const dds::topic::BuiltinTopicKey& TCMDataReaderBuiltinTopicData<D>::key() const
{
    return ((D)this->delegate()).key();
}

template <typename D>
const ::dds::core::policy::ProductData& TCMDataReaderBuiltinTopicData<D>::product() const
{
    return ((D)this->delegate()).product();
}

template <typename D>
const dds::topic::BuiltinTopicKey& TCMDataReaderBuiltinTopicData<D>::subscriber_key() const
{
    return ((D)this->delegate()).subscriber_key();
}

template <typename D>
const std::string& TCMDataReaderBuiltinTopicData<D>::name() const
{
    return ((D)this->delegate()).name();
}

template <typename D>
const ::dds::core::policy::History& TCMDataReaderBuiltinTopicData<D>::history() const
{
    return ((D)this->delegate()).history();
}

template <typename D>
const ::dds::core::policy::ResourceLimits& TCMDataReaderBuiltinTopicData<D>::resource_limits() const
{
    return ((D)this->delegate()).resource_limits();
}

template <typename D>
const ::dds::core::policy::ReaderDataLifecycle& TCMDataReaderBuiltinTopicData<D>::reader_data_lifecycle() const
{
    return ((D)this->delegate()).reader_data_lifecycle();
}

template <typename D>
const ::dds::core::policy::SubscriptionKey& TCMDataReaderBuiltinTopicData<D>::subscription_keys() const
{
    return ((D)this->delegate()).subscription_keys();
}

template <typename D>
const ::dds::core::policy::Lifespan& TCMDataReaderBuiltinTopicData<D>::reader_lifespan() const
{
    return ((D)this->delegate()).reader_lifespan();
}

template <typename D>
const ::dds::core::policy::Share& TCMDataReaderBuiltinTopicData<D>::share() const
{
    return ((D)this->delegate()).share();
}


}
}
// End of implementation

#endif /* OSPL_DDS_TOPIC_TBUILTINTOPIC_HPP_ */
