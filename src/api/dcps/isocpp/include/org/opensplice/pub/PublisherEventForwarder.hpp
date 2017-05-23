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

#ifndef ORG_OPENSPLICE_PUB_PUBLISHER_EVENT_HANDLER_HPP_
#define ORG_OPENSPLICE_PUB_PUBLISHER_EVENT_HANDLER_HPP_

#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{

template<typename PUBT>
class OSPL_ISOCPP_IMPL_API PublisherEventForwarder: public DDS::PublisherListener
{
public:
    PublisherEventForwarder(
        const PUBT &pub_,
        dds::pub::PublisherListener* listener);

    virtual ~PublisherEventForwarder();


    virtual dds::pub::PublisherListener* listener();


    //DWL
    virtual void on_offered_deadline_missed(DDS::DataWriter_ptr writer, const DDS::OfferedDeadlineMissedStatus& status) {};
    virtual void on_offered_incompatible_qos(DDS::DataWriter_ptr writer, const DDS::OfferedIncompatibleQosStatus& status) {};
    virtual void on_liveliness_lost(DDS::DataWriter_ptr writer, const DDS::LivelinessLostStatus& status) {};
    virtual void on_publication_matched(DDS::DataWriter_ptr writer, const DDS::PublicationMatchedStatus& status) {};

    dds::core::WeakReference<PUBT> pub_;
    dds::pub::PublisherListener *listener_;
};
}
}
}
#endif /* ORG_OPENSPLICE_PUB_PUBLISHER_EVENT_HANDLER_HPP_ */
