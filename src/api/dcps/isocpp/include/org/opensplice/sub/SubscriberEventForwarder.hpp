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

#ifndef ORG_OPENSPLICE_SUB_SUBSCRIBER_EVENT_HANDLER_HPP_
#define ORG_OPENSPLICE_SUB_SUBSCRIBER_EVENT_HANDLER_HPP_

#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{

template<typename SUBT>
class OSPL_ISOCPP_IMPL_API SubscriberEventForwarder: public DDS::SubscriberListener
{
public:
    SubscriberEventForwarder(
        const SUBT& sub_,
        dds::sub::SubscriberListener* listener);

    virtual ~SubscriberEventForwarder();


    virtual dds::sub::SubscriberListener* listener();

    virtual void on_data_on_readers(DDS::Subscriber_ptr subscriber);

    //ADR
    virtual void on_requested_deadline_missed(DDS::DataReader_ptr reader, const DDS::RequestedDeadlineMissedStatus& status) {};
    virtual void on_requested_incompatible_qos(DDS::DataReader_ptr reader, const DDS::RequestedIncompatibleQosStatus& status) {};
    virtual void on_sample_rejected(DDS::DataReader_ptr reader, const DDS::SampleRejectedStatus& status) {};
    virtual void on_liveliness_changed(DDS::DataReader_ptr reader, const DDS::LivelinessChangedStatus& status) {};
    virtual void on_data_available(DDS::DataReader_ptr reader) {};
    virtual void on_subscription_matched(DDS::DataReader_ptr reader, const DDS::SubscriptionMatchedStatus& status) {};
    virtual void on_sample_lost(DDS::DataReader_ptr reader, const DDS::SampleLostStatus& status) {};

    SUBT sub_;
    dds::sub::SubscriberListener* listener_;
};
}
}
}
#endif /* ORG_OPENSPLICE_SUB_SUBSCRIBER_EVENT_HANDLER_HPP_ */
