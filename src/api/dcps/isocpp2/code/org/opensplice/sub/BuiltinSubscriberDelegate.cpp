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


/**
 * @file
 */

#include <dds/sub/DataReader.hpp>
#include <dds/sub/Subscriber.hpp>
#include <dds/topic/BuiltinTopic.hpp>
#include <dds/topic/detail/find.hpp>
#include <dds/topic/detail/discovery.hpp>
#include <dds/sub/detail/TDataReaderImpl.hpp>

#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/sub/BuiltinSubscriberDelegate.hpp>
#include <org/opensplice/core/ScopedLock.hpp>

#include <org/opensplice/topic/BuiltinTopicTraits.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{
template <typename T>
dds::sub::DataReader<T>
create_builtin_reader(
    SubscriberDelegate& subscriber,
    const std::string& topic_name);

}
}
}


org::opensplice::core::Mutex
org::opensplice::sub::BuiltinSubscriberDelegate::builtinLock;

org::opensplice::sub::BuiltinSubscriberDelegate::BuiltinSubscriberDelegate(
    const dds::domain::DomainParticipant& dp,
    const dds::sub::qos::SubscriberQos& qos) :
        SubscriberDelegate(dp, qos, NULL, dds::core::status::StatusMask::none())
{

}


std::vector<org::opensplice::sub::AnyDataReaderDelegate::ref_type>
org::opensplice::sub::BuiltinSubscriberDelegate::find_datareaders(
    const std::string& topic_name)
{
    std::vector<org::opensplice::sub::AnyDataReaderDelegate::ref_type> list;

    list = SubscriberDelegate::find_datareaders(topic_name);
    if (list.size() == 0) {
        org::opensplice::sub::AnyDataReaderDelegate::ref_type reader = get_builtin_reader(*this, topic_name);
        if (reader) {
            list.push_back(reader);
        }
    }

    return list;
}

org::opensplice::sub::SubscriberDelegate::ref_type
org::opensplice::sub::BuiltinSubscriberDelegate::get_builtin_subscriber(
        const dds::domain::DomainParticipant& dp)
{
    org::opensplice::core::ScopedMutexLock scopedLock(builtinLock);

    SubscriberDelegate::ref_type builtin_subscriber;

    org::opensplice::core::EntityDelegate::ref_type entity = dp.delegate()->builtin_subscriber();
    if (entity) {
        builtin_subscriber = OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<SubscriberDelegate>(entity);
    } else {
        dds::sub::qos::SubscriberQos qos;

        qos << dds::core::policy::PresentationAccessScopeKind::TOPIC;
        qos << dds::core::policy::Partition("__BUILT-IN PARTITION__");

        builtin_subscriber.reset(new org::opensplice::sub::BuiltinSubscriberDelegate(dp, qos));
        builtin_subscriber->init(builtin_subscriber);
        dp.delegate()->builtin_subscriber(builtin_subscriber);
    }

    return builtin_subscriber;
}


template <typename T>
dds::sub::DataReader<T>
org::opensplice::sub::create_builtin_reader(
    SubscriberDelegate& subscriber,
    const std::string& topic_name)
{
    dds::sub::qos::DataReaderQos rQos;

    dds::topic::Topic<T> topic =
            dds::topic::find< dds::topic::Topic<T> >(subscriber.participant(), topic_name);
    if (topic.is_nil()) {
        topic = dds::topic::discover< dds::topic::Topic<T> >(subscriber.participant(), topic_name, dds::core::Duration::zero());
        if (topic.is_nil()) {
            ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not find builtin topic \"%s\"", topic_name.c_str());
        }
    }
    subscriber.default_datareader_qos(rQos);
    rQos = topic.qos();
    dds::sub::DataReader<T> reader(subscriber.wrapper(), topic, rQos);

    return reader;
}

org::opensplice::sub::AnyDataReaderDelegate::ref_type
org::opensplice::sub::BuiltinSubscriberDelegate::get_builtin_reader(
    SubscriberDelegate& subscriber,
    const std::string& topic_name)
{
    org::opensplice::core::ScopedMutexLock scopedLock(builtinLock);

    org::opensplice::sub::AnyDataReaderDelegate::ref_type builtin_reader;

    if (topic_name == "DCPSParticipant") {
        builtin_reader = create_builtin_reader<dds::topic::ParticipantBuiltinTopicData>(subscriber, topic_name).delegate();
    } else if (topic_name == "DCPSTopic") {
        builtin_reader = create_builtin_reader<dds::topic::TopicBuiltinTopicData>(subscriber, topic_name).delegate();
    } else if (topic_name == "DCPSPublication") {
        builtin_reader = create_builtin_reader<dds::topic::PublicationBuiltinTopicData>(subscriber, topic_name).delegate();
    } else if (topic_name == "DCPSSubscription") {
        builtin_reader = create_builtin_reader<dds::topic::SubscriptionBuiltinTopicData>(subscriber, topic_name).delegate();
    } else if (topic_name == "CMParticipant") {
        builtin_reader = create_builtin_reader<org::opensplice::topic::CMParticipantBuiltinTopicData>(subscriber, topic_name).delegate();
    } else if (topic_name == "CMPublisher") {
        builtin_reader = create_builtin_reader<org::opensplice::topic::CMPublisherBuiltinTopicData>(subscriber, topic_name).delegate();
    } else if (topic_name == "CMSubscriber") {
        builtin_reader = create_builtin_reader<org::opensplice::topic::CMSubscriberBuiltinTopicData>(subscriber, topic_name).delegate();
    } else if (topic_name == "CMDataWriter") {
        builtin_reader = create_builtin_reader<org::opensplice::topic::CMDataWriterBuiltinTopicData>(subscriber, topic_name).delegate();
    } else if (topic_name == "CMDataReader") {
        builtin_reader = create_builtin_reader<org::opensplice::topic::CMDataReaderBuiltinTopicData>(subscriber, topic_name).delegate();
    } else if (topic_name == "DCPSType") {
        builtin_reader = create_builtin_reader<org::opensplice::topic::TypeBuiltinTopicData>(subscriber, topic_name).delegate();
    }

    return builtin_reader;
}
