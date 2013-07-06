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

}
}
// End of implementation

#endif /* OSPL_DDS_TOPIC_TBUILTINTOPIC_HPP_ */
