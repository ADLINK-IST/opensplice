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

#include <dds/sub/SubscriberListener.hpp>
#include <org/opensplice/sub/SubscriberEventForwarder.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{

template<>
SubscriberEventForwarder<dds::sub::Subscriber>::SubscriberEventForwarder(
    const dds::sub::Subscriber& sub,
    dds::sub::SubscriberListener* listener) :
    sub_(sub),
    listener_(listener)

{
}

template<>
SubscriberEventForwarder<dds::sub::Subscriber>::~SubscriberEventForwarder()
{
}

template<>
dds::sub::SubscriberListener*
SubscriberEventForwarder<dds::sub::Subscriber>::listener()
{
    return listener_;
}

template<>
void
SubscriberEventForwarder<dds::sub::Subscriber>::on_data_on_readers(DDS::Subscriber_ptr subscriber)
{
    OMG_DDS_LOG("EVT", "on_data_available");
    listener_->on_data_on_readers(sub_);
}
/*
template<>
void SubscriberEventForwarder<dds::sub::Subscriber>::on_requested_deadline_missed (DDS::DataReader_ptr reader, const DDS::RequestedDeadlineMissedStatus& status)
{

    status.transform();
    listener_->on_requested_deadline_missed(dds::sub::AnyDataReader, dds:cond::RequestedDeadlineMissedStatus)
}
*/
}
}
}
