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

#include <org/opensplice/sub/SubscriberDelegate.hpp>
#include <org/opensplice/core/memory.hpp>
#include <org/opensplice/sub/qos/QosConverter.hpp>
#include <org/opensplice/core/exception_helper.hpp>


namespace org
{
namespace opensplice
{
namespace sub
{

SubscriberDelegate::SubscriberDelegate(
    const dds::domain::DomainParticipant& dp,
    const dds::sub::qos::SubscriberQos& qos,
    const dds::core::status::StatusMask& event_mask) :
    dp_(dp),
    qos_(qos),
    listener_(0),
    mask_(event_mask),
    sub_(),
    sub_event_forwarder_()
{
    DDS::SubscriberQos sqos = org::opensplice::sub::qos::convertQos(qos);

    DDS::Subscriber_ptr s = dp->dp_->create_subscriber(sqos, 0,
                            event_mask.to_ulong());

    if(s == 0) throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                    OSPL_CONTEXT_LITERAL(
                        "dds::core::NullReferenceError : Unable to create Subscriber. "
                        "Nil return from ::create_subscriber")));

    sub_.reset(s, ::org::opensplice::core::SubDeleter(dp_->dp_));

    DDS::DataReaderQos oldqos;
    DDS::ReturnCode_t result = sub_->get_default_datareader_qos(oldqos);
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_default_datareader_qos"));

    default_dr_qos_ = org::opensplice::sub::qos::convertQos(oldqos);

    entity_ = DDS::Entity::_narrow(s);
}

void SubscriberDelegate::init_builtin(DDS::Subscriber_ptr ddssub)
{
    if(ddssub == 0) throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                    OSPL_CONTEXT_LITERAL(
                        "dds::core::NullReferenceError : Unable to get builtin Subscriber. "
                        "Nil return from ::get_builtin_subscriber")));

    DDS::SubscriberQos qos;
    DDS::ReturnCode_t result = ddssub->get_qos(qos);
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_qos"));
    qos_ = org::opensplice::sub::qos::convertQos(qos);

    sub_.reset(ddssub, ::org::opensplice::core::SubDeleter(dp_->dp_));

    DDS::DataReaderQos oldqos;
    result = sub_->get_default_datareader_qos(oldqos);
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_default_datareader_qos"));

    default_dr_qos_ = org::opensplice::sub::qos::convertQos(oldqos);
    entity_ = DDS::Entity::_narrow(ddssub);

    org::opensplice::core::SubDeleter* d = OSPL_CXX11_STD_MODULE::get_deleter<org::opensplice::core::SubDeleter>(sub_);
    if(d)
    {
        d->set_builtin();
    }
}

bool SubscriberDelegate::is_builtin() const
{
    org::opensplice::core::SubDeleter* d = OSPL_CXX11_STD_MODULE::get_deleter<org::opensplice::core::SubDeleter>(sub_);
    if(d)
    {
        return d->is_builtin();
    }

    return false;
}

SubscriberDelegate::~SubscriberDelegate()
{
    if(listener_ != 0)
    {
        DDS::ReturnCode_t result = sub_->set_listener(0, DDS::STATUS_MASK_NONE);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_listener(nil)"));
    }
}

void SubscriberDelegate::notify_datareaders()
{
    DDS::ReturnCode_t result = sub_->notify_datareaders();
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::notify_datareaders"));
}

const dds::domain::DomainParticipant&
SubscriberDelegate::participant() const
{
    return dp_;
}

const dds::sub::qos::SubscriberQos& SubscriberDelegate::qos() const
{
    return qos_;
}

void SubscriberDelegate::qos(const dds::sub::qos::SubscriberQos& sqos)
{
    DDS::ReturnCode_t result = sub_->set_qos(org::opensplice::sub::qos::convertQos(sqos));
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_qos"));
    qos_ = sqos;
}

dds::sub::qos::DataReaderQos
SubscriberDelegate::default_datareader_qos()
{
    DDS::DataReaderQos oldqos;

    DDS::ReturnCode_t result = sub_->get_default_datareader_qos(oldqos);
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_default_datareader_qos"));

    dds::sub::qos::DataReaderQos newqos = org::opensplice::sub::qos::convertQos(oldqos);
    default_dr_qos_ = newqos;
    return default_dr_qos_;
}

void
SubscriberDelegate::default_datareader_qos(const dds::sub::qos::DataReaderQos& qos)
{
    DDS::ReturnCode_t result = sub_->set_default_datareader_qos(convertQos(qos));
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_default_datareader_qos"));

    default_dr_qos_ = qos;
}

dds::sub::SubscriberListener*
SubscriberDelegate::listener() const
{
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : SubscriberListener is not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
    return this->listener_;
}

void SubscriberDelegate::event_forwarder(
    dds::sub::SubscriberListener* listener,
    const dds::core::smart_ptr_traits<DDS::SubscriberListener>::ref_type& forwarder,
    const dds::core::status::StatusMask& event_mask)
{
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                          OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : SubscriberListener is not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif
    dds::core::smart_ptr_traits<DDS::SubscriberListener>::ref_type tmp_fwd;
    if(listener)
    {
        tmp_fwd = forwarder;
    }
    listener_ = listener;
    sub_event_forwarder_.swap(tmp_fwd);
    mask_ = event_mask;
    DDS::ReturnCode_t result = sub_->set_listener(sub_event_forwarder_.get(), event_mask.to_ulong());
    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_listener"));
}

void SubscriberDelegate::close()
{
    org::opensplice::core::SubDeleter* d = OSPL_CXX11_STD_MODULE::get_deleter<org::opensplice::core::SubDeleter>(sub_);
    if(d)
    {
        d->close(sub_.get());
    }
}

}
}
}
