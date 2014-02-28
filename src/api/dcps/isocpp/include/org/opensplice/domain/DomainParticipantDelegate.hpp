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

#ifndef ORG_OPENSPLICE_DOMAIN_DOMAIN_PARTICIPANT_DELEGATE_HPP_
#define ORG_OPENSPLICE_DOMAIN_DOMAIN_PARTICIPANT_DELEGATE_HPP_


// DDS-PSM-Cxx Includes
#include <dds/core/ref_traits.hpp>
#include <dds/core/Time.hpp>
#include <dds/core/InstanceHandle.hpp>
#include <dds/core/status/State.hpp>
#include <dds/topic/qos/TopicQos.hpp>

// OpenSplice Includes
#include <org/opensplice/core/config.hpp>
#include <org/opensplice/domain/Domain.hpp>
#include <org/opensplice/core/EntityDelegate.hpp>


namespace dds
{
namespace domain
{
class DomainParticipantListener;
}
}

namespace org
{
namespace opensplice
{
namespace domain
{
class DomainParticipantDelegate;
}
}
}

class OSPL_ISOCPP_IMPL_API org::opensplice::domain::DomainParticipantDelegate :
    public virtual  ::org::opensplice::core::EntityDelegate
{
public:
    explicit DomainParticipantDelegate(uint32_t id);

    DomainParticipantDelegate(uint32_t id,
                              const dds::domain::qos::DomainParticipantQos& qos,
                              const dds::core::status::StatusMask&);

    virtual ~DomainParticipantDelegate();

public:
    dds::domain::DomainParticipantListener* listener() const;

public:
    const dds::domain::qos::DomainParticipantQos& qos() const;

    void qos(const dds::domain::qos::DomainParticipantQos& qos);

    uint32_t domain_id();

    void assert_liveliness();

    bool contains_entity(const ::dds::core::InstanceHandle& handle);

    dds::core::Time current_time();

    void close();

    const dds::topic::qos::TopicQos& default_topic_qos() const;
    void default_topic_qos(const dds::topic::qos::TopicQos& qos);

    const ::dds::pub::qos::PublisherQos& default_publisher_qos() const;
    void default_publisher_qos(const ::dds::pub::qos::PublisherQos& qos);

    const ::dds::sub::qos::SubscriberQos& default_subscriber_qos() const;
    void default_subscriber_qos(const ::dds::sub::qos::SubscriberQos& qos);

    static const dds::domain::qos::DomainParticipantQos init_default_participant_qos();
    static const ::dds::domain::qos::DomainParticipantQos default_participant_qos();
    static void default_participant_qos(const ::dds::domain::qos::DomainParticipantQos& qos);
    void event_forwarder(dds::domain::DomainParticipantListener* listener,
                         const dds::core::smart_ptr_traits<DDS::DomainParticipantListener>::ref_type& forwarder,
                         const dds::core::status::StatusMask& event_mask);

protected:
    static dds::domain::qos::DomainParticipantQos default_participant_qos_;

private:
    /**
    *  @internal Common initialisation routine for use by constructors.
    * Creates the particiapnt and initialises cached default QoS
    * copies.
    */
    void common_init(::DDS::DomainId_t domainId,
                     const ::DDS::DomainParticipantQos& qos,
                     ::DDS::DomainParticipantListener_ptr a_listener,
                     ::DDS::StatusMask mask);

    dds::domain::qos::DomainParticipantQos qos_;
    dds::topic::qos::TopicQos default_topic_qos_;
    dds::pub::qos::PublisherQos default_pub_qos_;
    dds::sub::qos::SubscriberQos default_sub_qos_;
    dds::domain::DomainParticipantListener* listener_;
    dds::core::status::StatusMask mask_;

public:
    dds::core::smart_ptr_traits<DDS::DomainParticipant>::ref_type dp_;
    dds::core::smart_ptr_traits<DDS::DomainParticipantListener>::ref_type domain_event_forwarder_;

};

#endif /* ORG_OPENSPLICE_DOMAIN_DOMAIN_PARTICIPANT_DELEGATE_HPP_ */
