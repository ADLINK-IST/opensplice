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
