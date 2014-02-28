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


/** @internal @todo remove includes **/
//#include <dds/pub/Publisher.hpp>
// Include DomainParticipantHolder so that auto_ptr can see its destructor:
//#include <dds/core/ref_traits.hpp>

#include <org/opensplice/pub/PublisherDelegate.hpp>
#include <org/opensplice/core/memory.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <org/opensplice/pub/qos/QosConverter.hpp>
#include <org/opensplice/core/exception_helper.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{

PublisherDelegate::PublisherDelegate(const dds::domain::DomainParticipant& dp,
                                     const dds::pub::qos::PublisherQos& qos,
                                     const dds::core::status::StatusMask& event_mask)
    :   dp_(dp),
        qos_(qos),
        listener_(0),
        mask_(event_mask),
        default_dwqos_(),
        pub_(),
        pub_event_forwarder_()
{
    DDS::PublisherQos pqos = convertQos(qos);

    DDS::Publisher_ptr p =
        dp->dp_->create_publisher(pqos, 0, event_mask.to_ulong());

    if(p == 0)
        throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                                                OSPL_CONTEXT_LITERAL(
                                                        "dds::core::NullReferenceError : Unable to create Publisher. "
                                                        "Nil return from ::create_publisher")));

    pub_.reset(p, ::org::opensplice::core::PubDeleter(dp_->dp_));

    entity_ = DDS::Entity::_narrow(p);
}

/*
    template <typename T>
    std::vector<dds::pub::DataWriter<T> >
    lookup_datawriter(const std::string& topic_name);
*/

PublisherDelegate::~PublisherDelegate()
{
    if(listener_ != 0)
    {
        DDS::ReturnCode_t result = pub_->set_listener(0, DDS::STATUS_MASK_NONE);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_listener(nil)"));
    }
}

const dds::pub::qos::PublisherQos&
PublisherDelegate::qos() const
{
    return qos_;
}

void PublisherDelegate::qos(const dds::pub::qos::PublisherQos& pqos)
{
    DDS::ReturnCode_t result = pub_->set_qos(org::opensplice::pub::qos::convertQos(pqos));
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_qos"));

    qos_ = pqos;
}

void
PublisherDelegate::wait_for_acknowledgments(const dds::core::Duration& max_wait)
{
    DDS::Duration_t ddsTimeout;
    ddsTimeout.sec =  static_cast<int32_t>(max_wait.sec());
    ddsTimeout.nanosec = max_wait.nanosec();

    DDS::ReturnCode_t result = pub_->wait_for_acknowledgments(ddsTimeout);
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::wait_for_acknowledgments"));
}


const dds::domain::DomainParticipant&
PublisherDelegate::participant() const
{
    return dp_;
}

bool
PublisherDelegate::suspend_publications()
{
    DDS::ReturnCode_t result = pub_->suspend_publications();
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::suspend_publications"));

    return true;
}

bool
PublisherDelegate::resume_publications()
{
    DDS::ReturnCode_t result = pub_->resume_publications();
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::resume_publications"));

    return true;
}

void
PublisherDelegate::begin_coherent_changes()
{
    DDS::ReturnCode_t result = pub_->begin_coherent_changes();
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::begin_coherent_changes"));
}

void
PublisherDelegate::end_coherent_changes()
{
    DDS::ReturnCode_t result = pub_->end_coherent_changes();
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::end_coherent_changes"));
}

void
PublisherDelegate::close()
{
    org::opensplice::core::PubDeleter* d = OSPL_CXX11_STD_MODULE::get_deleter<org::opensplice::core::PubDeleter>(pub_);
    if(d)
    {
        d->close(pub_.get());
    }
}

void
PublisherDelegate::retain()
{

}
void
PublisherDelegate::default_datawriter_qos(const dds::pub::qos::DataWriterQos& dwqos)
{
    DDS::ReturnCode_t result = pub_->set_default_datawriter_qos(convertQos(dwqos));
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::end_coherent_changes"));

    default_dwqos_ = dwqos;
}

dds::pub::qos::DataWriterQos
PublisherDelegate::default_datawriter_qos()
{
    DDS::DataWriterQos oldqos;

    DDS::ReturnCode_t result = pub_->get_default_datawriter_qos(oldqos);
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_default_datawriter_qos"));

    dds::pub::qos::DataWriterQos newqos = org::opensplice::pub::qos::convertQos(oldqos);
    default_dwqos_ = newqos;
    return default_dwqos_;
}

dds::pub::PublisherListener* PublisherDelegate::listener() const
{
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : PublisherListener is not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
    return this->listener_;
}

/** @internal @todo OSPL-1942 implement listeners */
void PublisherDelegate::event_forwarder(dds::pub::PublisherListener* listener,
                                        const dds::core::smart_ptr_traits<DDS::PublisherListener>::ref_type& forwarder,
                                        const dds::core::status::StatusMask& event_mask)
{
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : PublisherListener is not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
    dds::core::smart_ptr_traits<DDS::PublisherListener>::ref_type tmp_fwd;
    if(listener)
    {
        tmp_fwd = forwarder;
    }
    listener_ = listener;
    pub_event_forwarder_.swap(tmp_fwd);
    mask_ = event_mask;
    DDS::ReturnCode_t result = pub_->set_listener(pub_event_forwarder_.get(), event_mask.to_ulong());
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_listener"));
}

}
}
}
