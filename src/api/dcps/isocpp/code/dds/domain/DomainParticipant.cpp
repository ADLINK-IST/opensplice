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


#include <dds/domain/DomainParticipant.hpp>



#if 0

namespace dds
{
namespace domain
{
template <>
dds::pub::Publisher
DomainParticipant<org::opensplice::domain::DomainParticipantImpl>::create_publisher(
    const dds::pub::qos::PublisherQos& pub_qos,
    dds::pub::PublisherListener* pub_listener,
    const dds::core::status::StatusMask& event_mask)
{
    return dds::pub::Publisher(
               new org::opensplice::pub::PublisherImpl(
                   dds::core::smart_ptr_traits<dds::domain::detail::DomainParticipantHolder>::ref_type(
                       new dds::domain::detail::DomainParticipantHolder(*this)),
                   pub_qos,
                   pub_listener,
                   event_mask));
}


template <>
const dds::sub::qos::SubscriberQos
DomainParticipant<org::opensplice::domain::DomainParticipantImpl>::default_subscriber_qos() const
{
    return dds::sub::qos::SubscriberQos();
}


template <>
dds::sub::Subscriber
DomainParticipant<org::opensplice::domain::DomainParticipantImpl>::create_subscriber(
    const dds::sub::qos::SubscriberQos& sub_qos,
    dds::sub::SubscriberListener* sub_listener,
    const dds::core::status::StatusMask& event_mask)
{
    return dds::sub::Subscriber(
               new org::opensplice::sub::SubscriberImpl(
                   dds::core::smart_ptr_traits<dds::domain::detail::DomainParticipantHolder>::ref_type(
                       new dds::domain::detail::DomainParticipantHolder(*this)),
                   sub_qos,
                   sub_listener,
                   event_mask));
}


template <>
dds::sub::Subscriber
DomainParticipant<org::opensplice::domain::DomainParticipantImpl>::create_subscriber()
{
    return create_subscriber(default_subscriber_qos(),
                             NULL,
                             dds::core::status::StatusMask::none());
}


template <>
const dds::topic::qos::TopicQos
DomainParticipant<org::opensplice::domain::DomainParticipantImpl>::default_topic_qos() const
{
    return dds::topic::qos::TopicQos();
}

}
}

#endif
