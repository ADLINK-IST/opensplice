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

#ifndef ORG_OPENSPLICE_TOPIC_TBUILTIN_TOPIC_HPP_
#define ORG_OPENSPLICE_TOPIC_TBUILTIN_TOPIC_HPP_


#include <dds/core/detail/conformance.hpp>
#include <dds/core/Value.hpp>
#include <dds/core/policy/CorePolicy.hpp>
#include <dds/topic/BuiltinTopicKey.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{
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

template <typename D>
class TTypeBuiltinTopicData;
}
}
}


/**
 * The CMParticipant topic...
 */
template <typename D>
class org::opensplice::topic::TCMParticipantBuiltinTopicData  : public ::dds::core::Value<D>
{
public:
    const dds::topic::BuiltinTopicKey&        key() const
    {
        return this->delegate().key();
    }
    const ::org::opensplice::core::policy::ProductData&   product() const
    {
        return this->delegate().product();
    }
};

/**
 * The CMPublisher topic...
 */
template <typename D>
class org::opensplice::topic::TCMPublisherBuiltinTopicData  : public ::dds::core::Value<D>
{
public:
    const dds::topic::BuiltinTopicKey&        key() const
    {
        return this->delegate().key();
    }
    const ::org::opensplice::core::policy::ProductData&   product() const
    {
        return this->delegate().product();
    }
    const dds::topic::BuiltinTopicKey&        participant_key() const
    {
        return this->delegate().participant_key();
    }
    const std::string&                        name() const
    {
        return this->delegate().name();
    }
    const ::dds::core::policy::EntityFactory& entity_factory() const
    {
        return this->delegate().entity_factory();
    }
    const ::dds::core::policy::Partition&     partition() const
    {
        return this->delegate().partition();
    }
};

/**
 * The CMSubscriber topic...
 */
template <typename D>
class org::opensplice::topic::TCMSubscriberBuiltinTopicData  : public ::dds::core::Value<D>
{
public:
    const dds::topic::BuiltinTopicKey&        key() const
    {
        return this->delegate().key();
    }
    const ::org::opensplice::core::policy::ProductData&   product() const
    {
        return this->delegate().product();
    }
    const dds::topic::BuiltinTopicKey&        participant_key() const
    {
        return this->delegate().participant_key();
    }
    const std::string&                        name() const
    {
        return this->delegate().name();
    }
    const ::dds::core::policy::EntityFactory& entity_factory() const
    {
        return this->delegate().entity_factory();
    }
    const ::dds::core::policy::Partition&     partition() const
    {
        return this->delegate().partition();
    }
    const ::org::opensplice::core::policy::Share&         share() const
    {
        return this->delegate().share();
    }
};

/**
 * The CMDataWriter topic...
 */
template <typename D>
class org::opensplice::topic::TCMDataWriterBuiltinTopicData  : public ::dds::core::Value<D>
{
public:
    const dds::topic::BuiltinTopicKey&              key() const
    {
        return this->delegate().key();
    }
    const ::org::opensplice::core::policy::ProductData&         product() const
    {
        return this->delegate().product();
    }
    const dds::topic::BuiltinTopicKey&              publisher_key() const
    {
        return this->delegate().publisher_key();
    }
    const std::string&                              name() const
    {
        return this->delegate().name();
    }
    const ::dds::core::policy::History&             history() const
    {
        return this->delegate().history();
    }
    const ::dds::core::policy::ResourceLimits&      resource_limits() const
    {
        return this->delegate().resource_limits();
    }
    const ::dds::core::policy::WriterDataLifecycle& writer_data_lifecycle() const
    {
        return this->delegate().writer_data_lifecycle();
    }
};

/**
 * The CMDataReader topic...
 */
template <typename D>
class org::opensplice::topic::TCMDataReaderBuiltinTopicData  : public ::dds::core::Value<D>
{
public:
    const dds::topic::BuiltinTopicKey&              key() const
    {
        return this->delegate().key();
    }
    const ::org::opensplice::core::policy::ProductData&         product() const
    {
        return this->delegate().product();
    }
    const dds::topic::BuiltinTopicKey&              subscriber_key() const
    {
        return this->delegate().subscriber_key();
    }
    const std::string&                              name() const
    {
        return this->delegate().name();
    }
    const ::dds::core::policy::History&             history() const
    {
        return this->delegate().history();
    }
    const ::dds::core::policy::ResourceLimits&      resource_limits() const
    {
        return this->delegate().resource_limits();
    }
    const ::dds::core::policy::ReaderDataLifecycle& reader_data_lifecycle() const
    {
        return this->delegate().reader_data_lifecycle();
    }
    const ::org::opensplice::core::policy::SubscriptionKey&     subscription_keys() const
    {
        return this->delegate().subscription_keys();
    }
    const ::org::opensplice::core::policy::ReaderLifespan&      reader_lifespan() const
    {
        return this->delegate().reader_lifespan();
    }
    const ::org::opensplice::core::policy::Share&               share() const
    {
        return this->delegate().share();
    }
};

/**
 * The Type topic...
 */
template <typename D>
class org::opensplice::topic::TTypeBuiltinTopicData  : public ::dds::core::Value<D>
{
public:
    const std::string& name() const
    {
        return this->delegate().name();
    }

    DataRepresentationId_t data_representation_id() const
    {
        return this->delegate().data_representation_id();
    }

    const TypeHash& type_hash() const
    {
        return this->delegate().type_hash();
    }

    const ::dds::core::ByteSeq& meta_data() const
    {
        return this->delegate().meta_data();
    }

    const ::dds::core::ByteSeq& extentions() const
    {
        return this->delegate().extentions();
    }
};


#endif /* ORG_OPENSPLICE_TOPIC_TBUILTIN_TOPIC_HPP_ */
