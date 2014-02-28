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

#include <dds/core/Exception.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <org/opensplice/domain/DomainParticipantDelegate.hpp>
#include <org/opensplice/core/memory.hpp>
#include <org/opensplice/core/exception_helper.hpp>
#include <org/opensplice/domain/qos/QosConverter.hpp>
#include <org/opensplice/topic/qos/QosConverter.hpp>
#include <org/opensplice/pub/qos/QosConverter.hpp>
#include <org/opensplice/sub/qos/QosConverter.hpp>

dds::domain::qos::DomainParticipantQos org::opensplice::domain::DomainParticipantDelegate::default_participant_qos_ = init_default_participant_qos();

org::opensplice::domain::DomainParticipantDelegate::DomainParticipantDelegate(uint32_t id) :
    listener_(0)
{
    this->common_init(id,
                      PARTICIPANT_QOS_DEFAULT,
                      0,
                      DDS::STATUS_MASK_NONE);
    DDS::DomainParticipantQos qos_out;
    org::opensplice::core::check_and_throw(dp_->get_qos(qos_out), OSPL_CONTEXT_LITERAL("Calling dp_->get_qos"));
    qos_ = org::opensplice::domain::qos::convertQos(qos_out);
}

org::opensplice::domain::DomainParticipantDelegate::DomainParticipantDelegate
(uint32_t id,
 const dds::domain::qos::DomainParticipantQos& qos,
 const dds::core::status::StatusMask& event_mask)
    : qos_(qos), listener_(0)
{
    DDS::DomainParticipantQos dQos = org::opensplice::domain::qos::convertQos(qos);
    this->common_init(id,
                      dQos,
                      0,
                      event_mask.to_ulong());
}

void
org::opensplice::domain::DomainParticipantDelegate::common_init(::DDS::DomainId_t domainId,
        const ::DDS::DomainParticipantQos& qos,
        ::DDS::DomainParticipantListener_ptr a_listener,
        ::DDS::StatusMask mask)
{
    DDS::DomainParticipantFactory_var dpf =
        DDS::DomainParticipantFactory::get_instance();

    if(dpf.in() == 0)
    {
        throw dds::core::PreconditionNotMetError(org::opensplice::core::exception_helper(
                    OSPL_CONTEXT_LITERAL(
                        "dds::core::PreconditionNotMetError: Unable to resolve the DomainParticipant Factory.")));
    }
    DDS::DomainParticipant* dp = 0;
    dp = dpf->create_participant(domainId,
                                 qos,
                                 a_listener,
                                 mask);

    if(DDS::is_nil(dp))
        throw dds::core::NullReferenceError(
            org::opensplice::core::exception_helper(
                OSPL_CONTEXT_LITERAL(
                    "dds::core::NullReferenceError : Unable to create DomainParticipant. "
                    "Nil return from ::create_paticipant")));

    dp_.reset(dp, org::opensplice::core::DPDeleter());
    entity_ = DDS::Entity::_narrow(dp);

    DDS::TopicQos top_qos;
    org::opensplice::core::check_and_throw(dp_->get_default_topic_qos(top_qos), OSPL_CONTEXT_LITERAL("Calling ::get_default_topic_qos"));
    this->default_topic_qos_ = org::opensplice::topic::qos::convertQos(top_qos);

    DDS::PublisherQos pub_qos;
    org::opensplice::core::check_and_throw(dp_->get_default_publisher_qos(pub_qos), OSPL_CONTEXT_LITERAL("Calling ::get_default_publisher_qos"));
    this->default_pub_qos_ = org::opensplice::pub::qos::convertQos(pub_qos);

    DDS::SubscriberQos sub_qos;
    org::opensplice::core::check_and_throw(dp_->get_default_subscriber_qos(sub_qos), OSPL_CONTEXT_LITERAL("Calling ::get_default_subscriber_qos"));
    this->default_sub_qos_ = org::opensplice::sub::qos::convertQos(sub_qos);
}


org::opensplice::domain::DomainParticipantDelegate::~DomainParticipantDelegate()
{
    OMG_DDS_LOG("MM", "~DomainParticipantImpl()");
}

uint32_t
org::opensplice::domain::DomainParticipantDelegate::domain_id()
{
    return dp_->get_domain_id();
}

void
org::opensplice::domain::DomainParticipantDelegate::assert_liveliness()
{
    org::opensplice::core::check_and_throw(dp_->assert_liveliness(), OSPL_CONTEXT_LITERAL("Calling ::assert_liveliness()"));
}

bool
org::opensplice::domain::DomainParticipantDelegate::contains_entity(const dds::core::InstanceHandle& handle)
{
    /** @internal @bug OSPL-918 DDS::Boolean is not (yet!) a bool
        @todo Remove fudge when OSPL-918 fixed
        @see http://jira.prismtech.com:8080/browse/OSPL-918 */
    return (dp_->contains_entity(handle->handle()) ? true : false);
}

dds::core::Time
org::opensplice::domain::DomainParticipantDelegate::current_time()
{
    DDS::Time_t now;
    org::opensplice::core::check_and_throw
    (dp_->get_current_time(now), OSPL_CONTEXT_LITERAL("Calling ::get_current_time"));
    return dds::core::Time(now.sec, now.nanosec);
}

const dds::domain::qos::DomainParticipantQos&
org::opensplice::domain::DomainParticipantDelegate::qos() const
{
    return this->qos_;
}


void
org::opensplice::domain::DomainParticipantDelegate::qos(const dds::domain::qos::DomainParticipantQos& qos)
{
    org::opensplice::core::check_and_throw
    (dp_->set_qos(org::opensplice::domain::qos::convertQos(qos)), OSPL_CONTEXT_LITERAL("Calling ::set_qos"));
    qos_ = qos;
}

void
org::opensplice::domain::DomainParticipantDelegate::close()
{
    org::opensplice::core::DPDeleter* d = OSPL_CXX11_STD_MODULE::get_deleter<org::opensplice::core::DPDeleter>(dp_);
    if(d)
    {
        d->close(dp_.get());
    }

}

const dds::topic::qos::TopicQos&
org::opensplice::domain::DomainParticipantDelegate::default_topic_qos() const
{
    return default_topic_qos_;
}

void
org::opensplice::domain::DomainParticipantDelegate::default_topic_qos(const dds::topic::qos::TopicQos& qos)
{
    org::opensplice::core::check_and_throw
    (dp_->set_default_topic_qos(org::opensplice::topic::qos::convertQos(qos)), OSPL_CONTEXT_LITERAL("Calling ::set_default_topic_qos"));
    default_topic_qos_ = qos;
}

const ::dds::pub::qos::PublisherQos&
org::opensplice::domain::DomainParticipantDelegate::default_publisher_qos() const
{
    return default_pub_qos_;
}

void
org::opensplice::domain::DomainParticipantDelegate::default_publisher_qos(const ::dds::pub::qos::PublisherQos& qos)
{
    org::opensplice::core::check_and_throw
    (dp_->set_default_publisher_qos(org::opensplice::pub::qos::convertQos(qos)), OSPL_CONTEXT_LITERAL("Calling ::set_default_publisher_qos"));
    default_pub_qos_ = qos;
}

const ::dds::sub::qos::SubscriberQos&
org::opensplice::domain::DomainParticipantDelegate::default_subscriber_qos() const
{
    return default_sub_qos_;
}

void
org::opensplice::domain::DomainParticipantDelegate::default_subscriber_qos(const ::dds::sub::qos::SubscriberQos& qos)
{
    org::opensplice::core::check_and_throw
    (dp_->set_default_subscriber_qos(org::opensplice::sub::qos::convertQos(qos)), OSPL_CONTEXT_LITERAL("Calling ::set_default_subscriber_qos"));
    default_sub_qos_ = qos;
}

const ::dds::domain::qos::DomainParticipantQos
org::opensplice::domain::DomainParticipantDelegate::init_default_participant_qos()
{
    DDS::DomainParticipantFactory_var dpf =
        DDS::DomainParticipantFactory::get_instance();

    DDS::DomainParticipantQos def_qos;
    org::opensplice::core::check_and_throw
    (dpf->get_default_participant_qos(def_qos), OSPL_CONTEXT_LITERAL("Calling ::get_default_participant_qos"));
    return org::opensplice::domain::qos::convertQos(def_qos);
}

const ::dds::domain::qos::DomainParticipantQos
org::opensplice::domain::DomainParticipantDelegate::default_participant_qos()
{
    return default_participant_qos_;
}

/**
* Sets the default DomainParticipant on the underlying factory.
* @param qos The new default qos
* @note This doesn't bother updating the local default_domain_qos_ member - it would be imediately meaningless.
* @bug OSPL-2456 Make static
*/
void
org::opensplice::domain::DomainParticipantDelegate::default_participant_qos(const ::dds::domain::qos::DomainParticipantQos& qos)
{
    DDS::DomainParticipantFactory_var dpf =
        DDS::DomainParticipantFactory::get_instance();
    DDS::DomainParticipantQos def_qos = org::opensplice::domain::qos::convertQos(qos);
    org::opensplice::core::check_and_throw
    (dpf->set_default_participant_qos(def_qos), OSPL_CONTEXT_LITERAL("Calling ::set_default_participant_qos"));
    default_participant_qos_ = qos;
}

dds::domain::DomainParticipantListener*
org::opensplice::domain::DomainParticipantDelegate::listener() const
{
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : DomainParticipantListener is not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
    return this->listener_;
}

/**
* @internal
* @bug OSPL-1944 Listener not implemented
*/
void
org::opensplice::domain::DomainParticipantDelegate::event_forwarder(dds::domain::DomainParticipantListener* listener,
        const dds::core::smart_ptr_traits<DDS::DomainParticipantListener>::ref_type& forwarder,
        const dds::core::status::StatusMask& event_mask)
{
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : DomainParticipantListener is not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif

    dds::core::smart_ptr_traits<DDS::DomainParticipantListener>::ref_type tmp_fwd;
    if(listener)
    {
        tmp_fwd = forwarder;
    }
    listener_ = listener;
    domain_event_forwarder_.swap(tmp_fwd);
    mask_ = event_mask;
    DDS::ReturnCode_t result = dp_->set_listener(domain_event_forwarder_.get(), event_mask.to_ulong());
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_listener"));
}
