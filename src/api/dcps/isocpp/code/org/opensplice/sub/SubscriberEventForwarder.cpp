/*
*                         Vortex OpenSplice
*
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
    dds::sub::SubscriberListener *listener) :
    listener_(listener)
{
    sub_ = dds::core::WeakReference<dds::sub::Subscriber>(sub);
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
    dds::sub::Subscriber s = sub_.lock();
    listener_->on_data_on_readers(s);
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
