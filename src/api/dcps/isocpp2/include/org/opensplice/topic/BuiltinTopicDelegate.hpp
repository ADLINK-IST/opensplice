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

#ifndef ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_DELEGATE_HPP
#define ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_DELEGATE_HPP


#include <dds/core/detail/conformance.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include <dds/topic/BuiltinTopicKey.hpp>
#include <org/opensplice/topic/TypeHash.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{
class ParticipantBuiltinTopicDataDelegate;
class TopicBuiltinTopicDataDelegate;
class PublicationBuiltinTopicDataDelegate;
class SubscriptionBuiltinTopicDataDelegate;
class CMParticipantBuiltinTopicDataDelegate;
class CMPublisherBuiltinTopicDataDelegate;
class CMSubscriberBuiltinTopicDataDelegate;
class CMDataWriterBuiltinTopicDataDelegate;
class CMDataReaderBuiltinTopicDataDelegate;
class TypeBuiltinTopicDataDelegate;

/* X-Types builtin. */
class BytesTopicTypeDelegate;
class StringTopicTypeDelegate;
class KeyedBytesTopicTypeDelegate;
class KeyedStringTopicTypeDelegate;

}
}
}

//==============================================================================
//            ParticipantBuiltinTopicDataDelegate
//==============================================================================

class org::opensplice::topic::ParticipantBuiltinTopicDataDelegate
{
public:
    const dds::topic::BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(const dds::topic::BuiltinTopicKey& key)
    {
        key_.delegate() = key;
    }

    void key(const v_builtinTopicKey& key)
    {
        key_.delegate().value(key);
    }

    const ::dds::core::policy::UserData& user_data() const
    {
        return user_data_;
    }

    void user_data(const dds::core::policy::UserData& policy)
    {
        user_data_.delegate() = policy;
    }

    void user_data(const v_userDataPolicy& policy)
    {
        user_data_.delegate().v_policy(policy);
    }

    void user_data(const v_userDataPolicyI& policy)
    {
        user_data_.delegate().v_policyI(policy);
    }

    void user_data(const v_builtinUserDataPolicy& policy)
    {
        user_data_.delegate().v_policy(policy);
    }

    bool operator ==(const ParticipantBuiltinTopicDataDelegate& other) const
    {
        return other.key_ == key_ && other.user_data_ == user_data_;
    }

protected:
    dds::topic::BuiltinTopicKey key_;
    ::dds::core::policy::UserData user_data_;
};

class org::opensplice::topic::TopicBuiltinTopicDataDelegate
{
public:
    const dds::topic::BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(const dds::topic::BuiltinTopicKey& key)
    {
        key_.delegate() = key;
    }

    void key(const v_builtinTopicKey& key)
    {
        key_.delegate().value(key);
    }

    const std::string& name() const
    {
        return name_;
    }

    void name(const char *name)
    {
        name_ = name;
    }

    const std::string& type_name() const
    {
        return type_name_;
    }

    void type_name(const char *name)
    {
        type_name_ = name;
    }

    const ::dds::core::policy::Durability& durability() const
    {
        return durability_;
    }

    void durability(const dds::core::policy::Durability& policy)
    {
        durability_.delegate() = policy;
    }

    void durability(const v_durabilityPolicy& policy)
    {
        durability_.delegate().v_policy(policy);
    }


#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

    const ::dds::core::policy::DurabilityService& durability_service() const
    {
        return durability_service_;
    }

    void durability_service(const dds::core::policy::DurabilityService& policy)
    {
        durability_service_.delegate() = policy;
    }

    void durability_service(const v_durabilityServicePolicy& policy)
    {
        durability_service_.delegate().v_policy(policy);
    }

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


    const ::dds::core::policy::Deadline& deadline() const
    {
        return deadline_;
    }

    void deadline(const dds::core::policy::Deadline& policy)
    {
        deadline_.delegate() = policy;
    }

    void deadline(const v_deadlinePolicy& policy)
    {
        deadline_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::LatencyBudget& latency_budget() const
    {
        return latency_budget_;
    }

    void latency_budget(const dds::core::policy::LatencyBudget& policy)
    {
        latency_budget_.delegate() = policy;
    }

    void latency_budget(const v_latencyPolicy& policy)
    {
        latency_budget_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Liveliness& liveliness() const
    {
        return liveliness_;
    }

    void liveliness(const dds::core::policy::Liveliness& policy)
    {
        liveliness_.delegate() = policy;
    }

    void liveliness(const v_livelinessPolicy& policy)
    {
        liveliness_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Reliability& reliability() const
    {
        return reliability_;
    }

    void reliability(const dds::core::policy::Reliability& policy)
    {
        reliability_.delegate() = policy;
    }

    void reliability(const v_reliabilityPolicy& policy)
    {
        reliability_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::TransportPriority& transport_priority() const
    {
        return transport_priority_;
    }

    void transport_priority(const dds::core::policy::TransportPriority& policy)
    {
        transport_priority_.delegate() = policy;
    }

    void transport_priority(const v_transportPolicy& policy)
    {
        transport_priority_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Lifespan& lifespan() const
    {
        return lifespan_;
    }

    void lifespan(const dds::core::policy::Lifespan& policy)
    {
        lifespan_.delegate() = policy;
    }

    void lifespan(const v_lifespanPolicy& policy)
    {
        lifespan_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::DestinationOrder& destination_order() const
    {
        return destination_order_;
    }

    void destination_order(const dds::core::policy::DestinationOrder& policy)
    {
        destination_order_.delegate() = policy;
    }

    void destination_order(const v_orderbyPolicy& policy)
    {
        destination_order_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::History& history() const
    {
        return history_;
    }

    void history(const dds::core::policy::History& policy)
    {
        history_.delegate() = policy;
    }

    void history(const v_historyPolicy& policy)
    {
        history_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::ResourceLimits& resource_limits() const
    {
        return resource_limits_;
    }

    void resource_limits(const dds::core::policy::ResourceLimits& policy)
    {
        resource_limits_.delegate() = policy;
    }

    void resource_limits(const v_resourcePolicy& policy)
    {
        resource_limits_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Ownership& ownership() const
    {
        return ownership_;
    }

    void ownership(const dds::core::policy::Ownership& policy)
    {
        ownership_.delegate() = policy;
    }

    void ownership(const v_ownershipPolicy& policy)
    {
        ownership_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::TopicData& topic_data() const
    {
        return topic_data_;
    }

    void topic_data(const dds::core::policy::TopicData& policy)
    {
        topic_data_.delegate() = policy;
    }

    void topic_data(const v_builtinTopicDataPolicy& policy)
    {
        topic_data_.delegate().v_policy(policy);
    }

    bool operator ==(const TopicBuiltinTopicDataDelegate& other) const
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
//            PublicationBuiltinTopicDataDelegate
//==============================================================================

class org::opensplice::topic::PublicationBuiltinTopicDataDelegate
{
public:
    PublicationBuiltinTopicDataDelegate() : ownership_strength_(0) { }

    const dds::topic::BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(const dds::topic::BuiltinTopicKey& key)
    {
        key_.delegate() = key;
    }

    void key(const v_builtinTopicKey& key)
    {
        key_.delegate().value(key);
    }

    const dds::topic::BuiltinTopicKey& participant_key() const
    {
        return participant_key_;
    }

    void participant_key(const dds::topic::BuiltinTopicKey& key)
    {
        participant_key_.delegate() = key;
    }

    void participant_key(const v_builtinTopicKey& key)
    {
        participant_key_.delegate().value(key);
    }

    const std::string& topic_name() const
    {
        return topic_name_;
    }

    void topic_name(const char *name)
    {
        topic_name_ = name;
    }

    const std::string& type_name() const
    {
        return type_name_;
    }

    void type_name(const char *name)
    {
        type_name_ = name;
    }

    const ::dds::core::policy::Durability& durability() const
    {
        return durability_;
    }

    void durability(const dds::core::policy::Durability& policy)
    {
        durability_.delegate() = policy;
    }

    void durability(const v_durabilityPolicy& policy)
    {
        durability_.delegate().v_policy(policy);
    }

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

    const ::dds::core::policy::DurabilityService&  durability_service() const
    {
        return durability_service_;
    }

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


    const ::dds::core::policy::Deadline& deadline() const
    {
        return deadline_;
    }

    void deadline(const dds::core::policy::Deadline& policy)
    {
        deadline_.delegate() = policy;
    }

    void deadline(const v_deadlinePolicy& policy)
    {
        deadline_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::LatencyBudget& latency_budget() const
    {
        return latency_budget_;
    }

    void latency_budget(const dds::core::policy::LatencyBudget& policy)
    {
        latency_budget_.delegate() = policy;
    }

    void latency_budget(const v_latencyPolicy& policy)
    {
        latency_budget_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Liveliness& liveliness() const
    {
        return liveliness_;
    }

    void liveliness(const dds::core::policy::Liveliness& policy)
    {
        liveliness_.delegate() = policy;
    }

    void liveliness(const v_livelinessPolicy& policy)
    {
        liveliness_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Reliability& reliability() const
    {
        return reliability_;
    }

    void reliability(const dds::core::policy::Reliability& policy)
    {
        reliability_.delegate() = policy;
    }

    void reliability(const v_reliabilityPolicy& policy)
    {
        reliability_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Lifespan& lifespan() const
    {
        return lifespan_;
    }

    void lifespan(const dds::core::policy::Lifespan& policy)
    {
        lifespan_.delegate()= policy;
    }

    void lifespan(const v_lifespanPolicy& policy)
    {
        lifespan_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::DestinationOrder& destination_order() const
    {
        return destination_order_;
    }

    void destination_order(const dds::core::policy::DestinationOrder& policy)
    {
        destination_order_.delegate() = policy;
    }

    void destination_order(const v_orderbyPolicy& policy)
    {
        destination_order_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Ownership& ownership() const
    {
        return ownership_;
    }

    void ownership(const dds::core::policy::Ownership& policy)
    {
        ownership_.delegate() = policy;
    }

    void ownership(const v_ownershipPolicy& policy)
    {
        ownership_.delegate().v_policy(policy);
    }

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT

    const ::dds::core::policy::OwnershipStrength&  ownership_strength() const
    {
        return ownership_strength_;
    }

    void ownership_strength(const dds::core::policy::OwnershipStrength& policy)
    {
        ownership_strength_.delegate() = policy;
    }

    void ownership_strength(const v_strengthPolicy& policy)
    {
        ownership_strength_.delegate().v_policy(policy);
    }

#endif  // OMG_DDS_OWNERSHIP_SUPPORT


    const ::dds::core::policy::Partition& partition() const
    {
        return partition_;
    }

    void partition(const dds::core::policy::Partition& policy)
    {
        partition_.delegate() = policy;
    }

    void partition(const v_builtinPartitionPolicy& policy)
    {
        partition_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Presentation& presentation() const
    {
        return presentation_;
    }

    void presentation(const dds::core::policy::Presentation& policy)
    {
        presentation_.delegate() = policy;
    }

    void presentation(const v_presentationPolicy& policy)
    {
        presentation_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::TopicData& topic_data() const
    {
        return topic_data_;
    }

    void topic_data(const dds::core::policy::TopicData& policy)
    {
        topic_data_.delegate() = policy;
    }

    void topic_data(const v_builtinTopicDataPolicy& policy)
    {
        topic_data_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::UserData& user_data() const
    {
        return user_data_;
    }

    void user_data(const dds::core::policy::UserData& policy)
    {
        user_data_.delegate() = policy;
    }

    void user_data(const v_builtinUserDataPolicy& policy)
    {
        user_data_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::GroupData& group_data() const
    {
        return group_data_;
    }

    void group_data(const dds::core::policy::GroupData& policy)
    {
        group_data_.delegate() = policy;
    }

    void group_data(const v_builtinGroupDataPolicy& policy)
    {
        group_data_.delegate().v_policy(policy);
    }


    bool operator ==(const PublicationBuiltinTopicDataDelegate& other) const
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
//            SubscriptionBuiltinTopicDataDelegate
//==============================================================================

class org::opensplice::topic::SubscriptionBuiltinTopicDataDelegate
{
public:
    const dds::topic::BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(const dds::topic::BuiltinTopicKey& key)
    {
        key_.delegate() = key;
    }

    void key(const v_builtinTopicKey& key)
    {
        key_.delegate().value(key);
    }

    const dds::topic::BuiltinTopicKey& participant_key() const
    {
        return participant_key_;
    }

    void participant_key(const dds::topic::BuiltinTopicKey& key)
    {
        participant_key_.delegate() = key;
    }

    void participant_key(const v_builtinTopicKey& key)
    {
        participant_key_.delegate().value(key);
    }

    const std::string& topic_name() const
    {
        return topic_name_;
    }

    void topic_name(const char *name)
    {
        topic_name_ = name;
    }

    const std::string& type_name() const
    {
        return type_name_;
    }

    void type_name(const char *name)
    {
        type_name_ = name;
    }

    const ::dds::core::policy::Durability& durability() const
    {
        return durability_;
    }

    void durability(const dds::core::policy::Durability& policy)
    {
        durability_.delegate() = policy;
    }

    void durability(const v_durabilityPolicy& policy)
    {
        durability_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Deadline& deadline() const
    {
        return deadline_;
    }

    void deadline(const dds::core::policy::Deadline& policy)
    {
        deadline_.delegate() = policy;
    }

    void deadline(const v_deadlinePolicy& policy)
    {
        deadline_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::LatencyBudget& latency_budget() const
    {
        return latency_budget_;
    }

    void latency_budget(const dds::core::policy::LatencyBudget& policy)
    {
        latency_budget_.delegate() = policy;
    }

    void latency_budget(const v_latencyPolicy& policy)
    {
        latency_budget_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Liveliness& liveliness() const
    {
        return liveliness_;
    }

    void liveliness(const dds::core::policy::Liveliness& policy)
    {
        liveliness_.delegate() = policy;
    }

    void liveliness(const v_livelinessPolicy& policy)
    {
        liveliness_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Reliability& reliability() const
    {
        return reliability_;
    }

    void reliability(const dds::core::policy::Reliability& policy)
    {
        reliability_.delegate() = policy;
    }

    void reliability(const v_reliabilityPolicy& policy)
    {
        reliability_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::DestinationOrder& destination_order() const
    {
        return destination_order_;
    }

    void destination_order(const dds::core::policy::DestinationOrder& policy)
    {
        destination_order_.delegate() = policy;
    }

    void destination_order(const v_orderbyPolicy& policy)
    {
        destination_order_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::TimeBasedFilter& time_based_filter() const
    {
        return time_based_filter_;
    }

    void time_based_filter(const dds::core::policy::TimeBasedFilter& policy)
    {
        time_based_filter_.delegate() = policy;
    }

    void time_based_filter(const v_pacingPolicy& policy)
    {
        time_based_filter_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Ownership& ownership() const
    {
        return ownership_;
    }

    void ownership(const dds::core::policy::Ownership& policy)
    {
        ownership_.delegate() = policy;
    }

    void ownership(const v_ownershipPolicy& policy)
    {
        ownership_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::TopicData& topic_data() const
    {
        return topic_data_;
    }

    void topic_data(const dds::core::policy::TopicData& policy)
    {
        topic_data_.delegate() = policy;
    }

    void topic_data(const v_builtinTopicDataPolicy& policy)
    {
        topic_data_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Partition& partition() const
    {
        return partition_;
    }

    void partition(const dds::core::policy::Partition& policy)
    {
        partition_.delegate() = policy;
    }

    void partition(const v_builtinPartitionPolicy& policy)
    {
        partition_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Presentation& presentation() const
    {
        return presentation_;
    }

    void presentation(const dds::core::policy::Presentation& policy)
    {
        presentation_.delegate() = policy;
    }

    void presentation(const v_presentationPolicy& policy)
    {
        presentation_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::UserData& user_data() const
    {
        return user_data_;
    }

    void user_data(const dds::core::policy::UserData& policy)
    {
        user_data_.delegate() = policy;
    }

    void user_data(const v_builtinUserDataPolicy& policy)
    {
        user_data_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::GroupData& group_data() const
    {
        return group_data_;
    }

    void group_data(const dds::core::policy::GroupData& policy)
    {
        group_data_.delegate() = policy;
    }

    void group_data(const v_builtinGroupDataPolicy& policy)
    {
        group_data_.delegate().v_policy(policy);
    }

    bool operator ==(const SubscriptionBuiltinTopicDataDelegate& other) const
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

class org::opensplice::topic::CMParticipantBuiltinTopicDataDelegate
{
public:
    const dds::topic::BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(const dds::topic::BuiltinTopicKey& key)
    {
        key_.delegate() = key;
    }

    void key(const v_builtinTopicKey& key)
    {
        key_.delegate().value(key);
    }

    const ::org::opensplice::core::policy::ProductData& product() const
    {
        return product_;
    }

    void product(const org::opensplice::core::policy::ProductData& policy)
    {
        product_.delegate() = policy;
    }

    void product(const v_productDataPolicy& policy)
    {
        product_.delegate().v_policy(policy);
    }

    bool operator ==(const CMParticipantBuiltinTopicDataDelegate& other) const
    {
        return other.key_ == key_
               && other.product_ == product_;
    }

protected:
    dds::topic::BuiltinTopicKey        key_;
    ::org::opensplice::core::policy::ProductData   product_;
};

//==============================================================================
//            CMPublisherBuiltinTopicDataDelegate
//==============================================================================

class org::opensplice::topic::CMPublisherBuiltinTopicDataDelegate
{
public:
    const dds::topic::BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(const dds::topic::BuiltinTopicKey& key)
    {
        key_.delegate() = key;
    }

    void key(const v_builtinTopicKey& key)
    {
        key_.delegate().value(key);
    }

    const ::org::opensplice::core::policy::ProductData& product() const
    {
        return product_;
    }

    void product(const org::opensplice::core::policy::ProductData& policy)
    {
        product_.delegate() = policy;
    }

    void product(const v_productDataPolicy& policy)
    {
        product_.delegate().v_policy(policy);
    }

    const dds::topic::BuiltinTopicKey& participant_key() const
    {
        return participant_key_;
    }

    void participant_key(const dds::topic::BuiltinTopicKey& key)
    {
        participant_key_.delegate() = key;
    }

    void participant_key(const v_builtinTopicKey& key)
    {
        participant_key_.delegate().value(key);
    }

    const std::string& name() const
    {
        return name_;
    }

    void name(const char *name)
    {
        name_ = name;
    }

    const ::dds::core::policy::EntityFactory& entity_factory() const
    {
        return entity_factory_;
    }

    void entity_factory(const dds::core::policy::EntityFactory& policy)
    {
        entity_factory_.delegate() = policy;
    }

    void entity_factory(const v_entityFactoryPolicy& policy)
    {
        entity_factory_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Partition& partition() const
    {
        return partition_;
    }

    void partition(const dds::core::policy::Partition& policy)
    {
        partition_.delegate() = policy;
    }

    void partition(const v_builtinPartitionPolicy& policy)
    {
        partition_.delegate().v_policy(policy);
    }

    bool operator ==(const CMPublisherBuiltinTopicDataDelegate& other) const
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
    ::org::opensplice::core::policy::ProductData   product_;
    dds::topic::BuiltinTopicKey        participant_key_;
    std::string                        name_;
    ::dds::core::policy::EntityFactory entity_factory_;
    ::dds::core::policy::Partition     partition_;
};

//==============================================================================
//            CMSubscriberBuiltinTopicDataDelegate
//==============================================================================

class org::opensplice::topic::CMSubscriberBuiltinTopicDataDelegate
{
public:
    const dds::topic::BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(const dds::topic::BuiltinTopicKey& key)
    {
        key_.delegate() = key;
    }

    void key(const v_builtinTopicKey& key)
    {
        key_.delegate().value(key);
    }

    const ::org::opensplice::core::policy::ProductData& product() const
    {
        return product_;
    }

    void product(const org::opensplice::core::policy::ProductData& policy)
    {
        product_.delegate() = policy;
    }

    void product(const v_productDataPolicy& policy)
    {
        product_.delegate().v_policy(policy);
    }

    const dds::topic::BuiltinTopicKey& participant_key() const
    {
        return participant_key_;
    }

    void participant_key(const dds::topic::BuiltinTopicKey& key)
    {
        participant_key_.delegate() = key;
    }

    void participant_key(const v_builtinTopicKey& key)
    {
        participant_key_.delegate().value(key);
    }

    const std::string& name() const
    {
        return name_;
    }

    void name(const char *name)
    {
        name_ = name;
    }

    const ::dds::core::policy::EntityFactory& entity_factory() const
    {
        return entity_factory_;
    }

    void entity_factory(const dds::core::policy::EntityFactory& policy)
    {
        entity_factory_.delegate() = policy;
    }

    void entity_factory(const v_entityFactoryPolicy& policy)
    {
        entity_factory_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::Partition& partition() const
    {
        return partition_;
    }

    void partition(const dds::core::policy::Partition& policy)
    {
        partition_.delegate() = policy;
    }

    void partition(const v_builtinPartitionPolicy& policy)
    {
        partition_.delegate().v_policy(policy);
    }

    const ::org::opensplice::core::policy::Share& share() const
    {
        return share_;
    }

    void share(const org::opensplice::core::policy::Share& policy)
    {
        share_.delegate() = policy;
    }

    void share(const v_sharePolicy& policy)
    {
        share_.delegate().v_policy(policy);
    }

    bool operator ==(const CMSubscriberBuiltinTopicDataDelegate& other) const
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
    ::org::opensplice::core::policy::ProductData   product_;
    dds::topic::BuiltinTopicKey        participant_key_;
    std::string                        name_;
    ::dds::core::policy::EntityFactory entity_factory_;
    ::dds::core::policy::Partition     partition_;
    ::org::opensplice::core::policy::Share         share_;
};

//==============================================================================
//            CMDataWriterBuiltinTopicDataDelegate
//==============================================================================

class org::opensplice::topic::CMDataWriterBuiltinTopicDataDelegate
{
public:
    const dds::topic::BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(const dds::topic::BuiltinTopicKey& key)
    {
        key_.delegate() = key;
    }

    void key(const v_builtinTopicKey& key)
    {
        key_.delegate().value(key);
    }

    const ::org::opensplice::core::policy::ProductData& product() const
    {
        return product_;
    }

    void product(const org::opensplice::core::policy::ProductData& policy)
    {
        product_.delegate() = policy;
    }

    void product(const v_productDataPolicy& policy)
    {
        product_.delegate().v_policy(policy);
    }

    const dds::topic::BuiltinTopicKey& publisher_key() const
    {
        return publisher_key_;
    }

    void publisher_key(const dds::topic::BuiltinTopicKey& key)
    {
        publisher_key_.delegate() = key;
    }

    void publisher_key(const v_builtinTopicKey& key)
    {
        publisher_key_.delegate().value(key);
    }

    const std::string& name() const
    {
        return name_;
    }

    void name(const char *name)
    {
        name_ = name;
    }

    const ::dds::core::policy::History& history() const
    {
        return history_;
    }

    void history(const dds::core::policy::History& policy)
    {
        history_.delegate() = policy;
    }

    void history(const v_historyPolicy& policy)
    {
        history_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::ResourceLimits& resource_limits() const
    {
        return resource_limits_;
    }

    void resource_limits(const dds::core::policy::ResourceLimits& policy)
    {
        resource_limits_.delegate() = policy;
    }

    void resource_limits(const v_resourcePolicy& policy)
    {
        resource_limits_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::WriterDataLifecycle& writer_data_lifecycle() const
    {
        return writer_data_lifecycle_;
    }

    void writer_data_lifecycle(const dds::core::policy::WriterDataLifecycle& policy)
    {
        writer_data_lifecycle_.delegate() = policy;
    }

    void writer_data_lifecycle(const v_writerLifecyclePolicy& policy)
    {
        writer_data_lifecycle_.delegate().v_policy(policy);
    }

    bool operator ==(const CMDataWriterBuiltinTopicDataDelegate& other) const
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
    ::org::opensplice::core::policy::ProductData product_;
    dds::topic::BuiltinTopicKey              publisher_key_;
    std::string                              name_;
    ::dds::core::policy::History             history_;
    ::dds::core::policy::ResourceLimits      resource_limits_;
    ::dds::core::policy::WriterDataLifecycle writer_data_lifecycle_;
};

//==============================================================================
//            CMDataReaderBuiltinTopicDataDelegate
//==============================================================================

class org::opensplice::topic::CMDataReaderBuiltinTopicDataDelegate
{
public:
    const dds::topic::BuiltinTopicKey& key() const
    {
        return key_;
    }

    void key(const dds::topic::BuiltinTopicKey& key)
    {
        key_.delegate() = key;
    }

    void key(const v_builtinTopicKey& key)
    {
        key_.delegate().value(key);
    }

    const ::org::opensplice::core::policy::ProductData& product() const
    {
        return product_;
    }

    void product(const org::opensplice::core::policy::ProductData& policy)
    {
        product_.delegate() = policy;
    }

    void product(const v_productDataPolicy& policy)
    {
        product_.delegate().v_policy(policy);
    }

    const dds::topic::BuiltinTopicKey& subscriber_key() const
    {
        return subscriber_key_;
    }

    void subscriber_key(const dds::topic::BuiltinTopicKey& key)
    {
        subscriber_key_.delegate() = key;
    }

    void subscriber_key(const v_builtinTopicKey& key)
    {
        subscriber_key_.delegate().value(key);
    }

    const std::string& name() const
    {
        return name_;
    }

    void name(const char *name)
    {
        name_ = name;
    }

    const ::dds::core::policy::History& history() const
    {
        return history_;
    }

    void history(const dds::core::policy::History& policy)
    {
        history_.delegate() = policy;
    }

    void history(const v_historyPolicy& policy)
    {
        history_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::ResourceLimits& resource_limits() const
    {
        return resource_limits_;
    }

    void resource_limits(const dds::core::policy::ResourceLimits& policy)
    {
        resource_limits_.delegate() = policy;
    }

    void resource_limits(const v_resourcePolicy& policy)
    {
        resource_limits_.delegate().v_policy(policy);
    }

    const ::dds::core::policy::ReaderDataLifecycle& reader_data_lifecycle() const
    {
        return reader_data_lifecycle_;
    }

    void reader_data_lifecycle(const dds::core::policy::ReaderDataLifecycle& policy)
    {
        reader_data_lifecycle_.delegate() = policy;
    }

    void reader_data_lifecycle(const v_readerLifecyclePolicy& policy)
    {
        reader_data_lifecycle_.delegate().v_policy(policy);
    }

    const ::org::opensplice::core::policy::SubscriptionKey& subscription_keys() const
    {
        return subscription_keys_;
    }

    void subscription_keys(const org::opensplice::core::policy::SubscriptionKey& policy)
    {
        subscription_keys_.delegate() = policy;
    }

    void subscription_keys(const v_userKeyPolicy& policy)
    {
        subscription_keys_.delegate().v_policy(policy);
    }

    const ::org::opensplice::core::policy::ReaderLifespan& reader_lifespan() const
    {
        return reader_lifespan_;
    }

    void reader_lifespan(const org::opensplice::core::policy::ReaderLifespan& policy)
    {
        reader_lifespan_.delegate() = policy;
    }

    void reader_lifespan(const v_readerLifespanPolicy& policy)
    {
        reader_lifespan_.delegate().v_policy(policy);
    }

    const ::org::opensplice::core::policy::Share& share() const
    {
        return share_;
    }

    void share(const org::opensplice::core::policy::Share& policy)
    {
        share_.delegate() = policy;
    }

    void share(const v_sharePolicy& policy)
    {
        share_.delegate().v_policy(policy);
    }

    bool operator ==(const CMDataReaderBuiltinTopicDataDelegate& other) const
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
    ::org::opensplice::core::policy::ProductData  product_;
    dds::topic::BuiltinTopicKey              subscriber_key_;
    std::string                              name_;
    ::dds::core::policy::History             history_;
    ::dds::core::policy::ResourceLimits      resource_limits_;
    ::dds::core::policy::ReaderDataLifecycle reader_data_lifecycle_;
    ::org::opensplice::core::policy::SubscriptionKey  subscription_keys_;
    ::org::opensplice::core::policy::ReaderLifespan   reader_lifespan_;
    ::org::opensplice::core::policy::Share   share_;
};

//==============================================================================
//            TypeBuiltinTopicDataDelegate
//==============================================================================

class org::opensplice::topic::TypeBuiltinTopicDataDelegate
{
public:
    const std::string& name() const
    {
        return name_;
    }

    void name(const char *name)
    {
        name_ = name;
    }

    DataRepresentationId_t data_representation_id() const
    {
        return data_representation_id_;
    }

    void data_representation_id(DataRepresentationId_t data_representation_id)
    {
        data_representation_id_ = data_representation_id;
    }

    const TypeHash& type_hash() const
    {
        return type_hash_;
    }

    void type_hash(const TypeHash& type_hash)
    {
        type_hash_ = type_hash;
    }

    const ::dds::core::ByteSeq& meta_data() const
    {
        return meta_data_;
    }

    void meta_data(const ::dds::core::ByteSeq& meta_data)
    {
        meta_data_ = meta_data;
    }

    const ::dds::core::ByteSeq& extentions() const
    {
        return extentions_;
    }

    void extentions(const ::dds::core::ByteSeq& extentions)
    {
        extentions_ = extentions;
    }

    bool operator ==(const TypeBuiltinTopicDataDelegate& other) const
    {
        return other.name_ == name_
               && other.data_representation_id_ == data_representation_id_
               && other.type_hash_ == type_hash_
               && other.meta_data_ == meta_data_
               && other.extentions_ == extentions_;
    }

protected:
    std::string name_;
    DataRepresentationId_t data_representation_id_;
    TypeHash type_hash_;
    ::dds::core::ByteSeq meta_data_;
    ::dds::core::ByteSeq extentions_;
};

//==============================================================================
//            BytesTopicTypeDelegate
//==============================================================================

class org::opensplice::topic::BytesTopicTypeDelegate
{
public:
    BytesTopicTypeDelegate(const std::vector<uint8_t>& value) :
        value_(value)
    {
    }

    const std::vector<uint8_t>& value() const
    {
        return value_;
    }

    void value(const std::vector<uint8_t>& value)
    {
        value_ = value;
    }

    bool operator ==(const BytesTopicTypeDelegate& other) const
    {
        return other.value_ == value_;
    }

protected:
    std::vector<uint8_t> value_;
};

//==============================================================================
//            StringTopicTypeDelegate
//==============================================================================

class org::opensplice::topic::StringTopicTypeDelegate
{
public:
    StringTopicTypeDelegate(const std::string& value) :
        value_(value)
    {
    }

    const std::string& value() const
    {
        return value_;
    }

    void value(const std::string& value)
    {
        value_ = value;
    }

    bool operator ==(const StringTopicTypeDelegate& other) const
    {
        return other.value_ == value_;
    }

protected:
    std::string value_;
};

//==============================================================================
//            KeyedBytesTopicTypeDelegate
//==============================================================================

class org::opensplice::topic::KeyedBytesTopicTypeDelegate
{
public:
    KeyedBytesTopicTypeDelegate(const std::string& key, const std::vector<uint8_t>& value) :
        key_(key), value_(value)
    {
    }

    const std::string& key() const
    {
        return key_;
    }

    void key(const std::string& key)
    {
        key_ = key;
    }

    const std::vector<uint8_t>& value() const
    {
        return value_;
    }

    void value(const std::vector<uint8_t>& value)
    {
        value_ = value;
    }

    bool operator ==(const KeyedBytesTopicTypeDelegate& other) const
    {
        return other.key_   == key_ &&
               other.value_ == value_;
    }

protected:
    std::string key_;
    std::vector<uint8_t> value_;
};

//==============================================================================
//            KeyedStringTopicTypeDelegate
//==============================================================================

class org::opensplice::topic::KeyedStringTopicTypeDelegate
{
public:
    KeyedStringTopicTypeDelegate(const std::string& key, const std::string& value) :
        key_(key), value_(value)
    {
    }

    const std::string& key() const
    {
        return key_;
    }

    void key(const std::string& key)
    {
        key_ = key;
    }

    const std::string& value() const
    {
        return value_;
    }

    void value(const std::string& value)
    {
        value_ = value;
    }

    bool operator ==(const KeyedStringTopicTypeDelegate& other) const
    {
        return other.key_   == key_ &&
               other.value_ == value_;
    }

protected:
    std::string key_;
    std::string value_;
};


#endif /* ORG_OPENSPLICE_TOPIC_BUILTIN_TOPIC_DELEGATE_HPP */
