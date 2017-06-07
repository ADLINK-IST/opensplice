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

#ifndef ORG_OPENSPLICE_DOMAIN_DOMAINPARTICIPANT_EVENT_HANDLER_HPP_
#define ORG_OPENSPLICE_DOMAIN_DOMAINPARTICIPANT_EVENT_HANDLER_HPP_

#include <dds/core/WeakReference.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace domain
{

template<typename DOMAINT>
class OSPL_ISOCPP_IMPL_API DomainParticipantEventForwarder: public DDS::DomainParticipantListener
{
public:
    DomainParticipantEventForwarder(
        const DOMAINT &domain_,
        dds::domain::DomainParticipantListener* listener);

    virtual ~DomainParticipantEventForwarder();

    virtual dds::domain::DomainParticipantListener* listener();

    //SUB
    virtual void on_data_on_readers(DDS::Subscriber_ptr subscriber) {};
    //PUB
    //TOPIC
    virtual void on_inconsistent_topic(DDS::Topic_ptr topic, const DDS::InconsistentTopicStatus& status) {};
    //ADW
    virtual void on_offered_deadline_missed(DDS::DataWriter_ptr writer, const DDS::OfferedDeadlineMissedStatus& status) {};
    virtual void on_offered_incompatible_qos(DDS::DataWriter_ptr writer, const DDS::OfferedIncompatibleQosStatus& status) {};
    virtual void on_liveliness_lost(DDS::DataWriter_ptr writer, const DDS::LivelinessLostStatus& status) {};
    virtual void on_publication_matched(DDS::DataWriter_ptr writer, const DDS::PublicationMatchedStatus& status) {};

    //ADR
    virtual void on_requested_deadline_missed(DDS::DataReader_ptr reader, const DDS::RequestedDeadlineMissedStatus& status) {};
    virtual void on_requested_incompatible_qos(DDS::DataReader_ptr reader, const DDS::RequestedIncompatibleQosStatus& status) {};
    virtual void on_sample_rejected(DDS::DataReader_ptr reader, const DDS::SampleRejectedStatus& status) {};
    virtual void on_liveliness_changed(DDS::DataReader_ptr reader, const DDS::LivelinessChangedStatus& status) {};
    virtual void on_data_available(DDS::DataReader_ptr reader) {};
    virtual void on_subscription_matched(DDS::DataReader_ptr reader, const DDS::SubscriptionMatchedStatus& status) {};
    virtual void on_sample_lost(DDS::DataReader_ptr reader, const DDS::SampleLostStatus& status) {};

    dds::core::WeakReference<DOMAINT> domain_;
    dds::domain::DomainParticipantListener *listener_;
};
}
}
}
#endif /* ORG_OPENSPLICE_PUB_PUBLISHER_EVENT_HANDLER_HPP_ */
